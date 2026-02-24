/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

/*
 * myutil.c — session management, charset conversion, and miscellaneous
 *            utilities.  MySQL equivalent of pgutil.c.
 */

#include "localdef.h"
#include "myapi.h"
#include "mymsg.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

/* =========================================================================
   msleep — portable millisecond sleep
   ========================================================================= */
void msleep(int msec)
{
#ifdef WIN32
    Sleep(msec);
#else
    struct timeval sleeptime;
    sleeptime.tv_sec  = msec / 1000;
    sleeptime.tv_usec = (msec % 1000) * 1000;
    select(0, 0, 0, 0, &sleeptime);
#endif
}

/* =========================================================================
   Character encoding helpers
   (same logic as pgutil.c — UTF-8 ↔ Windows wide char)
   ========================================================================= */
static int _oci_replace_wave_dash = 1;
unsigned int g_oci_code_page = CP_UTF8;

static int oci_str_to_win_str_normal(const ocichar *src, TCHAR *dst, int sz)
{
    return MultiByteToWideChar(g_oci_code_page, 0, (LPCCH)src, -1, dst, sz);
}

static int oci_str_to_win_str_replace_wave_dash(const ocichar *src,
    TCHAR *dst, int sz)
{
    int r = MultiByteToWideChar(g_oci_code_page, 0, (LPCCH)src, -1, dst, sz);

    /* Convert Unicode wave-dash (U+301C) → fullwidth tilde (U+FF5E) */
    if (dst) {
        TCHAR *p = dst;
        for (; *p; p++) {
            if (*p == 0x301c) *p = 0xff5e;
        }
    }
    return r;
}

static int (*oci_str_to_win_str_func)(const ocichar *src, TCHAR *dst, int sz)
    = oci_str_to_win_str_normal;

static void set_oci_str_to_win_str_func(void)
{
    oci_str_to_win_str_func = oci_str_to_win_str_normal;
    if (_oci_replace_wave_dash)
        oci_str_to_win_str_func = oci_str_to_win_str_replace_wave_dash;
}

void oci_set_replace_wave_dash(int flg)
{
    _oci_replace_wave_dash = flg;
    set_oci_str_to_win_str_func();
}

int oci_str_to_win_str(const ocichar *src, TCHAR *dst, int sz)
{
    if (src == NULL) {
        if (dst != NULL && sz > 0) dst[0] = _T('\0');
        return 0;
    }
    return oci_str_to_win_str_func(src, dst, sz);
}

int win_str_to_oci_str(const TCHAR *src, ocichar *dst, int sz)
{
    return WideCharToMultiByte(g_oci_code_page, 0, src, -1,
        (LPSTR)dst, sz, NULL, NULL);
}

/* =========================================================================
   Version parsing
   ========================================================================= */
static int _get_remote_version(HMySession ss, TCHAR *msg_buf)
{
    const char *ver_str;
    TCHAR       ver_tchar[256];
    int         vmaj = 0, vmin = 0, vrev = 0;

    (void)msg_buf;

    ver_str = fp_mysql_get_server_info(ss->conn);
    if (ver_str == NULL) return 1;

    oci_str_to_win_str((const ocichar *)ver_str, ver_tchar,
        MY_ARRAY_SIZEOF(ver_tchar));

    swscanf(ver_tchar, _T("%d.%d.%d"), &vmaj, &vmin, &vrev);
    ss->remote_version = (100 * vmaj + vmin) * 100 + vrev;
    return 0;
}

/* =========================================================================
   Session lifecycle helpers
   ========================================================================= */
static void my_init_session(HMySession ss)
{
    ss->conn           = NULL;
    ss->notice_proc    = NULL;
    ss->notice_arg     = NULL;
    ss->remote_version = 0;
    ss->auto_commit_off = 0;
    ss->thread_id      = 0;
    ss->user           = NULL;
    ss->host           = NULL;
    ss->db             = NULL;
    ss->port           = NULL;
    ss->charset        = NULL;
    ss->passwd         = NULL;
}

static void my_free_session(HMySession ss)
{
    if (ss == NULL) return;
    if (ss->user)   free(ss->user);
    if (ss->host)   free(ss->host);
    if (ss->db)     free(ss->db);
    if (ss->port)   free(ss->port);
    if (ss->charset) free(ss->charset);
    if (ss->passwd) free(ss->passwd);
    free(ss);
}

/* =========================================================================
   my_login — connect to MySQL using individual parameters
   ========================================================================= */
