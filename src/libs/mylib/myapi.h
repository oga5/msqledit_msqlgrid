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

#ifndef _MYLIB_MY_API_H_INCLUDE_
#define _MYLIB_MY_API_H_INCLUDE_

#include <tchar.h>
#include <stdio.h>

typedef unsigned int my_type;  /* MYSQL_TYPE_* values */

/* Notice processor callback type */
#ifndef MY_NOTICE_PROCESSOR_DEFINED
#define MY_NOTICE_PROCESSOR_DEFINED
typedef void (*my_notice_processor)(void *arg, const TCHAR *message);
#endif

/* myutil.c */
HMySession my_login(const TCHAR *host, const TCHAR *user, const TCHAR *passwd,
    const TCHAR *dbname, const TCHAR *port, const TCHAR *charset, TCHAR *msg_buf);
void my_logout(HMySession ss);
const TCHAR *my_user(HMySession ss);
const TCHAR *my_host(HMySession ss);
const TCHAR *my_db(HMySession ss);
const TCHAR *my_port(HMySession ss);
const TCHAR *my_explain_plan(HMySession ss, const TCHAR *sql, TCHAR *msg_buf);
int my_get_remote_version(HMySession ss);

my_notice_processor my_set_notice_processor(HMySession ss,
    my_notice_processor proc, void *arg);

int my_auto_commit_off(HMySession ss, TCHAR *msg_buf);
int my_rollback(HMySession ss, TCHAR *msg_buf);
int my_commit(HMySession ss, TCHAR *msg_buf);
int my_trans_is_idle(HMySession ss);

int my_parameter_status(HMySession ss, const TCHAR *paramName,
    TCHAR *buf, int buf_size, TCHAR *msg_buf);

extern unsigned int g_oci_code_page;
typedef unsigned char ocichar;
int oci_str_to_win_str(const ocichar *src, TCHAR *dst, int sz);
int win_str_to_oci_str(const TCHAR *src, ocichar *dst, int sz);

/* myerr.c */
void my_err_msg(HMySession ss, TCHAR *msg_buf);

/* mysql.c */
int my_exec_sql(HMySession ss, const TCHAR *sql, TCHAR *msg_buf);
int my_exec_sql_ex(HMySession ss, const TCHAR *sql, TCHAR *msg_buf,
    volatile int *cancel_flg);
int my_notice(HMySession ss, TCHAR *msg_buf);

/* mydataset.c */
int my_create_dataset_ex(HMySession ss, const TCHAR *sql,
    TCHAR *msg_buf, volatile int *cancel_flg, void *hWnd,
    HMyDataset *result);
HMyDataset my_create_dataset(HMySession ss, const TCHAR *sql, TCHAR *msg_buf);
void my_free_dataset(HMyDataset dataset);
int my_dataset_row_cnt(HMyDataset dataset);
int my_dataset_col_cnt(HMyDataset dataset);
const TCHAR *my_dataset_data(HMyDataset dataset, int row, int col);
size_t my_dataset_len(HMyDataset dataset, int row, int col);
const TCHAR *my_dataset_data2(HMyDataset dataset, int row, const TCHAR *colname);
const TCHAR *my_dataset_get_colname(HMyDataset dataset, int col);
int my_dataset_get_colsize(HMyDataset dataset, int col);
my_type my_dataset_get_coltype(HMyDataset dataset, int col);
int my_dataset_get_col_no(HMyDataset dataset, const TCHAR *colname);
int my_dataset_is_null(HMyDataset dataset, int row, int col);
int my_dataset_is_null2(HMyDataset dataset, int row, const TCHAR *colname);
int my_dataset_is_valid(HMyDataset dataset);

/* dsetutil.c */
int my_save_dataset_csv(const TCHAR *path, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf);
int my_save_dataset_csv_fp(FILE *fp, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf);
int my_save_dataset_tsv(const TCHAR *path, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf);
int my_save_dataset_ex(const TCHAR *path, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf, int col_cnt, TCHAR sepa);

/* winmy.c */
#ifdef WIN32
int my_init_library(TCHAR *msg_buf);
int my_free_library(TCHAR *msg_buf);
int my_library_is_ok(void);
#endif

#endif /* _MYLIB_MY_API_H_INCLUDE_ */
