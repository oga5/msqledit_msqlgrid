/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _MYLIB_LOCAL_DEF_H_INCLUDE_
#define _MYLIB_LOCAL_DEF_H_INCLUDE_

/*
 * Opaque MySQL types — we load libmysql.dll dynamically and never include
 * mysql.h directly, so we re-declare only the fields we actually use.
 */
typedef void            MYSQL;
typedef void            MYSQL_RES;
typedef char **         MYSQL_ROW;
typedef unsigned long long my_ulonglong;

typedef struct {
    char          *catalog;
    char          *db;
    char          *table;
    char          *org_table;
    char          *name;
    char          *org_name;
    unsigned int   name_length;
    unsigned int   org_name_length;
    unsigned int   table_length;
    unsigned int   org_table_length;
    unsigned int   db_length;
    unsigned int   catalog_length;
    unsigned int   def_length;
    unsigned int   flags;
    unsigned int   decimals;
    unsigned int   charsetnr;
    unsigned int   type;       /* enum_field_types / MYSQL_TYPE_* */
    char          *def;
    unsigned long  length;
    unsigned long  max_length;
} MYSQL_FIELD;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#include <tchar.h>
#include <stdlib.h>
#include <string.h>

#define MY_ARRAY_SIZEOF(arr)    (sizeof(arr) / sizeof((arr)[0]))

/*
 * MinGW cross-compilation shim: MinGW's swprintf (which _stprintf maps to)
 * requires an explicit buffer-size argument in C99+ mode, whereas MSVC's
 * _swprintf does not.  Redirect to _sntprintf when building with MinGW so
 * that the same _stprintf(buf, fmt, ...) call sites compile everywhere.
 *
 * 4096 is chosen as a safe upper bound: every msg_buf passed into the
 * mylib API is expected to be at least this large (typical callers
 * allocate 1024–4096 TCHARs).  On the real MSVC target this shim is not
 * active and the target buffer size governs output length.
 */
#if defined(__MINGW32__) || defined(__MINGW64__)
# ifdef _stprintf
#  undef _stprintf
# endif
# define _stprintf(buf, ...) _sntprintf((buf), 4096, __VA_ARGS__)
#endif

extern unsigned int g_oci_code_page;

/* Forward-declare the notice-processor type before the session struct.
   The same typedef appears in myapi.h; both are guarded by the same macro
   so only one definition survives per translation unit. */
#ifndef MY_NOTICE_PROCESSOR_DEFINED
#define MY_NOTICE_PROCESSOR_DEFINED
typedef void (*my_notice_processor)(void *arg, const TCHAR *message);
#endif

/* -----------------------------------------------------------------------
   Handle typedefs (pointer-to-incomplete struct).
   External callers see these via mylib.h; internal code sees the full
   struct body below.
   ----------------------------------------------------------------------- */
typedef struct my_session_st *      HMySession;
typedef struct my_session_st        MySession;
struct my_session_st {
    MYSQL               *conn;
    my_notice_processor  notice_proc;
    void                *notice_arg;
    int                  remote_version;
    int                  auto_commit_off;
    unsigned long        thread_id;   /* for KILL QUERY */

    TCHAR               *user;
    TCHAR               *host;
    TCHAR               *db;
    TCHAR               *port;
    TCHAR               *charset;
    TCHAR               *passwd;      /* kept for KILL QUERY reconnect */
};

typedef struct my_dataset_st *      HMyDataset;
typedef struct my_dataset_st        MyDataset;
struct my_dataset_st {
    int            row_cnt;
    int            col_cnt;
    TCHAR        **col_data;   /* row_cnt * col_cnt flat array of pointers */
    TCHAR        **cname;      /* col_cnt column name pointers */
    TCHAR         *buf;        /* single allocation backing col_data + cname */
    int           *is_null;    /* row_cnt * col_cnt null flags */
    unsigned int  *col_type;   /* col_cnt MYSQL_TYPE_* values */
    unsigned long *col_size;   /* col_cnt column declared lengths */
};

/* -----------------------------------------------------------------------
   Function-pointer typedefs for libmysql.dll
   ----------------------------------------------------------------------- */
typedef MYSQL *(*FP_mysql_init)(MYSQL *mysql);
typedef MYSQL *(*FP_mysql_real_connect)(MYSQL *mysql,
    const char *host, const char *user, const char *passwd,
    const char *db, unsigned int port,
    const char *unix_socket, unsigned long clientflag);
typedef void        (*FP_mysql_close)(MYSQL *sock);
typedef int         (*FP_mysql_options)(MYSQL *mysql, int option, const void *arg);
typedef int         (*FP_mysql_set_character_set)(MYSQL *mysql, const char *csname);

typedef int         (*FP_mysql_query)(MYSQL *mysql, const char *q);
typedef MYSQL_RES  *(*FP_mysql_store_result)(MYSQL *mysql);
typedef void        (*FP_mysql_free_result)(MYSQL_RES *result);

typedef my_ulonglong  (*FP_mysql_num_rows)(MYSQL_RES *res);
typedef unsigned int  (*FP_mysql_num_fields)(MYSQL_RES *res);
typedef MYSQL_ROW     (*FP_mysql_fetch_row)(MYSQL_RES *result);
typedef unsigned long *(*FP_mysql_fetch_lengths)(MYSQL_RES *result);
typedef MYSQL_FIELD   *(*FP_mysql_fetch_field)(MYSQL_RES *result);
typedef MYSQL_FIELD   *(*FP_mysql_fetch_fields)(MYSQL_RES *result);
typedef unsigned int  (*FP_mysql_field_count)(MYSQL *mysql);