HMySession my_login(const TCHAR *host, const TCHAR *user, const TCHAR *passwd,
    const TCHAR *dbname, const TCHAR *port, const TCHAR *charset,
    TCHAR *msg_buf)
{
    HMySession ss = NULL;
    char host_buf[256]    = "localhost";
    char user_buf[256]    = "";
    char passwd_buf[512]  = "";
    char db_buf[256]      = "";
    char port_str[32]     = "3306";
    char charset_buf[64]  = "utf8mb4";
    unsigned int port_num = 3306;

    /* Convert wide-char parameters to UTF-8.
     * win_str_to_oci_str writes to a plain char buffer, so pass the full
     * byte count (sizeof) not a halved value. */
    if (host   && *host)   win_str_to_oci_str(host,   (ocichar *)host_buf,   (int)sizeof(host_buf));
    if (user)              win_str_to_oci_str(user,   (ocichar *)user_buf,   (int)sizeof(user_buf));
    if (passwd)            win_str_to_oci_str(passwd, (ocichar *)passwd_buf, (int)sizeof(passwd_buf));
    if (dbname)            win_str_to_oci_str(dbname, (ocichar *)db_buf,     (int)sizeof(db_buf));
    if (port && *port) {
        win_str_to_oci_str(port, (ocichar *)port_str, (int)sizeof(port_str));
        port_num = (unsigned int)atoi(port_str);
        if (port_num == 0) port_num = 3306;
    }
    if (charset && *charset)
        win_str_to_oci_str(charset, (ocichar *)charset_buf, (int)sizeof(charset_buf));

    ss = (HMySession)malloc(sizeof(MySession));
    if (ss == NULL) {
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return NULL;
    }
    my_init_session(ss);

    ss->conn = fp_mysql_init(NULL);
    if (ss->conn == NULL) {
        my_free_session(ss);
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return NULL;
    }

    if (fp_mysql_real_connect(ss->conn,
            host_buf, user_buf, passwd_buf, db_buf,
            port_num, NULL, 0) == NULL)
    {
        if (msg_buf != NULL) {
            TCHAR errbuf[1024];
            _stprintf(msg_buf, MYERR_NOT_LOGIN_MSG);
            oci_str_to_win_str(
                (const ocichar *)fp_mysql_error(ss->conn),
                errbuf, MY_ARRAY_SIZEOF(errbuf));
            _tcscat(msg_buf, _T("\n"));
            _tcscat(msg_buf, errbuf);
        }
        fp_mysql_close(ss->conn);
        ss->conn = NULL;
        my_free_session(ss);
        return NULL;
    }

    /* Set character set (default: utf8mb4) */
    fp_mysql_set_character_set(ss->conn, charset_buf);

    /* Cache connection metadata as wide strings */
    {
        TCHAR tbuf[512];
        oci_str_to_win_str((const ocichar *)user_buf,    tbuf, MY_ARRAY_SIZEOF(tbuf)); ss->user    = _tcsdup(tbuf);
        oci_str_to_win_str((const ocichar *)host_buf,    tbuf, MY_ARRAY_SIZEOF(tbuf)); ss->host    = _tcsdup(tbuf);
        oci_str_to_win_str((const ocichar *)db_buf,      tbuf, MY_ARRAY_SIZEOF(tbuf)); ss->db      = _tcsdup(tbuf);
        oci_str_to_win_str((const ocichar *)port_str,    tbuf, MY_ARRAY_SIZEOF(tbuf)); ss->port    = _tcsdup(tbuf);
        oci_str_to_win_str((const ocichar *)charset_buf, tbuf, MY_ARRAY_SIZEOF(tbuf)); ss->charset = _tcsdup(tbuf);
    }

    /* Keep password for KILL QUERY reconnect */
    ss->passwd = passwd ? _tcsdup(passwd) : _tcsdup(_T(""));

    ss->thread_id = fp_mysql_thread_id(ss->conn);

    _get_remote_version(ss, msg_buf);

    return ss;
}

/* =========================================================================
   my_logout
   ========================================================================= */
void my_logout(HMySession ss)
{
    if (ss == NULL) return;
    if (ss->conn != NULL) {
        fp_mysql_close(ss->conn);
        ss->conn = NULL;
    }
    my_free_session(ss);
}

/* =========================================================================
   Simple accessors
   ========================================================================= */
const TCHAR *my_user(HMySession ss) { return ss->user; }
const TCHAR *my_host(HMySession ss) { return ss->host; }
const TCHAR *my_db(HMySession ss)   { return ss->db;   }
const TCHAR *my_port(HMySession ss) { return ss->port; }

int my_get_remote_version(HMySession ss)
{
    return ss->remote_version;
}

/* =========================================================================
   Transaction control
   ========================================================================= */
