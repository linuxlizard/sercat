// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 */ 

#ifndef XASSERT_H
#define XASSERT_H

#include <stdint.h>

#define XASSERT(expr,value) (expr) ? (void)0: XAssertFail(#expr, __FILE__, __LINE__, value)

void XAssertFail(const char* expr, const char*filename, int line, uintmax_t num);

#endif

