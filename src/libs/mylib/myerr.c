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