int my_auto_commit_off(HMySession ss, TCHAR *msg_buf)
{
    if (fp_mysql_autocommit(ss->conn, 0) != 0) {
        my_err_msg(ss, msg_buf);
        return 1;
    }
    ss->auto_commit_off = 1;
    return 0;
}

int my_commit(HMySession ss, TCHAR *msg_buf)
{
    if (fp_mysql_commit(ss->conn) != 0) {
        my_err_msg(ss, msg_buf);
        return 1;
    }
    return 0;
}

int my_rollback(HMySession ss, TCHAR *msg_buf)
{
    if (fp_mysql_rollback(ss->conn) != 0) {
        my_err_msg(ss, msg_buf);
        return 1;
    }
    return 0;
}

/* =========================================================================
   my_set_notice_processor
   ========================================================================= */
my_notice_processor my_set_notice_processor(HMySession ss,
    my_notice_processor proc, void *arg)
{
    my_notice_processor old = ss->notice_proc;
    ss->notice_proc = proc;
    ss->notice_arg  = arg;
    return old;
}

/* =========================================================================
   my_explain_plan — execute "EXPLAIN <sql>" and return formatted text.
   Caller must free() the returned buffer when done.
   ========================================================================= */
const TCHAR *my_explain_plan(HMySession ss, const TCHAR *sql, TCHAR *msg_buf)
{
    TCHAR      *explain_sql = NULL;
    HMyDataset  dset        = NULL;
    TCHAR      *plan        = NULL;
    size_t      sql_len;
    size_t      buf_size;
    int         r, c;

    /* "EXPLAIN " prefix (8 chars) + original sql + NUL + small margin */
#define EXPLAIN_PREFIX      _T("EXPLAIN ")
#define EXPLAIN_PREFIX_LEN  8   /* length of "EXPLAIN " */

    sql_len = _tcslen(sql);
    explain_sql = (TCHAR *)malloc((sql_len + EXPLAIN_PREFIX_LEN + 2) * sizeof(TCHAR));
    if (explain_sql == NULL) {
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return NULL;
    }
    _stprintf(explain_sql, _T("%s%s"), EXPLAIN_PREFIX, sql);

    dset = my_create_dataset(ss, explain_sql, msg_buf);
    free(explain_sql);
    if (dset == NULL) return NULL;

    /* Calculate buffer size: "colname: value\t...colname: value\n" per row */
    buf_size = 1; /* for '\0' */
    for (r = 0; r < my_dataset_row_cnt(dset); r++) {
        for (c = 0; c < my_dataset_col_cnt(dset); c++) {
            buf_size += _tcslen(my_dataset_get_colname(dset, c)) + 2; /* ": " */
            buf_size += _tcslen(my_dataset_data(dset, r, c))    + 1; /* tab or newline */
        }
        buf_size += 1; /* newline */
    }

    plan = (TCHAR *)malloc(buf_size * sizeof(TCHAR));
    if (plan == NULL) {
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        my_free_dataset(dset);
        return NULL;
    }
    _tcscpy(plan, _T(""));

    for (r = 0; r < my_dataset_row_cnt(dset); r++) {
        for (c = 0; c < my_dataset_col_cnt(dset); c++) {
            if (c > 0) _tcscat(plan, _T("\t"));
            _tcscat(plan, my_dataset_get_colname(dset, c));
            _tcscat(plan, _T(": "));
            _tcscat(plan, my_dataset_data(dset, r, c));
        }
        _tcscat(plan, _T("\n"));
    }

    my_free_dataset(dset);
    return plan;
}

/* =========================================================================
   my_parameter_status — SHOW VARIABLES LIKE 'paramName'
   ========================================================================= */
int my_parameter_status(HMySession ss, const TCHAR *paramName,
    TCHAR *buf, int buf_size, TCHAR *msg_buf)
{
    TCHAR      sql[512];
    HMyDataset dset;

    _stprintf(sql, _T("SHOW VARIABLES LIKE '%s'"), paramName);
    dset = my_create_dataset(ss, sql, msg_buf);
    if (dset == NULL) {
        if (buf_size > 0) buf[0] = _T('\0');
        return 0; /* not-found is non-fatal */
    }

    if (my_dataset_row_cnt(dset) > 0) {
        /* SHOW VARIABLES returns: Variable_name | Value  (col 0 and 1) */
        _tcsncpy(buf, my_dataset_data(dset, 0, 1), buf_size - 1);
        buf[buf_size - 1] = _T('\0');
    } else {
        if (buf_size > 0) buf[0] = _T('\0');
    }

    my_free_dataset(dset);
    return 0;
}
