// SPDX-License-Identifier: GPL-2.0-only
/*
 * XAssert - an embraced and extended assert() macro which can also report a
 * value.
 *
 * err = some_function()
 * XASSERT(err==0, err);  <-- on failure, will report the value of err
 */

#include <stdio.h>
#include <stdlib.h>

#include "xassert.h"

void XAssertFail(const char* expr, const char*filename, int line, uintmax_t num)
{
	fprintf(stderr, "Assert Fail: %s at %s %d value=%jx\n", expr, filename, line, num );
	abort();
}

