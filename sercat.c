// SPDX-License-Identifier: GPL-2.0-only
/* 
 * sercat Serial port netcat-y/socat-y thing.
 * 
 * davep 20201113
 *
 * Copyright 2020 David Poole <davep@mbuf.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <regex.h>
#include <sys/wait.h>

#include "args.h"
#include "xassert.h"

//#define NON_BLOCKING_SERIAL 

sig_atomic_t main_quit = 0;


/*
 * The inetline (internet line) is a parser for the CRLF terminated string
 * standard in internet protocols. Tries to handle weird problems such as bare
 * CR, split CRLF (missing the CR but seeing the LF), etc.
 */
enum INETLINE_STATE {
	STATE_START = 1,
	STATE_LINE,
	STATE_EOL
};

struct inetline {
	enum INETLINE_STATE state;
	size_t len;
	char str[1024];
};

#define CR 0x0d
#define LF 0x0a

void inetline_init(struct inetline* line)
{
	memset(line, 0, sizeof(struct inetline));
	line->state = STATE_START;
}

ssize_t inetline_char(struct inetline* line, char c)
{
	// receive a character into the state machine.
	// Returns -errno on error
	// 	0 on "not a full line yet"
	// 	and length on "we found a full line! woo!"

	if (line->len + 1 >= sizeof(line->str)) {
		return -ENOMEM;
	}
	if (line->state == STATE_START) {
		line->len = 0;
		if (c==CR) {
			line->state = STATE_EOL;
		}
		else if (c==LF) {
			// stay in START
		}
		else { 
			line->state = STATE_LINE;
			line->str[line->len++] = c;
		}
	}
	else if (line->state == STATE_EOL) {
		// we are eating a CRLF
		if (c==LF) {
			// found the end of a line; return our length then start from the
			// beginning
			line->state = STATE_START;
			// null terminate
			line->str[line->len++] = 0;
			return line->len;
		}
		else if (c==CR) {
			// ignore
		}
		else {
			// seeing bare CR
			XASSERT(0,c);
		}
	}
	else if (line->state == STATE_LINE) {
		// consuming a line
		if (c==CR) {
			line->state = STATE_EOL;
		}
		else {
			line->str[line->len++] = c;
		}
	}
	// not a full line yet
	return 0;
}

/*
 *  Signal handling.
 */

void signal_term( int signum )
{
	fprintf( stderr, "signal num=%d!\n", signum);
	main_quit = 1;
}

void init_signals( void )
{
	int uerr;
	struct sigaction sigterm; 
	struct sigaction sigint; 

	memset( &sigterm, 0, sizeof(sigterm) );
	sigterm.sa_handler = signal_term;
	uerr = sigaction( SIGTERM, &sigterm, NULL );
	if( uerr != 0 ) {
		perror( "sigaction(SIGTERM)" );
		exit(1);
	}

	memset( &sigint, 0, sizeof(sigint) );
	sigint.sa_handler = signal_term;
	uerr = sigaction( SIGINT, &sigint, NULL );
	if( uerr != 0 ) {
		perror( "sigaction(SIGINT)" );
		exit(1);
	}
}

/*
 * Serial port management
 */

int serial_open_port( struct args* args )
{
	struct termios oldtio, newtio;
	int fd;

	/* stupid human check -- I've been running this thing 
	 *  as root (very very dumb) so I don't want to accidently
	 *  write DNP to /dev/hda1 or something equally tragic.
	 */
//	if( strncmp( path, "/dev/ttyS", 9 ) != 0 ) {
//		LOG_MESSAGE1( LOG_ERR, "\"%s\" not a serial port.", path );		
//		return ERR_FAIL;
//	}

	/* open serial port */
#ifdef NON_BLOCKING_SERIAL
	fd = open( args->serial_port, O_RDWR|O_NOCTTY|O_NONBLOCK );
#else
	fd = open( args->serial_port, O_RDWR|O_NOCTTY );
#endif
	if( fd < 0 ) {
		fprintf( stderr, "open() of %s failed : %s", 
				args->serial_port, strerror(errno) );
		return -1;
	}

	/* davep 29-Apr-2009 ; temp debug */
#ifdef DEBUG
	fprintf( stderr, "%s opened %s successfully\n", __FUNCTION__, args->serial_port );
#endif

	tcgetattr( fd, &oldtio );

	memset( &newtio, 0, sizeof(newtio) );

	newtio.c_cflag = CLOCAL | CREAD ;

	/* parity */
	switch( args->parity ) {
		case SERIAL_NO_PARITY :
			/* nothing -- default is no parity */
			break;

		case SERIAL_EVEN_PARITY :
			newtio.c_cflag |= PARENB;
			break;

		case SERIAL_ODD_PARITY :
			newtio.c_cflag |= PARENB | PARODD;
			break;

		default :
			XASSERT(0, args->parity);
	}

	/* data bits */
	if( args->databits == SERIAL_8_DATA_BITS ) {
		newtio.c_cflag |= CS8;
	}
	else if( args->databits == SERIAL_7_DATA_BITS ) {
		newtio.c_cflag |= CS7;
	}
	else {
		XASSERT( 0, args->databits );
	}

	/* stop bits */
	if( args->stopbits == SERIAL_1_STOP_BITS ) {
		/* nothing -- default is 1 stop bit */
	}
	else if( args->stopbits == SERIAL_2_STOP_BITS ) {
		newtio.c_cflag |= CSTOPB;
	}
	else {
		XASSERT(0, args->stopbits);
	}

	/* hardware flow control? */
	if( args->flow_control==FLOW_CONTROL_HARDWARE ) {
		newtio.c_cflag |= CRTSCTS;
	}

	newtio.c_lflag = 0;
	/* no additional lflags */

	newtio.c_iflag = 0; // | IXON | IXANY | IXOFF | IMAXBEL;

	/* software flow control? */
	if( args->flow_control==FLOW_CONTROL_SOFTWARE ) {
		newtio.c_iflag |= (IXON | IXOFF | IXANY);
	}

	newtio.c_oflag = 0;
	/* no additional oflags */

	cfsetospeed( &newtio, args->baudrate );
	cfsetispeed( &newtio, args->baudrate );

#ifndef NON_BLOCKING_SERIAL
	newtio.c_cc[VMIN]  = 1; /* block until this many chars recvd */
	newtio.c_cc[VTIME] = 0; /* inter character timer unused */
#endif

	tcflush( fd, TCIFLUSH );
	tcsetattr( fd, TCSANOW, &newtio );

	return fd;
}

