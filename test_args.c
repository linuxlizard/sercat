// SPDX-License-Identifier: GPL-2.0-only
/*
 * argument parsing
 * davep 20201115
 *
 * (C) 2020 David Poole <davep@.mbuf.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#include <getopt.h>

#include "args.h"
#include "str.h"
#include "xassert.h"

void reset(void)
{
	optind = 0;
	opterr = 0;
	optopt = 0;
}

void test_no_args(void)
{
	struct args args;

	char* argv[] = { "davewashere", NULL };

	int ret = parse_args(1, argv, &args);
	XASSERT(ret != 0, ret);

	reset();
}

void test_missing_device(void)
{
	struct args args;

	// missing required serial port
	char* argv[] = { "davewashere",
			"-b", "115200", NULL };

	int ret = parse_args(3, argv, &args);
	XASSERT(ret != 0, ret);

	reset();
}

void test_bad_baud(void)
{
	struct args args;

	char* argv[] = { "davewashere", "-d",
			"-b", "42",
			"-D", "/dev/ttyUSB0", 
			NULL };

	int ret = parse_args(6, argv, &args);
	XASSERT(ret!=0, ret);

	reset();
}

void test_bad_flow(void)
{
	struct args args;

	char* argv[] = { "davewashere", "-d",
			"-b", "9600",
			"-D", "/dev/ttyUSB0", 
			"-f", "foo",
			NULL };

	int ret = parse_args(8, argv, &args);
	XASSERT(ret!=0, ret);

	reset();

	char* argv2[] = { "davewashere", "-d",
			"-b", "9600",
			"-D", "/dev/ttyUSB0", 
			"-f", "hardfoo",
			NULL };

	ret = parse_args(8, argv2, &args);
	XASSERT(ret!=0, ret);

	reset();
}

void test_valid(void)
{
	struct args args;

	char* argv[] = { "davewashere", "-d",
			"-b", "57600",
			"-D", "/dev/ttyUSB0", 
			"-f", "hard",
			NULL };

	int ret = parse_args(8, argv, &args);
	XASSERT(ret==0, ret);
	XASSERT(args.baudrate == B57600, args.baudrate);
	XASSERT( str_match( args.serial_port, strlen(args.serial_port),
					"/dev/ttyUSB0", 12), 0);
	XASSERT(args.debug == 1, args.debug);
	XASSERT(args.flow_control == FLOW_CONTROL_HARDWARE, args.flow_control);

	reset();
}

int main(void)
{
	test_no_args();
	test_missing_device();
	test_bad_baud();
	test_bad_flow();
	test_valid();

	return EXIT_SUCCESS;
}

