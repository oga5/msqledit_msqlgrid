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

#include "UnicodeArchive.h"

#define ALL_USERS	_T("ALL USERS")

CString get_table_schema_name(HMySession ss, const TCHAR *table_name, TCHAR *msg_buf);

int get_dataset(HMySession ss, const TCHAR *sql, TCHAR *msg_buf, 
	void *hWnd, volatile int *cancel_flg, HMyDataset *result);

CString explain_plan(HMySession ss, const TCHAR *sql, TCHAR *msg_buf);

int execute_sql(HMySession ss, const TCHAR *sql, TCHAR *msg_buf,
	volatile int *cancel_flg);

int download(HMySession ss, CUnicodeArchive *ar, const TCHAR *sql, TCHAR *msg_buf,
	BOOL put_column_name, void *hWnd, volatile int *cancel_flg);

HMyDataset get_object_list(HMySession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf);
HMyDataset get_user_list(HMySession ss, TCHAR *msg_buf);
HMyDataset get_column_list(HMySession ss, const TCHAR *relname, const TCHAR *schema, TCHAR *msg_buf);
HMyDataset get_index_list_by_table(HMySession ss, const TCHAR *relname, 
	const TCHAR *schema, TCHAR *msg_buf);
HMyDataset get_trigger_list_by_table(HMySession ss, const TCHAR *relname,
	const TCHAR *schema, TCHAR *msg_buf);

HMyDataset get_object_properties(HMySession ss, const TCHAR *type, const TCHAR *name, 
	const TCHAR *schema, TCHAR *msg_buf);


