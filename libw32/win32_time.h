#ifndef LIBW32_WIN32_TIME_H_INCLUDED
#define LIBW32_WIN32_TIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_time_h,"$Id: win32_time.h,v 1.12 2020/06/18 13:29:15 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 time functionality.
 *
 * Copyright (c) 1998 - 2019, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <sys/cdefs.h>

__BEGIN_DECLS

LIBW32_API unsigned int sleep(unsigned int);

struct timeval;
struct timezone;

LIBW32_API int          gettimeofday(struct timeval *tv, struct timezone *tz);

struct utimbuf;

LIBW32_API int          w32_utime(const char *path, const struct utimbuf *times);

#if defined(_MSC_VER) || defined(__WATCOMC__)
LIBW32_API time_t       timegm(struct tm *tm);
#endif

__END_DECLS

#endif /*LIBW32_WIN32_TIME_H_INCLUDED*/
