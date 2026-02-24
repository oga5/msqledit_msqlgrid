/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

/*
 * mydataset.c — fetch a MySQL result set and convert it to an HMyDataset.
 *               MySQL equivalent of pgdataset.c.
 *
 * All field values are decoded from UTF-8 to TCHAR and packed into a single
 * heap allocation (dataset->buf), so the MYSQL_RES is freed immediately
 * after the conversion.  This means the dataset lives independently of the
 * connection and can be passed to other threads.
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
   my_free_dataset
   ========================================================================= */
void my_free_dataset(HMyDataset dataset)
{
    if (dataset == NULL) return;

    if (dataset->buf)      free(dataset->buf);
    if (dataset->col_data) free(dataset->col_data);
    if (dataset->cname)    free(dataset->cname);
    if (dataset->is_null)  free(dataset->is_null);
    if (dataset->col_type) free(dataset->col_type);
    if (dataset->col_size) free(dataset->col_size);

    free(dataset);
}

/* =========================================================================
   my_build_dataset_from_res — convert a MYSQL_RES into HMyDataset.
   The MYSQL_RES is NOT freed here; the caller is responsible.
   Declared in localdef.h so mysql.c can reuse it if needed.
   ========================================================================= */
HMyDataset my_build_dataset_from_res(MYSQL_RES *res, TCHAR *msg_buf)
{
    HMyDataset     dataset  = NULL;
    MYSQL_FIELD   *fields   = NULL;
    MYSQL_ROW     *all_rows = NULL;
    unsigned long *all_lens = NULL;
    TCHAR         *p;
    size_t         memsize;
    int            r, c, cnt;

    if (res == NULL) return NULL;

    dataset = (HMyDataset)malloc(sizeof(MyDataset));
    if (dataset == NULL) {
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return NULL;
    }
    memset(dataset, 0, sizeof(MyDataset));

    dataset->row_cnt = (int)fp_mysql_num_rows(res);
    dataset->col_cnt = (int)fp_mysql_num_fields(res);

    /* Fetch field descriptors (valid for the lifetime of res) */
    fields = fp_mysql_fetch_fields(res);

    /* Allocate per-column metadata arrays */
    dataset->col_type = (unsigned int  *)malloc((size_t)dataset->col_cnt * sizeof(unsigned int));
    dataset->col_size = (unsigned long *)malloc((size_t)dataset->col_cnt * sizeof(unsigned long));
    dataset->cname    = (TCHAR **)       malloc((size_t)dataset->col_cnt * sizeof(TCHAR *));

    /* Flat row*col arrays */
    if (dataset->row_cnt > 0) {
        dataset->col_data = (TCHAR **)malloc((size_t)dataset->row_cnt *
            (size_t)dataset->col_cnt * sizeof(TCHAR *));
        dataset->is_null  = (int *)malloc((size_t)dataset->row_cnt *
            (size_t)dataset->col_cnt * sizeof(int));
    } else {
        /* Zero rows: still allocate minimal arrays for safety */
        dataset->col_data = (TCHAR **)malloc(sizeof(TCHAR *));
        dataset->is_null  = (int *)   malloc(sizeof(int));
    }

    if (!dataset->col_type || !dataset->col_size || !dataset->cname ||
        !dataset->col_data || !dataset->is_null)
    {
        my_free_dataset(dataset);
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return NULL;
    }

    /* Record field metadata */
    for (c = 0; c < dataset->col_cnt; c++) {
        dataset->col_type[c] = fields[c].type;
        dataset->col_size[c] = fields[c].length;
    }

    /* ----------------------------------------------------------------
       Fetch all rows into temporary arrays.
       With mysql_store_result() the row pointers remain valid until
       the MYSQL_RES is freed, so we only need to copy the lengths.
       ---------------------------------------------------------------- */
    all_rows = (MYSQL_ROW *)    malloc((size_t)dataset->row_cnt * sizeof(MYSQL_ROW));
    all_lens = (unsigned long *)malloc((size_t)dataset->row_cnt *
        (size_t)dataset->col_cnt * sizeof(unsigned long));

    if (!all_rows || !all_lens) {
        free(all_rows);
        free(all_lens);
        my_free_dataset(dataset);
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return NULL;
    }

    for (r = 0; r < dataset->row_cnt; r++) {
        all_rows[r] = fp_mysql_fetch_row(res);
        if (all_rows[r] != NULL) {
            unsigned long *lens = fp_mysql_fetch_lengths(res);
            if (lens != NULL) {
                memcpy(&all_lens[r * dataset->col_cnt], lens,
                    (size_t)dataset->col_cnt * sizeof(unsigned long));
            } else {
                memset(&all_lens[r * dataset->col_cnt], 0,
                    (size_t)dataset->col_cnt * sizeof(unsigned long));
            }
        } else {
            memset(&all_lens[r * dataset->col_cnt], 0,
                (size_t)dataset->col_cnt * sizeof(unsigned long));
        }
    }

    /* ----------------------------------------------------------------
       Calculate total TCHAR buffer size.
       oci_str_to_win_str(src, NULL, 0) returns the number of wide
       chars needed including the NUL terminator (same as pglib).
       For SQL-NULL values we reserve 1 TCHAR for the NUL.
       ---------------------------------------------------------------- */
    memsize = 0;
    for (c = 0; c < dataset->col_cnt; c++) {
        int n = oci_str_to_win_str((const ocichar *)fields[c].name, NULL, 0);
        memsize += (n > 0) ? (size_t)n : 1;
    }
    for (r = 0; r < dataset->row_cnt; r++) {
        for (c = 0; c < dataset->col_cnt; c++) {
            if (all_rows[r] != NULL && all_rows[r][c] != NULL) {
                int n = oci_str_to_win_str(
                    (const ocichar *)all_rows[r][c], NULL, 0);
                memsize += (n > 0) ? (size_t)n : 1;
            } else {
                memsize += 1; /* NUL for SQL NULL */
            }
        }
    }
    memsize += 1; /* safety margin */

    dataset->buf = (TCHAR *)malloc(memsize * sizeof(TCHAR));
    if (dataset->buf == NULL) {
        free(all_rows);
        free(all_lens);
        my_free_dataset(dataset);
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return NULL;
    }

    /* ----------------------------------------------------------------
       Fill the buffer: column names first, then row data.
       ---------------------------------------------------------------- */
    p = dataset->buf;

    for (c = 0; c < dataset->col_cnt; c++) {
        dataset->cname[c] = p;
        cnt = oci_str_to_win_str((const ocichar *)fields[c].name, p, (int)memsize);
        if (cnt > 0) p += cnt;
        else         { *p = _T('\0'); p++; }
    }

    for (r = 0; r < dataset->row_cnt; r++) {
        for (c = 0; c < dataset->col_cnt; c++) {
            int idx = r * dataset->col_cnt + c;
            dataset->col_data[idx] = p;

            if (all_rows[r] != NULL && all_rows[r][c] != NULL) {
                dataset->is_null[idx] = 0;
                cnt = oci_str_to_win_str(
                    (const ocichar *)all_rows[r][c], p, (int)memsize);
                if (cnt > 0) p += cnt;
                else         { *p = _T('\0'); p++; }
            } else {
                dataset->is_null[idx] = 1;
                *p = _T('\0');
                p++;
            }
        }
    }

    free(all_rows);
    free(all_lens);
    return dataset;
}

