#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_unlink_c,"$Id: w32_unlink.c,v 1.11 2019/03/15 23:12:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win2 unlink() system calls
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"
#include <unistd.h>

/*
//  NAME
//      unlink - remove a directory entry
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      int unlink(const char *path);
//      int w32_unlink(const char *path);
//
//  DESCRIPTION
//      The unlink() function shall remove a link to a file. If path names a symbolic link,
//      unlink() shall remove the symbolic link named by path and shall not affect any file
//      or directory named by the contents of the symbolic link. Otherwise, unlink() shall
//      remove the link named by the pathname pointed to by path and shall decrement the
//      link count of the file referenced by the link.
//
//      When the file's link count becomes 0 and no process has the file open, the space
//      occupied by the file shall be freed and the file shall no longer be accessible. If
//      one or more processes have the file open when the last link is removed, the link
//      shall be removed before unlink() returns, but the removal of the file contents
//      shall be postponed until all references to the file are closed.
//
//      The path argument shall not name a directory unless the process has appropriate
//      privileges and the implementation supports using unlink() on directories.
//
//      Upon successful completion, unlink() shall mark for update the st_ctime and
//      st_mtime fields of the parent directory. Also, if the file's link count is not 0,
//      the st_ctime field of the file shall be marked for update.
//
//  RETURN VALUE
//      Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned
//      and errno set to indicate the error. If -1 is returned, the named file shall not be
//      changed.
//
//  ERRORS
//      The unlink() function shall fail and shall not unlink the file if:
//
//      [EACCES]
//          Search permission is denied for a component of the path prefix, or write
//          permission is denied on the directory containing the directory entry to be
//          removed.
//
//      [EBUSY]
//          The file named by the path argument cannot be unlinked because it is being used
//          by the system or another process and the implementation considers this an error.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of the path
//          argument.
//
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component is
//          longer than {NAME_MAX}.
//
//      [ENOENT]
//          A component of path does not name an existing file or path is an empty string.
//
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//
//      [EPERM]
//          The file named by path is a directory, and either the calling process does not
//          have appropriate privileges, or the implementation prohibits using unlink() on
//          directories.
//
//      [EPERM] or [EACCES]
//          The S_ISVTX flag is set on the directory containing the file referred to by the
//          path argument and the caller is not the file owner, nor is the caller the
//          directory owner, nor does the caller have appropriate privileges. [Option End]
//
//      [EROFS]
//          The directory entry to be unlinked is part of a read-only file system.
//
//      The unlink() function may fail and not unlink the file if:
//
//      [EBUSY]
//          The file named by path is a named STREAM.
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
//          the path argument.
//
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the path argument,
//          the length of the substituted pathname string exceeded {PATH_MAX}.
//
//      [ETXTBSY]
//          The entry to be unlinked is the last directory entry to a pure procedure
//          (shared text) file that is being executed.
*/
int
w32_unlink(const char *path)
{
    int ret = -1;                               // success=0, otherwise=-1

    if (!path) {
        errno = EFAULT;

    } else if (!*path) {
        errno = ENOENT;

    } else {
        DWORD attrs, rc = 0;                    // completion code

#ifndef ERROR_DIRECTORY_NOT_SUPPORTED
#define ERROR_DIRECTORY_NOT_SUPPORTED 336L      // An operation is not supported on a directory.
#endif

        if (INVALID_FILE_ATTRIBUTES /*0xffffffff*/
                    == (attrs = GetFileAttributesA(path)) ) {
            rc = GetLastError();
        } else {
            if (FILE_ATTRIBUTE_DIRECTORY & attrs) {
                if (FILE_ATTRIBUTE_REPARSE_POINT & attrs) {
                    if (! RemoveDirectoryA(path)) {
                        rc = GetLastError();
                    }
                } else {
                    rc = ERROR_DIRECTORY_NOT_SUPPORTED;
                }
            } else {
                if (! DeleteFileA(path) &&
                        ERROR_ACCESS_DENIED == (rc = GetLastError())) {
                    (void) WIN32_CHMOD(path, S_IWRITE);
                    rc = (DeleteFileA(path) ? 0 : GetLastError());
                }
            }
        }

        if (0 == rc) {
            ret = 0;                            // success
        } else {
            switch (rc) {
            case ERROR_ACCESS_DENIED:
            case ERROR_SHARING_VIOLATION:
            case ERROR_PRIVILEGE_NOT_HELD:
                errno = EACCES;  break;
            case ERROR_FILE_NOT_FOUND:
                errno = ENOENT;  break;
            case ERROR_PATH_NOT_FOUND:
                errno = ENOTDIR; break;
            case ERROR_DIRECTORY_NOT_SUPPORTED:
                errno = EISDIR;  break;
            case ERROR_NOT_ENOUGH_MEMORY:
                errno = ENOMEM;  break;
            default:
                errno = w32_errno_cnv(rc);
                break;
            }
        }
    }
    return ret;
}

/*end*/
