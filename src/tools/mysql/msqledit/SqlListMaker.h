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

#ifndef _SQL_LIST_MAKER_H_INCLUDE
#define _SQL_LIST_MAKER_H_INCLUDE

#include "CodeAssistData.h"
#include "EditData.h"
#include "CodeAssistListMaker.h"
#include "mylib.h"

class CSqlListMakerCache_Object
{
private:
	CString			*m_object_name;
	HMyDataset		**m_dataset_arr;
	HMyDataset		*m_dataset_buf;

	void LRU(int idx);

public:
	CSqlListMakerCache_Object();
	~CSqlListMakerCache_Object();

	void ClearCache();
	BOOL MakeList(CString object_name, HMyDataset *&dataset, BOOL sort_column_name);
};

class CSqlListMaker : public CCodeAssistListMaker
{
private:
	static CSqlListMakerCache_Object	m_cache_object;
	static BOOL					m_sort_column_name;

	CMapStringToString			m_table_map;
	CString						m_buffer;

	BOOL GetSqlRow(CEditData *edit_data, POINT &start_pt, POINT &end_pt);
	BOOL MakeTableMap(CMapStringToString &table_map,
		CEditData *edit_data, POINT start_pt, POINT end_pt);
	void ParseBracket(CEditData *edit_data, POINT parse_start_pt, POINT parse_end_pt,
		POINT &bracket_end_pt);

	BOOL MakeObjectList_InlineView(CCodeAssistData *assist_data, const CString &object_name);
	BOOL MakeObjectList(CCodeAssistData *assist_data, const CString &object_name);

public:
	static void ClearCache() {
		m_cache_object.ClearCache();
	}
	static void SetSortColumns(BOOL b_sort);

	CSqlListMaker();

	virtual BOOL MakeList(CCodeAssistData *assist_data, CEditData *edit_data);
};

#endif _SQL_LIST_MAKER_H_INCLUDE
