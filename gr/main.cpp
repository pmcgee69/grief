#include <edidentifier.h>
__CIDENT_RCSID(gr_main_cpp,"$Id: main.cpp,v 1.15 2020/06/18 12:45:13 cvsuser Exp $")

/* -*- mode: cpp; indent-width: 4; -*- */
/* $Id: main.cpp,v 1.15 2020/06/18 12:45:13 cvsuser Exp $
 * main(), address c/c++ linkage for several environments.
 * Regardless of configuration force binding to the C++ runtime library.
 *
 *
 */

#if (defined(_WIN32) || defined(WIN32)) && \
	!defined(WINDOWS_MEAN_AND_LEAN)
#define WINDOWS_MEAN_AND_LEAN
#endif

#include <editor.h>
#include <chkalloc.h>
#include "signals.h"

#if defined(_CRT_NO_POSIX_ERROR_CODES)
#if (_MSC_VER >= 1900) || defined(__MAKEDEPEND__)
#undef _CRT_NO_POSIX_ERROR_CODES                /* <system_error> is incompatible with _CRT_NO_POSIX_ERROR_CODES */
    // additional research required, yet there is limited information detailing _CRT_NO_POSIX_ERROR_CODES usage and how these interact with winsocks.
#endif //_MSC_VER
#endif //_CRT_NO_POSIX_ERROR_CODES

#include <iostream>
#include <exception>
    // XXX: avoid std::string, open-watcom linker issues resulting in debug symbol issues.


extern "C" {
    int cmain(int argc, char **argv);           /* cmain.c */
    void cpp_linkage(const char *str);
}


static void
cpp_terminate()
{
    sighandler_abrt(SIGABRT);
    std::terminate();
}


int
main(int argc, char **argv)
{
    int memchk = 0;

    for (int i = 1; i < argc; ++i) {
        const char *opt, *arg = argv[i];

        if ('-' == arg[0]) {
            if ('-' == arg[1]) {
                /* --rtc|--native */
                if (0 == strcmp(arg + 2, "rtc")) {
                    argv[i] = (char *)"-Prtc";
                    memchk = 2;

                } else if (0 == strcmp(arg + 2, "native")) {
                    argv[i] = (char *)"-Pnative";
                    if (!memchk) memchk = 1;
                }

            } else if ('P' == arg[1]) {
                /* -P<flags> */
                arg += 2;

                if (!*arg && ++i < argc && '-' != argv[i][0]) {
                    arg = argv[i];
                }
                                                /* rtc */
                if (NULL != (opt = strstr(arg, "rtc"))) {
                    if (0 == opt[3] || ',' == opt[3]) {
                        memchk = 2;
                        break;
                    }

                } else if (!memchk &&           /* native,purify */
                        (NULL != (opt = strstr(arg, "native")) || NULL != (opt = strstr(arg, "purify")))) {
                    if (0 == opt[6] || ',' == opt[6]) {
                        memchk = 1;
                    }
                }
            }
        }
    }

    if (2 == memchk) {
        chk_rtc();                              /* memory run-time checks */
    } else if (memchk) {
        chk_native();                           /* native memory allocator */
    }

    std::set_terminate(cpp_terminate);
    return cmain(argc, argv);
}


void
cpp_linkage(const char *str)
{
    std::ostream *out = &std::cout;
    *out << str;
}

/*end*/
