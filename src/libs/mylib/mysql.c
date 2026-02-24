/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

/*
 * mysql.c — SQL execution (exec-only path, no result-set returned to caller).
 *           MySQL equivalent of pgsql.c.
 *
 * For async cancellation support the query is executed in a worker thread.
 * Cancellation is delivered by opening a second temporary connection and
 * issuing "KILL QUERY <thread_id>", which causes the blocked mysql_query()
 * call in the worker to return an ER_QUERY_INTERRUPTED error.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "localdef.h"
#include "myapi.h"
#include "mymsg.h"

#ifdef WIN32
#undef ERROR
#include <windows.h>
#endif

/* =========================================================================
   my_kill_query — open a disposable connection and send KILL QUERY.
   Declared in localdef.h so mydataset.c can also call it.
   ========================================================================= */
void my_kill_query(HMySession ss)
{
    MYSQL        *kill_conn;
    char          kill_sql[64];
    int           kill_sql_len;
    char          host_buf[256]   = "localhost";
    char          user_buf[256]   = "";
    char          passwd_buf[512] = "";
    char          db_buf[256]     = "";
    char          port_buf[32]    = "3306";
    unsigned int  port_num        = 3306;

    if (ss->host)   win_str_to_oci_str(ss->host,   (ocichar *)host_buf,   (int)sizeof(host_buf));
    if (ss->user)   win_str_to_oci_str(ss->user,   (ocichar *)user_buf,   (int)sizeof(user_buf));
    if (ss->passwd) win_str_to_oci_str(ss->passwd, (ocichar *)passwd_buf, (int)sizeof(passwd_buf));
    if (ss->db)     win_str_to_oci_str(ss->db,     (ocichar *)db_buf,     (int)sizeof(db_buf));
    if (ss->port) {
        win_str_to_oci_str(ss->port, (ocichar *)port_buf, (int)sizeof(port_buf));
        port_num = (unsigned int)atoi(port_buf);
        if (port_num == 0) port_num = 3306;
    }

    kill_conn = fp_mysql_init(NULL);
    if (kill_conn == NULL) return;

    if (fp_mysql_real_connect(kill_conn, host_buf, user_buf, passwd_buf,
            db_buf, port_num, NULL, 0) == NULL) {
        fp_mysql_close(kill_conn);
        return;
    }

    /* Validate thread_id before constructing KILL QUERY */
    if (ss->thread_id == 0 || ss->thread_id > 0xFFFFFFFFUL) {
        fp_mysql_close(kill_conn);
        return;
    }

    kill_sql_len = snprintf(kill_sql, sizeof(kill_sql), "KILL QUERY %lu", (unsigned long)ss->thread_id);
    if (kill_sql_len < 0 || kill_sql_len >= (int)sizeof(kill_sql)) {
        fp_mysql_close(kill_conn);
        return;
    }
    fp_mysql_query(kill_conn, kill_sql);
    fp_mysql_close(kill_conn);
}

/* =========================================================================
   Worker thread data and entry point
   ========================================================================= */
#ifdef WIN32
typedef struct {
    MYSQL      *conn;
    const char *sql;
    int         result;
    volatile int done;
} ExecSqlThreadData;

static DWORD WINAPI exec_sql_thread_func(LPVOID arg)
{
    ExecSqlThreadData *data = (ExecSqlThreadData *)arg;
    data->result = fp_mysql_query(data->conn, data->sql);
    data->done   = 1;
    return 0;
}
#endif /* WIN32 */

/* =========================================================================
   my_exec_sql_ex — execute SQL; support asynchronous cancellation.
   ========================================================================= */
