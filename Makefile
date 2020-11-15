CC=gcc
CFLAGS=-g -Wall -Wpedantic

all: sercat

sercat: sercat.o args.o xassert.o

sercat.o: sercat.c args.h xassert.h

args.o: args.c args.h

xassert.o: xassert.c xassert.h

