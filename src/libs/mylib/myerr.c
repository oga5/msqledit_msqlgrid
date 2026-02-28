/*
 * Copyright (c) 2026, Atsushi Ogawa
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/* ORIGINAL License is below: */
/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

/*
 * myerr.c — error message retrieval.  MySQL equivalent of pgerr.c.
 */

#include <stdio.h>
#include <stdlib.h>

#include "localdef.h"
#include "myapi.h"

/* =========================================================================
   my_err_msg — copy the last MySQL error message into msg_buf as TCHAR.
   ========================================================================= */
void my_err_msg(HMySession ss, TCHAR *msg_buf)
{
    const char *msg;

    if (msg_buf == NULL) return;

    msg = fp_mysql_error(ss->conn);
    if (msg == NULL || *msg == '\0') {
        _tcscpy(msg_buf, _T("Unknown MySQL error."));
        return;
    }

    oci_str_to_win_str((const ocichar *)msg, msg_buf, 512);
}
