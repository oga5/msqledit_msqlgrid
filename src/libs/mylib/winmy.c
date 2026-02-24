/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

/*
 * winmy.c — load libmysql.dll at runtime and resolve all function pointers.
 *
 * Define DLL_MAIN *before* including localdef.h so that the function-pointer
 * variables are *defined* (not merely declared) in this translation unit.
 */

#define DLL_MAIN
#include "localdef.h"
#include "myapi.h"
#include "mymsg.h"

#ifdef WIN32

#include <tchar.h>
#include <windows.h>
#include <wchar.h>

static HINSTANCE my_dll = NULL;

/* The DLL name can be overridden at compile time via -DLIBMYSQL_DLL_NAME=... */
#ifndef LIBMYSQL_DLL_NAME
#define LIBMYSQL_DLL_NAME   _T("libmysql.dll")
#endif

/* Convenience macro: resolve one symbol; jump to ERR1 on failure. */
#define LOAD_SYM(fp, type, name)                                    \
    do {                                                            \
        fp = (type)GetProcAddress(my_dll, name);                    \
        if (fp == NULL) goto ERR1;                                  \
    } while (0)

/* Optional symbol — missing is not fatal. */
#define LOAD_SYM_OPT(fp, type, name)                                \
    do {                                                            \
        fp = (type)GetProcAddress(my_dll, name);                    \
    } while (0)

int my_init_library(TCHAR *msg_buf)
{
    my_dll = LoadLibrary(LIBMYSQL_DLL_NAME);
    if (my_dll == NULL) {
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_LOAD_LIBMYSQL_MSG);
        return MYERR_LOAD_LIBMYSQL;
    }

    LOAD_SYM(fp_mysql_init,              FP_mysql_init,              "mysql_init");
    LOAD_SYM(fp_mysql_real_connect,      FP_mysql_real_connect,      "mysql_real_connect");
    LOAD_SYM(fp_mysql_close,             FP_mysql_close,             "mysql_close");
    LOAD_SYM(fp_mysql_options,           FP_mysql_options,           "mysql_options");
    LOAD_SYM(fp_mysql_set_character_set, FP_mysql_set_character_set, "mysql_set_character_set");

    LOAD_SYM(fp_mysql_query,             FP_mysql_query,             "mysql_query");
    LOAD_SYM(fp_mysql_store_result,      FP_mysql_store_result,      "mysql_store_result");
    LOAD_SYM(fp_mysql_free_result,       FP_mysql_free_result,       "mysql_free_result");

    LOAD_SYM(fp_mysql_num_rows,          FP_mysql_num_rows,          "mysql_num_rows");
    LOAD_SYM(fp_mysql_num_fields,        FP_mysql_num_fields,        "mysql_num_fields");
    LOAD_SYM(fp_mysql_fetch_row,         FP_mysql_fetch_row,         "mysql_fetch_row");
    LOAD_SYM(fp_mysql_fetch_lengths,     FP_mysql_fetch_lengths,     "mysql_fetch_lengths");
    LOAD_SYM(fp_mysql_fetch_field,       FP_mysql_fetch_field,       "mysql_fetch_field");
    LOAD_SYM(fp_mysql_fetch_fields,      FP_mysql_fetch_fields,      "mysql_fetch_fields");
    LOAD_SYM(fp_mysql_field_count,       FP_mysql_field_count,       "mysql_field_count");

    LOAD_SYM(fp_mysql_error,             FP_mysql_error,             "mysql_error");
    LOAD_SYM(fp_mysql_errno,             FP_mysql_errno,             "mysql_errno");
    LOAD_SYM(fp_mysql_sqlstate,          FP_mysql_sqlstate,          "mysql_sqlstate");

    LOAD_SYM(fp_mysql_autocommit,        FP_mysql_autocommit,        "mysql_autocommit");
    LOAD_SYM(fp_mysql_commit,            FP_mysql_commit,            "mysql_commit");
    LOAD_SYM(fp_mysql_rollback,          FP_mysql_rollback,          "mysql_rollback");

    LOAD_SYM(fp_mysql_get_server_info,   FP_mysql_get_server_info,   "mysql_get_server_info");
    LOAD_SYM(fp_mysql_thread_id,         FP_mysql_thread_id,         "mysql_thread_id");
    LOAD_SYM(fp_mysql_affected_rows,     FP_mysql_affected_rows,     "mysql_affected_rows");

    /* Optional — not fatal if absent in older connector versions */
    LOAD_SYM_OPT(fp_mysql_warning_count, FP_mysql_warning_count,     "mysql_warning_count");
    LOAD_SYM_OPT(fp_mysql_info,          FP_mysql_info,              "mysql_info");

    return 0; /* OK */

ERR1:
    if (my_dll != NULL) {
        FreeLibrary(my_dll);
        my_dll = NULL;
    }
    if (msg_buf != NULL) _stprintf(msg_buf, MYERR_INIT_LIBMYSQL_MSG);
    return MYERR_INIT_LIBMYSQL;
}

int my_free_library(TCHAR *msg_buf)
{
    BOOL ret;

    if (my_dll != NULL) {
        ret = FreeLibrary(my_dll);
        if (ret == FALSE) {
            my_dll = NULL;
            if (msg_buf != NULL) _stprintf(msg_buf, MYERR_FREE_LIBMYSQL_MSG);
            return MYERR_FREE_LIBMYSQL;
        }
    }

    my_dll = NULL;
    return 0;
}

int my_library_is_ok(void)
{
    if (my_dll == NULL) return FALSE;
    return TRUE;
}

#else /* WIN32 */

int my_init_library(TCHAR *msg_buf)
{
    return 0;
}

int my_free_library(TCHAR *msg_buf)
{
    return 0;
}

int my_library_is_ok(void)
{
    return TRUE;
}

#endif /* WIN32 */
