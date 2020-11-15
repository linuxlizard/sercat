# SPDX-License-Identifier: GPL-2.0-only
#
# sercat : like netcat and socat but for serial ports
# davep 20201115
#
# (C) 2020 David Pole <davep@.mbuf.com>
#

CC=gcc
CFLAGS=-g -Wall -Wpedantic

all: sercat

sercat: sercat.o args.o xassert.o str.o

sercat.o: sercat.c args.h xassert.h str.h

args.o: args.c args.h

xassert.o: xassert.c xassert.h

test: test_args test_str
	./test_args && ./test_str

test_args: test_args.o args.o xassert.o str.o

test_str: test_str.o str.o xassert.o 

test_args.o: test_args.c args.h xassert.h str.h

test_str.o: test_str.c str.h xassert.h 

clean:
	$(RM) -f *.o sercat
