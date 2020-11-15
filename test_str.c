// SPDX-License-Identifier: GPL-2.0-only
/*
 * simple string handling functions
 * davep 20201115
 *
 * (C) 2020 David Poole <davep@.mbuf.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "xassert.h"

void test_str_match(void)
{
	bool flag;

	flag = str_match( "hello", 5, "world", 5);
	XASSERT(!flag, flag);

	flag = str_match( "hello", 5, "helloworld", 10);
	XASSERT(!flag, flag);

	flag = str_match( "hello", 5, "hello", 5);
	XASSERT(flag, flag);

	flag = str_match( "", 0, "", 0);
	XASSERT(flag, flag);
}

int main(void)
{
	return EXIT_SUCCESS;
}