/*
 *  Run chat.  See 'man 8 chat' for details on chat.
 */

int run_chat(int fd)
{
	const char* chat = "/usr/sbin/chat";
	
	pid_t pid = fork();
	if (pid == 0) {
		// child;
		close(0);
		close(1);
		dup2(fd,0);
		dup2(fd,1);
		execl(chat, chat, "-f", "chat-passwd", "-v", "-s", NULL);
//		int err = execl(chat, chat, "-f", "chatf", "-v", "-s", NULL);
		// should not reach here
		perror("execl");
		return -errno;
	}
	printf("waiting for pid=%d\n", pid);
	int wstatus=0;
	pid_t ret_pid = waitpid(pid, &wstatus, 0);
	printf("ret_pid=%d wstatus=%d exited=%d with code=%d\n", ret_pid, wstatus, 
		WIFEXITED(wstatus), 
		WEXITSTATUS(wstatus));
	return WEXITSTATUS(wstatus);
}

void test_regex(regex_t* prompt)
{
	int err; 

	err = regexec(prompt, "[console@E3000-c93: /]$ ", 0, NULL, 0);
	printf("regex err=%d\n", err);

	err = regexec(prompt, "[console@E3000-c93: \\033[1m/\\033[0;0m]$", 0, NULL, 0);
	printf("regex err=%d\n", err);

	// https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_sequences
	// http://ascii-table.com/ansi-escape-sequences.php
	// CSI == ^[[
	// CSI 1 m "Bold or increased intensity "
	// CSI 0;0m " Set Graphics Mode:"  m == "All attributes off "
}

int main(int argc, char* argv[] )
{
	struct args args;

	if (parse_args(argc, argv, &args) != 0) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

	init_signals();

	int fd = serial_open_port(&args);

	if( fd < 0 ) {
		/* serial_open_port() displays error */
		exit(EXIT_FAILURE);
	}

//	run_chat(fd);
//	return EXIT_SUCCESS;

	// prompt visually looks like:
	// [console@E3000-c93: /]$
	regex_t prompt;
	int err = regcomp(&prompt, "\\[console[^\\$]\\+\\$ ", 0);
	if (err != 0) {
		char msg[1024];
		regerror(err, &prompt, msg, sizeof(msg));
		fprintf(stderr, "%s\n", msg);
		return EXIT_FAILURE;
	}

//	return EXIT_SUCCESS;

	ssize_t ret;
	for (int i=1 ; i<argc ; i++ ) {
		ret = write(fd, argv[i], strlen(argv[i]));
		ret = write(fd, " ", 1);
#ifdef DEBUG
		printf("send ret=%zd\n", ret);
#endif
	}
	write(fd, "\n", 1);

	struct inetline line;
	inetline_init(&line);
	char c;

	int final_err = EXIT_SUCCESS;
	while( !main_quit ) {
		ret = read(fd, &c, 1);

		ret = inetline_char(&line, c);
		if (ret > 0) {
			// we have a full line
			printf("%s\n", line.str);
		}
		else if (ret < 0) {
			final_err = EXIT_FAILURE;
			fprintf(stderr, "inetline_char() failed err=%d\n", err);
			break;
		}
		else {
			// did we hit a prompt? if so, we can leave
			err = regexec(&prompt, line.str, 0, NULL, 0);
			if (err==0) {
				break;
			}
		}
	}

	regfree(&prompt);
	close(fd);
	return final_err;
}