int my_exec_sql_ex(HMySession ss, const TCHAR *sql, TCHAR *msg_buf,
    volatile int *cancel_flg)
{
    char        *sql_buf;
    int          buf_size;
    int          query_result = 0;
    MYSQL_RES   *res          = NULL;

    if (msg_buf != NULL) _tcscpy(msg_buf, _T(""));

    /* Convert SQL to UTF-8 */
    buf_size = win_str_to_oci_str(sql, NULL, 0);
    sql_buf  = (char *)malloc((size_t)buf_size);
    if (sql_buf == NULL) {
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return 1;
    }
    win_str_to_oci_str(sql, (ocichar *)sql_buf, buf_size);

    /* Snapshot the connection thread-id so KILL QUERY knows which thread. */
    ss->thread_id = fp_mysql_thread_id(ss->conn);

#ifdef WIN32
    if (cancel_flg != NULL) {
        ExecSqlThreadData td;
        HANDLE            hThread;

        td.conn   = ss->conn;
        td.sql    = sql_buf;
        td.result = 0;
        td.done   = 0;

        hThread = CreateThread(NULL, 0, exec_sql_thread_func, &td, 0, NULL);
        if (hThread != NULL) {
            while (!td.done) {
                /* Pause while cancel dialog is visible (cancel_flg == 2) */
                if (*cancel_flg == 2) { msleep(500); continue; }

                if (*cancel_flg == 1) {
                    my_kill_query(ss);
                    WaitForSingleObject(hThread, 5000);
                    CloseHandle(hThread);
                    free(sql_buf);
                    if (msg_buf != NULL) _stprintf(msg_buf, MYERR_CANCEL_MSG);
                    return MYERR_CANCEL;
                }
                msleep(10);
            }
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
            query_result = td.result;
        } else {
            /* Thread creation failed — fall back to synchronous */
            query_result = fp_mysql_query(ss->conn, sql_buf);
        }
    } else {
        query_result = fp_mysql_query(ss->conn, sql_buf);
    }
#else
    query_result = fp_mysql_query(ss->conn, sql_buf);
#endif /* WIN32 */

    free(sql_buf);

    if (query_result != 0) {
        my_err_msg(ss, msg_buf);
        return 1;
    }

    /* Consume any pending result set (e.g. from a SELECT inside a proc). */
    res = fp_mysql_store_result(ss->conn);
    if (res != NULL) {
        fp_mysql_free_result(res);
    }

    /* Build success message */
    if (msg_buf != NULL) {
        my_ulonglong affected = fp_mysql_affected_rows(ss->conn);
        const char  *info     = fp_mysql_info ? fp_mysql_info(ss->conn) : NULL;

        if (info != NULL && *info != '\0') {
            oci_str_to_win_str((const ocichar *)info, msg_buf, 512);
        } else if (affected == (my_ulonglong)-1 || affected == 0) {
            _stprintf(msg_buf, MYMSG_SQL_OK);
        } else {
            char  nrow_buf[64];
            TCHAR nrow_tchar[64];
            sprintf(nrow_buf, "%llu", (unsigned long long)affected);
            oci_str_to_win_str((const ocichar *)nrow_buf, nrow_tchar,
                MY_ARRAY_SIZEOF(nrow_tchar));
            _stprintf(msg_buf, MYMSG_NROW_OK_MSG, nrow_tchar);
        }
    }

    return 0;
}

/* =========================================================================
   my_exec_sql — synchronous wrapper
   ========================================================================= */
int my_exec_sql(HMySession ss, const TCHAR *sql, TCHAR *msg_buf)
{
    return my_exec_sql_ex(ss, sql, msg_buf, NULL);
}

/* =========================================================================
   my_notice — call SHOW WARNINGS and deliver each row to notice_proc.
   ========================================================================= */
int my_notice(HMySession ss, TCHAR *msg_buf)
{
    HMyDataset dset;
    int        r;

    if (ss->notice_proc == NULL) return 0;

    dset = my_create_dataset(ss, _T("SHOW WARNINGS"), msg_buf);
    if (dset == NULL) return 0; /* no warnings is not an error */

    /* SHOW WARNINGS columns: Level | Code | Message */
    for (r = 0; r < my_dataset_row_cnt(dset); r++) {
        TCHAR notice_buf[2048];
        _stprintf(notice_buf, _T("%s %s: %s"),
            my_dataset_data(dset, r, 0),
            my_dataset_data(dset, r, 1),
            my_dataset_data(dset, r, 2));
        ss->notice_proc(ss->notice_arg, notice_buf);
    }

    my_free_dataset(dset);
    return 0;
}
