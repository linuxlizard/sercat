// SPDX-License-Identifier: GPL-2.0-only
/*
 * argument parsing
 * davep 20201115
 *
 * (C) 2020 David Poole <davep@.mbuf.com>
 */
#ifndef ARGS_H
#define ARGS_H

#include <termios.h>
#include <stdbool.h>

/* values for parity */
#define SERIAL_NO_PARITY    1
#define SERIAL_EVEN_PARITY  2
#define SERIAL_ODD_PARITY   4

/* values for data bits */
#define SERIAL_8_DATA_BITS  8 
#define SERIAL_7_DATA_BITS  7 

/* values for stop bits */
#define SERIAL_1_STOP_BITS  1 
#define SERIAL_2_STOP_BITS  2 

/* flow control 
 * hardware flor == RTSCTS 
 * software flow == XON/XOFF
 *
 * no support for DTR/DTS flow control
 */
#define FLOW_CONTROL_NONE     1
#define FLOW_CONTROL_HARDWARE 2
#define FLOW_CONTROL_SOFTWARE 3

#define DEFAULT_BAUD_RATE  B115200
//#define BAUD_RATE  B19200
//#define BAUD_RATE  B38400
//#define BAUD_RATE  B38400

struct args 
{
	// serial port configuration
	speed_t baudrate;
	int parity;
	int stopbits;
	int databits;
	int flow_control;
	char serial_port[FILENAME_MAX+1];

	// debug/verbosity
	int debug;

	// dump all output in hex
	bool hex;

	// chat 
	bool use_chat;
	char chat_file[FILENAME_MAX+1];
};


int parse_args(int argc, char** argv, struct args* args);

#endif

