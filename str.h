// SPDX-License-Identifier: GPL-2.0-only
/*
 * simple string handling functions
 * davep 20201115
 *
 * (C) 2020 David Poole <davep@.mbuf.com>
 */

#ifndef STR_H
#define STR_H

#include <stdbool.h>

bool str_match( const char *s1, size_t s1_len, const char *s2, size_t s2_len);

#endif

