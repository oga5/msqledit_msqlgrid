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
#include "getsrc.h"

/*
 * MySQLのSHOW CREATE TABLEを使ってテーブルのDDLを取得する
 * oid は "schema.tablename" または "tablename" の形式
 */
int dumpTableSchema(HMySession ss, const TCHAR *oid, CEditData *edit_data, TCHAR *msg_buf)
{
TCHAR sql_buf[4096];
HMyDataset dataset = NULL;
int ret_v = 0;

// oidはschema.tablenameまたはtablenameの形式
_stprintf(sql_buf, _T("SHOW CREATE TABLE `%s`"), oid);

dataset = my_create_dataset(ss, sql_buf, msg_buf);
if(dataset == NULL) return 1;

if(my_dataset_row_cnt(dataset) == 0) {
_stprintf(msg_buf, _T("no result for SHOW CREATE TABLE"));
ret_v = 1;
goto END;
}

if(my_dataset_col_cnt(dataset) < 2) {
_stprintf(msg_buf, _T("unexpected result from SHOW CREATE TABLE"));
ret_v = 1;
goto END;
}

{
const TCHAR *src = my_dataset_data(dataset, 0, 1);
if(src != NULL && _tcslen(src) > 0) {
edit_data->paste(src);
}
}

END:
my_free_dataset(dataset);
return ret_v;
}

/*
 * MySQLではインデックスのDDLはCREATE TABLEの一部として取得する
 * table_oid はテーブル名(schema.table形式も可)
 */
int dumpIndexSchema(HMySession ss, const TCHAR *table_oid, const TCHAR *index_oid,
CEditData *edit_data, TCHAR *msg_buf)
{
return dumpTableSchema(ss, table_oid, edit_data, msg_buf);
}