typedef const char   *(*FP_mysql_error)(MYSQL *mysql);
typedef unsigned int  (*FP_mysql_errno)(MYSQL *mysql);
typedef const char   *(*FP_mysql_sqlstate)(MYSQL *mysql);

typedef int  (*FP_mysql_autocommit)(MYSQL *mysql, char auto_mode);
typedef int  (*FP_mysql_commit)(MYSQL *mysql);
typedef int  (*FP_mysql_rollback)(MYSQL *mysql);

typedef const char    *(*FP_mysql_get_server_info)(MYSQL *mysql);
typedef unsigned long  (*FP_mysql_thread_id)(MYSQL *mysql);
typedef unsigned int   (*FP_mysql_warning_count)(MYSQL *mysql);
typedef const char    *(*FP_mysql_info)(MYSQL *mysql);
typedef my_ulonglong   (*FP_mysql_affected_rows)(MYSQL *mysql);

/* -----------------------------------------------------------------------
   DLL_MAIN controls whether the pointers are *defined* (in winmy.c) or
   merely *declared* (in every other translation unit).
   ----------------------------------------------------------------------- */
#ifdef DLL_MAIN
    FP_mysql_init               fp_mysql_init               = NULL;
    FP_mysql_real_connect       fp_mysql_real_connect       = NULL;
    FP_mysql_close              fp_mysql_close              = NULL;
    FP_mysql_options            fp_mysql_options            = NULL;
    FP_mysql_set_character_set  fp_mysql_set_character_set  = NULL;
    FP_mysql_query              fp_mysql_query              = NULL;
    FP_mysql_store_result       fp_mysql_store_result       = NULL;
    FP_mysql_free_result        fp_mysql_free_result        = NULL;
    FP_mysql_num_rows           fp_mysql_num_rows           = NULL;
    FP_mysql_num_fields         fp_mysql_num_fields         = NULL;
    FP_mysql_fetch_row          fp_mysql_fetch_row          = NULL;
    FP_mysql_fetch_lengths      fp_mysql_fetch_lengths      = NULL;
    FP_mysql_fetch_field        fp_mysql_fetch_field        = NULL;
    FP_mysql_fetch_fields       fp_mysql_fetch_fields       = NULL;
    FP_mysql_field_count        fp_mysql_field_count        = NULL;
    FP_mysql_error              fp_mysql_error              = NULL;
    FP_mysql_errno              fp_mysql_errno              = NULL;
    FP_mysql_sqlstate           fp_mysql_sqlstate           = NULL;
    FP_mysql_autocommit         fp_mysql_autocommit         = NULL;
    FP_mysql_commit             fp_mysql_commit             = NULL;
    FP_mysql_rollback           fp_mysql_rollback           = NULL;
    FP_mysql_get_server_info    fp_mysql_get_server_info    = NULL;
    FP_mysql_thread_id          fp_mysql_thread_id          = NULL;
    FP_mysql_warning_count      fp_mysql_warning_count      = NULL;
    FP_mysql_info               fp_mysql_info               = NULL;
    FP_mysql_affected_rows      fp_mysql_affected_rows      = NULL;
#else /* DLL_MAIN */
    extern FP_mysql_init               fp_mysql_init;
    extern FP_mysql_real_connect       fp_mysql_real_connect;
    extern FP_mysql_close              fp_mysql_close;
    extern FP_mysql_options            fp_mysql_options;
    extern FP_mysql_set_character_set  fp_mysql_set_character_set;
    extern FP_mysql_query              fp_mysql_query;
    extern FP_mysql_store_result       fp_mysql_store_result;
    extern FP_mysql_free_result        fp_mysql_free_result;
    extern FP_mysql_num_rows           fp_mysql_num_rows;
    extern FP_mysql_num_fields         fp_mysql_num_fields;
    extern FP_mysql_fetch_row          fp_mysql_fetch_row;
    extern FP_mysql_fetch_lengths      fp_mysql_fetch_lengths;
    extern FP_mysql_fetch_field        fp_mysql_fetch_field;
    extern FP_mysql_fetch_fields       fp_mysql_fetch_fields;
    extern FP_mysql_field_count        fp_mysql_field_count;
    extern FP_mysql_error              fp_mysql_error;
    extern FP_mysql_errno              fp_mysql_errno;
    extern FP_mysql_sqlstate           fp_mysql_sqlstate;
    extern FP_mysql_autocommit         fp_mysql_autocommit;
    extern FP_mysql_commit             fp_mysql_commit;
    extern FP_mysql_rollback           fp_mysql_rollback;
    extern FP_mysql_get_server_info    fp_mysql_get_server_info;
    extern FP_mysql_thread_id          fp_mysql_thread_id;
    extern FP_mysql_warning_count      fp_mysql_warning_count;
    extern FP_mysql_info               fp_mysql_info;
    extern FP_mysql_affected_rows      fp_mysql_affected_rows;
#endif /* DLL_MAIN */

/* -----------------------------------------------------------------------
   Internal helpers declared here so all .c files can call them.
   ----------------------------------------------------------------------- */
/* myutil.c */
void msleep(int msec);

/* mysql.c */
void my_kill_query(HMySession ss);

/* myerr.c */
void my_err_msg(HMySession ss, TCHAR *msg_buf);

/* mydataset.c */
HMyDataset my_build_dataset_from_res(MYSQL_RES *res, TCHAR *msg_buf);

#include "myapi.h"

#endif /* _MYLIB_LOCAL_DEF_H_INCLUDE_ */
