#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_sockfd_c,"$Id: w32_sockfd.c,v 1.5 2019/03/15 23:12:20 cvsuser Exp $")

/*
 * win32 socket file-descriptor support
 *
 * Copyright (c) 2007, 2012 - 2019 Adam Young.
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
 * ==extra==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0601              /* enable vista+ features (WSAPoll) */
#endif

#include "win32_include.h"
#include "win32_internal.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

static int                  x_fdinit;           /* hard limit */
static int                  x_fdlimit = 128;    /* soft limit */
static SOCKET *             x_fdsockets;


/*
 *  file-descriptor association initialisation.
 */
LIBW32_API void
w32_sockfd_init(void)
{
    if (!x_fdinit) {
        unsigned s;

        if (NULL != (x_fdsockets = calloc(sizeof(SOCKET), WIN32_FILDES_MAX))) {
            for (s = 0; s < WIN32_FILDES_MAX; ++s) {
                x_fdsockets[s] = INVALID_SOCKET;
            }
            x_fdinit = WIN32_FILDES_MAX;
        }
    }
}


/*
 *  update the file-descriptor limit
 */
LIBW32_API int
w32_sockfd_limit(int fd)
{
    assert(fd >= -1 /*error*/ && fd < WIN32_FILDES_MAX);

    if (fd > x_fdlimit) {
        x_fdlimit = fd;
    }
    return fd;
}


/*
 *  associate a file-descriptor with a socket.
 */
LIBW32_API void
w32_sockfd_open(int fd, SOCKET s)
{
    assert(fd >= 0 && fd < WIN32_FILDES_MAX);

    w32_sockfd_init();
    w32_sockfd_limit(fd);

    if (x_fdsockets && fd >= 0 && fd < x_fdinit) {
        assert(s != INVALID_SOCKET);
        x_fdsockets[fd] = s;
    }
}


/*
 *      retrieve the socket associated with a file-descriptor.
 */
LIBW32_API SOCKET
w32_sockfd_get(int fd)
{
    if (fd >= WIN32_FILDES_MAX) {               /* not an osf handle; hard limit */
        return fd;

    } else if (fd >= 0 && fd < x_fdinit) {
        if (x_fdsockets) {                      /* local socket mapping */
            return x_fdsockets[fd];

        /*
         *  MSVC 2015+ no longer suitable without fdlimit; asserts when out-of-range
         */
        } else if (fd < x_fdlimit) {
            SOCKET s;
            if ((s = _get_osfhandle(fd)) != (SOCKET)INVALID_HANDLE_VALUE) {
                return s;
            }
        }
    }
    return INVALID_SOCKET;
}


/*
 *      unassociate a file-descriptor with a socket.
 */
LIBW32_API void
w32_sockfd_close(int fd, SOCKET s)
{
    if (fd >= 0 && fd < x_fdinit) {
        if (s == INVALID_SOCKET || x_fdsockets[fd] == s) {
            x_fdsockets[fd] = INVALID_SOCKET;
        }
    }
}


/*
 *      determine whether a socket file descriptor, for read/write/close usage.
 */
//  static int
//  IsStdHandle(int fd)
//  {
//      switch((DWORD)fd) {
//	case STD_INPUT_HANDLE:	// (DWORD)-10   The standard input device. Initially, this is the console input buffer, CONIN$.
//      case STD_OUTPUT_HANDLE: // (DWORD)-11   The standard output device. Initially, this is the active console screen buffer, CONOUT$.
//      case STD_ERROR_HANDLE:  // (DWORD)-12   The standard error device. Initially, this is the active console screen buffer, CONOUT$.
//          return 1;
//      }
//	return 0;
//  }

LIBW32_API int
w32_issockfd(int fd, SOCKET *s)
{
    SOCKET t_s = INVALID_SOCKET;

    if (fd >= 0) {
        if (fd >= WIN32_FILDES_MAX) {           /* not an osf handle; hard limit */
            /*
             *  TODO: restrict logic further
             *      HANDLES should always be DWORD aligned, hence must be "(fd & 0x7) == 0"
             *      Confirm and can this go futher??
             *          Mininal handle value??
             */
            t_s = (SOCKET)fd;

        } else if (fd >= x_fdinit ||            /* local socket mapping */
                    (t_s = x_fdsockets[fd]) == INVALID_SOCKET) {
            /*
             *  MSVC 2015+ no longer suitable without fdlimit; asserts when out-of-range
             */
            if (fd >= x_fdlimit ||
                    _get_osfhandle(fd) == (SOCKET)INVALID_HANDLE_VALUE) {
                t_s = (SOCKET)fd;               /* invalid assume socket; otherwise file */
            }
        }
    }
    if (s) *s = t_s;
    return (t_s != INVALID_SOCKET);
}

/*end*/

