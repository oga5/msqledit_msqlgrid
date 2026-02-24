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

// table名からschema名を取得 (MySQL: 現在のデータベースに限定して検索)
CString get_table_schema_name(HMySession ss, const TCHAR *table_name, TCHAR *msg_buf)
{
HMyDataset dataset = NULL;
TCHAR sql_buf[4000];
CString schema_name = _T("");

if(_tcslen(table_name) == 0) {
return _T("");
}

_stprintf(sql_buf,
_T("SELECT table_schema FROM information_schema.tables \n")
_T("WHERE table_name = '%s' AND table_schema = DATABASE() LIMIT 1"),
table_name);

dataset = my_create_dataset(ss, sql_buf, msg_buf);
if(dataset == NULL) return schema_name;

if(my_dataset_row_cnt(dataset) > 0) {
schema_name = my_dataset_data(dataset, 0, 0);
}

my_free_dataset(dataset);
return schema_name;
}


/*------------------------------------------------------------------------------
 データベースの検索
------------------------------------------------------------------------------*/
int get_dataset(HMySession ss, const TCHAR *sql, TCHAR *msg_buf, 
void *hWnd, volatile int *cancel_flg, HMyDataset *result)
{
return my_create_dataset_ex(ss, sql, msg_buf, cancel_flg, hWnd, result);
}

/*------------------------------------------------------------------------------
 SQL実行
------------------------------------------------------------------------------*/
int execute_sql(HMySession ss, const TCHAR *sql, TCHAR *msg_buf,
volatile int *cancel_flg)
{
return my_exec_sql_ex(ss, sql, msg_buf, cancel_flg);
}


/*------------------------------------------------------------------------------
 SQL実行計画の取得
------------------------------------------------------------------------------*/
CString explain_plan(HMySession ss, const TCHAR *sql, TCHAR *msg_buf)
{
const TCHAR*plan;

plan = my_explain_plan(ss, sql, msg_buf);
if(plan == NULL) return _T("");

CString exp_plan = plan;

free((void *)plan);

return exp_plan;
}

static int pg_save_dataset_main_ar(CUnicodeArchive *ar, HMyDataset dataset, int put_colname, TCHAR *msg_buf)
{
int r, c;
int col_cnt = my_dataset_col_cnt(dataset);
int row_cnt = my_dataset_row_cnt(dataset);
const TCHAR*sepa = _T(",");

if(put_colname == 1) {
for(c = 0; c < col_cnt; c++) {
if(c != 0) {
ar->WriteString(sepa);
}
ar->CsvOut(my_dataset_get_colname(dataset, c), TRUE);
}
ar->WriteNextLine();
}

for(r = 0; r < row_cnt; r++) {
for(c = 0; c < col_cnt; c++) {
if(c != 0) {
ar->WriteString(sepa);
}
ar->CsvOut(my_dataset_data(dataset, r, c), TRUE);
}
ar->WriteNextLine();
}

return 0;
}

/*------------------------------------------------------------------------------
 SQL実行結果の保存
------------------------------------------------------------------------------*/
int download(HMySession ss, CUnicodeArchive *ar, const TCHAR *sql, TCHAR *msg_buf,
BOOL put_column_name, void *hWnd, volatile int *cancel_flg)
{
HMyDataset dataset = NULL;
int ret_v;

ret_v = my_create_dataset_ex(ss, sql, msg_buf, cancel_flg, hWnd, &dataset);
if(ret_v != 0) return ret_v;

ret_v = pg_save_dataset_main_ar(ar, dataset, put_column_name, msg_buf);
if(ret_v != 0) return ret_v;

my_free_dataset(dataset);

return ret_v;
}

// データベース一覧を取得 (PostgreSQLのユーザー一覧の代替)
HMyDataset get_user_list(HMySession ss, TCHAR *msg_buf)
{
return my_create_dataset(ss, _T("SHOW DATABASES"), msg_buf);
}

HMyDataset get_object_list(HMySession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf)
{
TCHAR sql_buf[8192];
TCHAR schema_cond[512] = _T("");

// ownerはデータベース名(スキーマ名)として使用する
// ALL_USERSの場合はschema条件を付けない
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(schema_cond, _T("'%s'"), owner);
} else {
_tcscpy(schema_cond, _T("table_schema"));  // ダミー(WHERE外すため)
}

