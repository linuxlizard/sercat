// SPDX-License-Identifier: GPL-2.0-only
/*
 * argument parsing
 * davep 20201115
 *
 * (C) 2020 David Poole <davep@.mbuf.com>
 */

#include <stdio.h>
#include <termios.h>
#include <string.h>
#define _GNU_SOURCE
#include <getopt.h>

#include "args.h"
#include "str.h"

struct baud_rate
{
	const char *str;
	int len; /* string length of str */
	speed_t speed;
} ;

struct baud_rate baudrate_list[] = {
	{ "110",  3, B110   },
	{ "150",  3, B150   },
	{ "300",  3, B300   },
	{ "600",  3, B600   },
	{ "1200", 4, B1200  },
	{ "2400", 4, B2400  },
	{ "4800", 4, B4800  },
	{ "9600", 4, B9600  },
	{ "19200",  5, B19200 },
	{ "38400",  5, B38400 },
	{ "57600",  5, B57600 },
	{ "115200", 6, B115200},
	{ "230400", 6, B230400},
};

static const struct args default_args = {
	.baudrate = DEFAULT_BAUD_RATE,
	.parity = SERIAL_NO_PARITY,
	.databits = SERIAL_8_DATA_BITS,
	.stopbits = SERIAL_1_STOP_BITS,
	.flow_control = FLOW_CONTROL_HARDWARE,
};

static void usage(void)
{
	// mimic minicom's options where possible
	const char* msg = "\
	-b --baudrate		: serial port baudrate (default 115200)\n\
	-d --debug		: increase debug/verbosity (multiple args for more verbosity)\n\
	-f --flow		: set flow control (hard,soft,none) (default hard)\n\
	-h --help		: display this help\n\
	-D --device		: serial port path\n\
	-H --displayhex		: display output as hex\n\
	-S --script=SCRIPT	: run this chat(8) script then quit\n\
";
	fprintf(stderr,msg);
}

int baud_string_to_baud( char *baudstring, speed_t *baudrate )
{
	int i;

	for( i=0 ; i<sizeof(baudrate_list)/sizeof(struct baud_rate) ; i++ ) {
		if(strncmp(baudrate_list[i].str,baudstring,baudrate_list[i].len )==0) {
			/* found it */
			*baudrate = baudrate_list[i].speed;
			return 0;
		}
	}

	return -1;
}

int parse_args(int argc, char* argv[], struct args* args)
{
//	char* ptr = argv[0];
//	printf("%s\n", ptr++);

	if (argc < 2) {
		usage();
		return -1;
	}

	memcpy(args, &default_args, sizeof(struct args));

	/* TODO add parity, data-bits, stop-bits to args parsing.
	 * Ignoring for now since the world has been N81 for a while.
	 */
	struct option long_options[] = {
		{"baudrate", required_argument, 0, 'b'},
		{"debug", no_argument, 0, 'd'},  // not in minicom
		{"flow", required_argument, 0, 'f'},
		{"help", no_argument, 0, 'h'},
		{"device", required_argument, 0, 'D'},
		{"displayhex", no_argument, 0, 'H'},
		{"script", required_argument, 0, 'S'},
		{0,0,0,0}
	};

	while(1) {
		int option_index = 0;

		int ret = getopt_long(argc, argv, "b:df:hD:HS:", long_options, &option_index);

		if (ret == -1) {
			// end of options
			break;
		}

		if (ret=='h') {
			usage();
			return -1;
		}

		switch (ret) {
			// baudrate
			case 'b':
				/* convert baud rate string to baud rate number */
				if( baud_string_to_baud( optarg, &args->baudrate ) != 0 ) {
					fprintf( stderr, "Invalid baud rate \"%s\"\n", optarg );
					return -1;
				}
				break;

			case 'd':
				args->debug++;
				break;

			case 'f' :
				if( str_match( optarg, strlen(optarg), "hard", 4 ) ) {
					args->flow_control = FLOW_CONTROL_HARDWARE;
				}
				else if( str_match( optarg, strlen(optarg), "soft", 4 ) ) {
					args->flow_control = FLOW_CONTROL_SOFTWARE;
				}
				else if( str_match( optarg, strlen(optarg), "none", 4 ) ) {
					args->flow_control = FLOW_CONTROL_NONE;
				}
				else {
					fprintf( stderr, "Invalid flow control \"%s\"\n", optarg );
					return -1;
				}
				break;

			case 'h' :
				usage();
				return -1;

			case 'D':
				strncpy(args->serial_port, optarg, sizeof(args->serial_port)-1);
				break;

			case 'H':
				args->hex = true;
				break;

			case 'S':
				strncpy(args->chat_file, optarg, sizeof(args->chat_file)-1);
				args->use_chat = true;
				break;

			case '?' :
				usage();
				return -1;
		}
	}

	if (!args->serial_port[0]) {
		fprintf(stderr, "serial port required (-D option)\n");
		return -1;
	}

	return 0;
}


