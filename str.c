// SPDX-License-Identifier: GPL-2.0-only
/*
 * simple string handling functions
 * davep 20201115
 *
 * (C) 2020 David Poole <davep@.mbuf.com>
 */

#include <string.h>

#include "str.h"

bool str_match( const char *s1, size_t s1_len, const char *s2, size_t s2_len)
{
	/* Strings must exactly match. Forcing length to avoid strlen() problems.
	 */
	return (s1_len==s2_len) && (strncmp(s1, s2, s1_len) == 0);
}

