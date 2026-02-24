/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdafx.h>
#include "mylib.h"
#include "mymsg.h"

#include "localsql.h"
#include "editdata.h"
#include "mbutil.h"
#include "getsrc.h"

/*
 * MySQLのSHOW CREATE文を使ってオブジェクトのDDLを取得し、edit_dataに書き込む
 * SHOW CREATE TABLE  -> col 1 (インデックス1)
 * SHOW CREATE VIEW   -> col 1
 * SHOW CREATE FUNCTION  -> col 2
 * SHOW CREATE PROCEDURE -> col 2
 * SHOW CREATE TRIGGER   -> col 2
 * SHOW CREATE EVENT     -> col 3
 */

static int exec_show_create(HMySession ss, const TCHAR *sql, int col_idx,
CEditData *edit_data, TCHAR *msg_buf)
{
HMyDataset dataset = NULL;
int ret_v = 0;

dataset = my_create_dataset(ss, sql, msg_buf);
if(dataset == NULL) return 1;

if(my_dataset_row_cnt(dataset) == 0) {
_stprintf(msg_buf, _T("no result"));
ret_v = 1;
goto END;
}

if(col_idx >= my_dataset_col_cnt(dataset)) {
_stprintf(msg_buf, _T("column index out of range"));
ret_v = 1;
goto END;
}

{
const TCHAR *src = my_dataset_data(dataset, 0, col_idx);
if(src != NULL && _tcslen(src) > 0) {
edit_data->paste(src);
}
}

END:
my_free_dataset(dataset);
return ret_v;
}

int create_object_source(HMySession ss, const TCHAR *object_type, const TCHAR *oid,
const TCHAR *object_name, const TCHAR *schema,
CEditData *edit_data, TCHAR *msg_buf, BOOL recalc_disp_size)
{
int ret_v = 0;
TCHAR sql_buf[4096];

INDENT_MODE indent_mode = edit_data->get_indent_mode();
edit_data->set_indent_mode(INDENT_MODE_NONE);

if(_tcscmp(object_type, _T("TABLE")) == 0) {
_stprintf(sql_buf, _T("SHOW CREATE TABLE `%s`.`%s`"), schema, object_name);
ret_v = exec_show_create(ss, sql_buf, 1, edit_data, msg_buf);
} else if(_tcscmp(object_type, _T("VIEW")) == 0) {
_stprintf(sql_buf, _T("SHOW CREATE VIEW `%s`.`%s`"), schema, object_name);
ret_v = exec_show_create(ss, sql_buf, 1, edit_data, msg_buf);
} else if(_tcscmp(object_type, _T("FUNCTION")) == 0) {
_stprintf(sql_buf, _T("SHOW CREATE FUNCTION `%s`.`%s`"), schema, object_name);
ret_v = exec_show_create(ss, sql_buf, 2, edit_data, msg_buf);
} else if(_tcscmp(object_type, _T("PROCEDURE")) == 0) {
_stprintf(sql_buf, _T("SHOW CREATE PROCEDURE `%s`.`%s`"), schema, object_name);
ret_v = exec_show_create(ss, sql_buf, 2, edit_data, msg_buf);
} else if(_tcscmp(object_type, _T("TRIGGER")) == 0) {
_stprintf(sql_buf, _T("SHOW CREATE TRIGGER `%s`.`%s`"), schema, object_name);
ret_v = exec_show_create(ss, sql_buf, 2, edit_data, msg_buf);
} else if(_tcscmp(object_type, _T("EVENT")) == 0) {
_stprintf(sql_buf, _T("SHOW CREATE EVENT `%s`.`%s`"), schema, object_name);
ret_v = exec_show_create(ss, sql_buf, 3, edit_data, msg_buf);
} else if(_tcscmp(object_type, _T("INDEX")) == 0) {
// MySQLではインデックスのDDLはCREATE TABLEの中に含まれる
// INDEX リストのtable_nameカラムを取得してCREATE TABLEを表示する
TCHAR idx_sql[4096];
_stprintf(idx_sql,
_T("SELECT table_name FROM information_schema.statistics \n")
_T("WHERE table_schema = '%s' AND index_name = '%s' LIMIT 1"),
schema, object_name);

HMyDataset ds = my_create_dataset(ss, idx_sql, msg_buf);
if(ds != NULL && my_dataset_row_cnt(ds) > 0) {
const TCHAR *tbl = my_dataset_data(ds, 0, 0);
_stprintf(sql_buf, _T("SHOW CREATE TABLE `%s`.`%s`"), schema, tbl);
ret_v = exec_show_create(ss, sql_buf, 1, edit_data, msg_buf);
} else {
ret_v = 1;
}
my_free_dataset(ds);
} else if(_tcscmp(object_type, _T("TABLE|VIEW")) == 0) {
// TABLE|VIEW はTABLEとして扱う
_stprintf(sql_buf, _T("SHOW CREATE TABLE `%s`.`%s`"), schema, object_name);
ret_v = exec_show_create(ss, sql_buf, 1, edit_data, msg_buf);
}

edit_data->set_indent_mode(indent_mode);

if(recalc_disp_size) {
edit_data->recalc_disp_size();
edit_data->set_cur(0, 0);
edit_data->reset_undo();
}

return ret_v;
}