if(_tcscmp(type, _T("TABLE")) == 0) {
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(sql_buf,
_T("SELECT table_name as name, table_schema as nspname, table_comment as comment, table_name as oid \n")
_T("FROM information_schema.tables \n")
_T("WHERE table_type = 'BASE TABLE' AND table_schema = '%s' \n")
_T("ORDER BY table_name"),
owner);
} else {
_tcscpy(sql_buf,
_T("SELECT table_name as name, table_schema as nspname, table_comment as comment, table_name as oid \n")
_T("FROM information_schema.tables \n")
_T("WHERE table_type = 'BASE TABLE' \n")
_T("ORDER BY table_name"));
}
} else if(_tcscmp(type, _T("VIEW")) == 0) {
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(sql_buf,
_T("SELECT table_name as name, table_schema as nspname, view_definition as comment, table_name as oid \n")
_T("FROM information_schema.views \n")
_T("WHERE table_schema = '%s' \n")
_T("ORDER BY table_name"),
owner);
} else {
_tcscpy(sql_buf,
_T("SELECT table_name as name, table_schema as nspname, view_definition as comment, table_name as oid \n")
_T("FROM information_schema.views \n")
_T("ORDER BY table_name"));
}
} else if(_tcscmp(type, _T("TABLE|VIEW")) == 0) {
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(sql_buf,
_T("SELECT table_name as name, table_schema as nspname, table_comment as comment, table_name as oid, 'table' as relkind \n")
_T("FROM information_schema.tables \n")
_T("WHERE table_type = 'BASE TABLE' AND table_schema = '%s' \n")
_T("UNION ALL \n")
_T("SELECT table_name as name, table_schema as nspname, view_definition as comment, table_name as oid, 'view' as relkind \n")
_T("FROM information_schema.views \n")
_T("WHERE table_schema = '%s' \n")
_T("ORDER BY name"),
owner, owner);
} else {
_tcscpy(sql_buf,
_T("SELECT table_name as name, table_schema as nspname, table_comment as comment, table_name as oid, 'table' as relkind \n")
_T("FROM information_schema.tables \n")
_T("WHERE table_type = 'BASE TABLE' \n")
_T("UNION ALL \n")
_T("SELECT table_name as name, table_schema as nspname, view_definition as comment, table_name as oid, 'view' as relkind \n")
_T("FROM information_schema.views \n")
_T("ORDER BY name"));
}
} else if(_tcscmp(type, _T("INDEX")) == 0) {
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(sql_buf,
_T("SELECT index_name as name, table_schema as nspname, table_name, non_unique, index_type, index_name as oid \n")
_T("FROM information_schema.statistics \n")
_T("WHERE table_schema = '%s' \n")
_T("GROUP BY table_schema, table_name, index_name, non_unique, index_type \n")
_T("ORDER BY table_name, index_name"),
owner);
} else {
_tcscpy(sql_buf,
_T("SELECT index_name as name, table_schema as nspname, table_name, non_unique, index_type, index_name as oid \n")
_T("FROM information_schema.statistics \n")
_T("GROUP BY table_schema, table_name, index_name, non_unique, index_type \n")
_T("ORDER BY table_name, index_name"));
}
} else if(_tcscmp(type, _T("FUNCTION")) == 0) {
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(sql_buf,
_T("SELECT routine_name as name, routine_schema as nspname, routine_comment as comment, routine_name as oid \n")
_T("FROM information_schema.routines \n")
_T("WHERE routine_type = 'FUNCTION' AND routine_schema = '%s' \n")
_T("ORDER BY routine_name"),
owner);
} else {
_tcscpy(sql_buf,
_T("SELECT routine_name as name, routine_schema as nspname, routine_comment as comment, routine_name as oid \n")
_T("FROM information_schema.routines \n")
_T("WHERE routine_type = 'FUNCTION' \n")
_T("ORDER BY routine_name"));
}
} else if(_tcscmp(type, _T("PROCEDURE")) == 0) {
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(sql_buf,
_T("SELECT routine_name as name, routine_schema as nspname, routine_comment as comment, routine_name as oid \n")
_T("FROM information_schema.routines \n")
_T("WHERE routine_type = 'PROCEDURE' AND routine_schema = '%s' \n")
_T("ORDER BY routine_name"),
owner);
} else {
_tcscpy(sql_buf,
_T("SELECT routine_name as name, routine_schema as nspname, routine_comment as comment, routine_name as oid \n")
_T("FROM information_schema.routines \n")
_T("WHERE routine_type = 'PROCEDURE' \n")
_T("ORDER BY routine_name"));
}
} else if(_tcscmp(type, _T("TRIGGER")) == 0) {
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(sql_buf,
_T("SELECT trigger_name as name, event_object_schema as nspname, event_object_table, event_manipulation, action_timing, trigger_name as oid \n")
_T("FROM information_schema.triggers \n")
_T("WHERE trigger_schema = '%s' \n")
_T("ORDER BY trigger_name"),
owner);
} else {
_tcscpy(sql_buf,
_T("SELECT trigger_name as name, event_object_schema as nspname, event_object_table, event_manipulation, action_timing, trigger_name as oid \n")
_T("FROM information_schema.triggers \n")
_T("ORDER BY trigger_name"));
}
} else if(_tcscmp(type, _T("EVENT")) == 0) {
if(_tcscmp(owner, ALL_USERS) != 0) {
_stprintf(sql_buf,
_T("SELECT event_name as name, event_schema as nspname, status, execute_at, interval_value, interval_field, event_name as oid \n")
_T("FROM information_schema.events \n")
_T("WHERE event_schema = '%s' \n")
_T("ORDER BY event_name"),
owner);
} else {
_tcscpy(sql_buf,
_T("SELECT event_name as name, event_schema as nspname, status, execute_at, interval_value, interval_field, event_name as oid \n")
_T("FROM information_schema.events \n")
_T("ORDER BY event_name"));
}
} else {
_stprintf(msg_buf, _T("not implement"));
return NULL;
}

