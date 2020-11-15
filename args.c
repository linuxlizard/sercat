#include <stdio.h>
#include <termios.h>
#define _GNU_SOURCE
#include <getopt.h>

#include "args.h"

static void usage(void)
{
	// mimic minicom's options where possible
	const char* msg = "\
	-b --bauderate		: serial port baudrate (default 115200)\n\
	-f --flow		: set flow control (hard,soft,none) (default hard)\n\
	-h --help		: display this help\n\
	-D --device		: serial port path\n\
	-H --displayhex		: display output as hex\n\
	-S --script=SCRIPT	: run this chat(8) script then quit\n\
";
	fprintf(stderr,msg);
}

int parse_args(int argc, char* argv[], struct args* args)
{
	if (argc < 2) {
		usage();
		return -1;
	}

	struct option long_options[] = {
		{"baudrate", required_argument, 0, 'b'},
		{"flow", required_argument, 0, 'f'},
		{"help", no_argument, 0, 'h'},
		{"device", required_argument, 0, 'D'},
		{"displayhex", no_argument, 0, 'H'},
		{"script", required_argument, 0, 'S'},
		{0,0,0,0}
	};

	while(1) {
		int option_index = 0;

		int ret = getopt_long(argc, argv, "b:f:hD:HS:", long_options, &option_index);
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
				printf("baudrate %s\n", optarg);
				break;

		}

	}

	return 0;
}


