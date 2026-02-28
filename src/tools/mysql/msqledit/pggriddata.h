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

#ifndef _ORA_GRID_DATA_H_INCLUDE
#define _ORA_GRID_DATA_H_INCLUDE

#include "griddata.h"
#include "mylib.h"

#include "mytype.h"

class CMyGridData : public CGridData
{
public:
	CMyGridData() {
		m_null_dataset = NULL;
		m_dataset = m_null_dataset;
		m_row_idx = NULL;
	}
	virtual ~CMyGridData();

	// FIXME: null objectを使う
	virtual int Get_ColCnt() { 
		if(m_dataset == NULL) return 0;
		return my_dataset_col_cnt(m_dataset);
	}
	virtual int Get_RowCnt() {
		if(m_dataset == NULL) return 0;
		return my_dataset_row_cnt(m_dataset);
	}
	virtual const TCHAR *Get_ColName(int col) {
		if(m_dataset == NULL) return _T("");
		return my_dataset_get_colname(m_dataset, col);
	}
	virtual const TCHAR *Get_ColData(int row, int col) {
		if(m_dataset == NULL) return _T("");
		return my_dataset_data(m_dataset, GetRowIdx(row), col);
	}
	virtual int Get_ColMaxSize(int col) {
		if(m_dataset == NULL) return 0;
		return my_dataset_get_colsize(m_dataset, col);
	}

	virtual int Get_ColDataType(int row, int col) {
		my_type t = my_dataset_get_coltype(m_dataset, col);
		if(t == MYSQL_TYPE_TINY || t == MYSQL_TYPE_SHORT || t == MYSQL_TYPE_LONG ||
			t == MYSQL_TYPE_LONGLONG || t == MYSQL_TYPE_INT24 ||
			t == MYSQL_TYPE_FLOAT || t == MYSQL_TYPE_DOUBLE ||
			t == MYSQL_TYPE_DECIMAL || t == MYSQL_TYPE_NEWDECIMAL) return GRID_DATA_NUMBER_TYPE;
		return GRID_DATA_CHAR_TYPE;
	}

	virtual BOOL IsColDataNull(int row, int col) {
		if(m_dataset == NULL) return FALSE;
		return my_dataset_is_null(m_dataset, GetRowIdx(row), col);
	}

	virtual int GetMaxColLen(int col, int limit_len);

	void SetDataset(HMyDataset dataset);

protected:
	virtual void SwapRow(int r1, int r2);

private:
	HMyDataset	m_null_dataset;
	HMyDataset	m_dataset;
	int			*m_row_idx;

	int GetRowIdx(int row) {
		if(m_row_idx == NULL) return row;
		return m_row_idx[row];
	}

	void ClearRowIdx();
	void InitRowIdx();
};

#endif  _ORA_GRID_DATA_H_INCLUDE