return my_create_dataset(ss, sql_buf, msg_buf);
}

HMyDataset get_column_list(HMySession ss, const TCHAR *relname, const TCHAR *schema, TCHAR *msg_buf)
{
TCHAR sql_buf[4096];

if(schema != NULL && _tcslen(schema) > 0) {
_stprintf(sql_buf,
_T("SELECT column_name, column_type, character_maximum_length, is_nullable, column_comment, ordinal_position \n")
_T("FROM information_schema.columns \n")
_T("WHERE table_schema = '%s' AND table_name = '%s' \n")
_T("ORDER BY ordinal_position"),
schema, relname);
} else {
_stprintf(sql_buf,
_T("SELECT column_name, column_type, character_maximum_length, is_nullable, column_comment, ordinal_position \n")
_T("FROM information_schema.columns \n")
_T("WHERE table_schema = DATABASE() AND table_name = '%s' \n")
_T("ORDER BY ordinal_position"),
relname);
}

return my_create_dataset(ss, sql_buf, msg_buf);
}

HMyDataset get_index_list_by_table(HMySession ss, const TCHAR *relname, 
const TCHAR *schema, TCHAR *msg_buf)
{
TCHAR sql_buf[4096];

if(schema != NULL && _tcslen(schema) > 0) {
_stprintf(sql_buf, _T("SHOW INDEX FROM `%s` FROM `%s`"), relname, schema);
} else {
_stprintf(sql_buf, _T("SHOW INDEX FROM `%s`"), relname);
}

return my_create_dataset(ss, sql_buf, msg_buf);
}

HMyDataset get_trigger_list_by_table(HMySession ss, const TCHAR *relname,
const TCHAR *schema, TCHAR *msg_buf)
{
TCHAR sql_buf[4096];

if(schema != NULL && _tcslen(schema) > 0) {
_stprintf(sql_buf,
_T("SELECT trigger_name, event_manipulation, action_timing, action_statement \n")
_T("FROM information_schema.triggers \n")
_T("WHERE trigger_schema = '%s' AND event_object_table = '%s' \n")
_T("ORDER BY trigger_name"),
schema, relname);
} else {
_stprintf(sql_buf,
_T("SELECT trigger_name, event_manipulation, action_timing, action_statement \n")
_T("FROM information_schema.triggers \n")
_T("WHERE trigger_schema = DATABASE() AND event_object_table = '%s' \n")
_T("ORDER BY trigger_name"),
relname);
}

return my_create_dataset(ss, sql_buf, msg_buf);
}

HMyDataset get_object_properties(HMySession ss, const TCHAR *type, const TCHAR *name, 
const TCHAR *schema, TCHAR *msg_buf)
{
TCHAR sql_buf[4096];

if(_tcscmp(type, _T("TABLE")) == 0) {
_stprintf(sql_buf,
_T("SELECT * FROM information_schema.tables \n")
_T("WHERE table_name = '%s' AND table_schema = '%s'"),
name, schema);
} else if(_tcscmp(type, _T("VIEW")) == 0) {
_stprintf(sql_buf,
_T("SELECT * FROM information_schema.views \n")
_T("WHERE table_name = '%s' AND table_schema = '%s'"),
name, schema);
} else if(_tcscmp(type, _T("INDEX")) == 0) {
_stprintf(sql_buf,
_T("SELECT * FROM information_schema.statistics \n")
_T("WHERE table_schema = '%s' AND index_name = '%s' \n")
_T("ORDER BY table_name, seq_in_index"),
schema, name);
} else if(_tcscmp(type, _T("FUNCTION")) == 0) {
_stprintf(sql_buf,
_T("SELECT * FROM information_schema.routines \n")
_T("WHERE routine_type = 'FUNCTION' AND routine_name = '%s' AND routine_schema = '%s'"),
name, schema);
} else if(_tcscmp(type, _T("PROCEDURE")) == 0) {
_stprintf(sql_buf,
_T("SELECT * FROM information_schema.routines \n")
_T("WHERE routine_type = 'PROCEDURE' AND routine_name = '%s' AND routine_schema = '%s'"),
name, schema);
} else if(_tcscmp(type, _T("TRIGGER")) == 0) {
_stprintf(sql_buf,
_T("SELECT * FROM information_schema.triggers \n")
_T("WHERE trigger_name = '%s' AND trigger_schema = '%s'"),
name, schema);
} else if(_tcscmp(type, _T("EVENT")) == 0) {
_stprintf(sql_buf,
_T("SELECT * FROM information_schema.events \n")
_T("WHERE event_name = '%s' AND event_schema = '%s'"),
name, schema);
} else {
_stprintf(msg_buf, _T("not implement"));
return NULL;
}

return my_create_dataset(ss, sql_buf, msg_buf);
}