/* =========================================================================
   Worker thread data and entry point for async query + store_result
   ========================================================================= */
#ifdef WIN32
typedef struct {
    MYSQL      *conn;
    const char *sql;
    int         query_result;
    MYSQL_RES  *res;
    volatile int done;
} DatasetThreadData;

static DWORD WINAPI dataset_thread_func(LPVOID arg)
{
    DatasetThreadData *data = (DatasetThreadData *)arg;
    data->query_result = fp_mysql_query(data->conn, data->sql);
    if (data->query_result == 0)
        data->res = fp_mysql_store_result(data->conn);
    data->done = 1;
    return 0;
}
#endif /* WIN32 */

/* =========================================================================
   my_create_dataset_ex
   ========================================================================= */
int my_create_dataset_ex(HMySession ss, const TCHAR *sql,
    TCHAR *msg_buf, volatile int *cancel_flg, void *hWnd,
    HMyDataset *result)
{
    char      *sql_buf;
    int        buf_size;
    int        query_result = 0;
    MYSQL_RES *res          = NULL;

    *result = NULL;
    if (msg_buf != NULL) _tcscpy(msg_buf, _T(""));

    /* Convert SQL to UTF-8 */
    buf_size = win_str_to_oci_str(sql, NULL, 0);
    sql_buf  = (char *)malloc((size_t)buf_size);
    if (sql_buf == NULL) {
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_MEM_ALLOC_MSG);
        return 1;
    }
    win_str_to_oci_str(sql, (ocichar *)sql_buf, buf_size);

    ss->thread_id = fp_mysql_thread_id(ss->conn);

#ifdef WIN32
    if (cancel_flg != NULL) {
        DatasetThreadData td;
        HANDLE            hThread;

        td.conn         = ss->conn;
        td.sql          = sql_buf;
        td.query_result = 0;
        td.res          = NULL;
        td.done         = 0;

        hThread = CreateThread(NULL, 0, dataset_thread_func, &td, 0, NULL);
        if (hThread != NULL) {
            while (!td.done) {
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

#ifdef WIN32
                if (hWnd != NULL && td.done) {
                    /* Will post final count after we fall through */
                }
#endif
            }
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
            query_result = td.query_result;
            res          = td.res;
        } else {
            /* Thread creation failed — synchronous fallback */
            query_result = fp_mysql_query(ss->conn, sql_buf);
            if (query_result == 0)
                res = fp_mysql_store_result(ss->conn);
        }
    } else {
        query_result = fp_mysql_query(ss->conn, sql_buf);
        if (query_result == 0)
            res = fp_mysql_store_result(ss->conn);
    }
