#include <edidentifier.h>
__CIDENT_RCSID(cr_append_c,"$Id: llappend.c,v 1.11 2018/10/01 22:13:55 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llappend.c,v 1.11 2018/10/01 22:13:55 cvsuser Exp $
 * Linked list management module
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

#include "llheaders.h"

List_p
ll_append(Head_p hp, void * atom)
{
    List_p lp;

    LL_HMAGIC(hp);
    LIST_INCR(ll_append);

    if ((lp = ll_alloc(atom)) != (List_p)NULL) {
        lp->ll_owner = hp;
        TAILQ_INSERT_TAIL(&hp->lh_queue, lp, ll_node);
        ++hp->lh_count;
    }  
    return (lp);
}