#else
    query_result = fp_mysql_query(ss->conn, sql_buf);
    if (query_result == 0)
        res = fp_mysql_store_result(ss->conn);
#endif /* WIN32 */

    free(sql_buf);

    if (query_result != 0) {
        my_err_msg(ss, msg_buf);
        return 1;
    }

    if (res == NULL) {
        /* Non-SELECT statement (DDL / DML) — no rows */
        if (fp_mysql_field_count(ss->conn) == 0) {
            if (msg_buf != NULL) {
                my_ulonglong affected = fp_mysql_affected_rows(ss->conn);
                const char  *info     = fp_mysql_info
                                        ? fp_mysql_info(ss->conn) : NULL;

                if (info != NULL && *info != '\0') {
                    oci_str_to_win_str((const ocichar *)info, msg_buf, 512);
                } else if (affected == (my_ulonglong)-1 || affected == 0) {
                    _stprintf(msg_buf, MYMSG_SQL_OK);
                } else {
                    char  nrow_buf[64];
                    TCHAR nrow_tchar[64];
                    sprintf(nrow_buf, "%llu", (unsigned long long)affected);
                    oci_str_to_win_str((const ocichar *)nrow_buf,
                        nrow_tchar, MY_ARRAY_SIZEOF(nrow_tchar));
                    _stprintf(msg_buf, MYMSG_NROW_OK_MSG, nrow_tchar);
                }
            }
            *result = NULL;
        } else {
            /* Expected a result set but got nothing */
            my_err_msg(ss, msg_buf);
            return 1;
        }
    } else {
        /* SELECT query — build dataset, then free the result */
        HMyDataset dataset = my_build_dataset_from_res(res, msg_buf);
        fp_mysql_free_result(res);

        if (dataset == NULL) return 1;

        if (msg_buf != NULL)
            _stprintf(msg_buf, MYMSG_SELECT_MSG, dataset->row_cnt);

#ifdef WIN32
        if (hWnd != NULL)
            PostMessage((HWND)hWnd, WM_OCI_DLG_ROW_CNT, 0,
                (LPARAM)dataset->row_cnt);
#endif
        *result = dataset;
    }

    return 0;
}

/* =========================================================================
   my_create_dataset — synchronous, no cancel support
   ========================================================================= */
HMyDataset my_create_dataset(HMySession ss, const TCHAR *sql, TCHAR *msg_buf)
{
    HMyDataset dataset = NULL;
    my_create_dataset_ex(ss, sql, msg_buf, NULL, NULL, &dataset);
    return dataset;
}

/* =========================================================================
   Accessors
   ========================================================================= */
int my_dataset_row_cnt(HMyDataset dataset) { return dataset->row_cnt; }
int my_dataset_col_cnt(HMyDataset dataset) { return dataset->col_cnt; }

const TCHAR *my_dataset_data(HMyDataset dataset, int row, int col)
{
    return dataset->col_data[row * dataset->col_cnt + col];
}

size_t my_dataset_len(HMyDataset dataset, int row, int col)
{
    return _tcslen(my_dataset_data(dataset, row, col));
}

const TCHAR *my_dataset_get_colname(HMyDataset dataset, int col)
{
    return dataset->cname[col];
}

int my_dataset_get_colsize(HMyDataset dataset, int col)
{
    if (dataset->col_size == NULL) return 0;
    return (int)dataset->col_size[col];
}

my_type my_dataset_get_coltype(HMyDataset dataset, int col)
{
    if (dataset->col_type == NULL) return 0;
    return dataset->col_type[col];
}

int my_dataset_get_col_no(HMyDataset dataset, const TCHAR *colname)
{
    int c;
    for (c = 0; c < dataset->col_cnt; c++) {
        if (_tcsicmp(dataset->cname[c], colname) == 0) return c;
    }
    return -1;
}

const TCHAR *my_dataset_data2(HMyDataset dataset, int row,
    const TCHAR *colname)
{
    int col = my_dataset_get_col_no(dataset, colname);
    if (col < 0) return NULL;
    return my_dataset_data(dataset, row, col);
}

int my_dataset_is_null(HMyDataset dataset, int row, int col)
{
    if (dataset->is_null == NULL) return 0;
    return dataset->is_null[row * dataset->col_cnt + col];
}

int my_dataset_is_null2(HMyDataset dataset, int row, const TCHAR *colname)
{
    int col = my_dataset_get_col_no(dataset, colname);
    if (col < 0) return 1; /* not found → treat as NULL */
    return my_dataset_is_null(dataset, row, col);
}

int my_dataset_is_valid(HMyDataset dataset)
{
    if (dataset == NULL)           return 0;
    if (dataset->col_data == NULL) return 0;
    return 1;
}
