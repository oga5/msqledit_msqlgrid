/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __EDITABLE_MY_EDIT_DATA_H_INCLUDED__
#define __EDITABLE_MY_EDIT_DATA_H_INCLUDED__

#include "EditableGridData.h"
#include "mylib.h"

#include "mytype.h"


class CEditableMyGridData : public CEditableGridData
{
public:
	static TCHAR *blank_str;

	CEditableMyGridData();
	virtual ~CEditableMyGridData();

	// FIXME: null objectを使う
	virtual int Get_ColCnt() { 
		if(m_dataset == NULL) return 0;
		return my_dataset_col_cnt(m_dataset) - m_reserve_columns;
	}

	virtual const TCHAR *Get_ColName(int col) {
		if(m_dataset == NULL) return _T("");
		return my_dataset_get_colname(m_dataset, col);
	}
	virtual const TCHAR *GetOriginalText(int row, int col);

	virtual int Get_ColMaxSize(int col) {
		if(m_dataset == NULL) return 0;
		return my_dataset_get_colsize(m_dataset, col);
	}

	virtual int Get_ColDataType(int row, int col) {
		unsigned int t = my_dataset_get_coltype(m_dataset, col);
		if(t == MYSQL_TYPE_TINY || t == MYSQL_TYPE_SHORT || t == MYSQL_TYPE_LONG ||
			t == MYSQL_TYPE_LONGLONG || t == MYSQL_TYPE_INT24 ||
			t == MYSQL_TYPE_FLOAT || t == MYSQL_TYPE_DOUBLE ||
			t == MYSQL_TYPE_DECIMAL || t == MYSQL_TYPE_NEWDECIMAL) return GRID_DATA_NUMBER_TYPE;
		return GRID_DATA_CHAR_TYPE;
	}

	virtual BOOL IsColDataNull(int row, int col) {
		if(m_dataset == NULL) return FALSE;
		if(IsUpdateCell(row, col) || IsInsertRow(row)) {
			if(IsBlank(row, col)) return FALSE;
			return (Get_ColData(row, col)[0] == '\0');
		}
		return my_dataset_is_null(m_dataset, GetRowIdx(row), col);
	}

	int SetDataset(HMySession ss, HMyDataset dataset, HMyDataset pkey_dataset, const TCHAR *owner, 
		const TCHAR *table_name, const TCHAR *sql, BOOL b_data_lock);

	virtual int GetSystemReserveColumnCnt() { return m_reserve_columns; }

	int SaveData(HMySession ss, POINT *err_pt, TCHAR *msg_buf, HWND hWnd, volatile int *cancel_flg);

	virtual int UpdateCell(int row, int col, const TCHAR *data, int len);

protected:

private:
	HMySession	m_ss;
	HMyDataset	m_null_dataset;
	HMyDataset	m_dataset;
	HMyDataset	m_pkey_dataset;

	CString		m_owner;
	CString		m_table_name;
	CString		m_sql;

	BOOL		m_b_data_lock;

	int			m_reserve_columns;

	CString GetUpdateWhereClause(int row);
	CString GetSqlColData(int row, int col);

	int DeleteData(HMySession ss, POINT *err_pt, int *row_cnt, 
		TCHAR *msg_buf, HWND hWnd, volatile int *cancel_flg);
	int InsertData(HMySession ss, POINT *err_pt, int *row_cnt, 
		TCHAR *msg_buf, HWND hWnd, volatile int *cancel_flg);
	int UpdateData(HMySession ss, POINT *err_pt, int *row_cnt, 
		TCHAR *msg_buf, HWND hWnd, volatile int *cancel_flg);

	void SetBlank(int row, int col);
	void UnSetBlank(int row, int col);
	BOOL IsBlank(int row, int col);
};


unsigned int _stdcall save_grid_data_thr(void *lpvThreadParam);

struct _thr_save_grid_data_st {
	CEditableMyGridData *grid_data;
	HMySession	ss;
	POINT		*err_pt;
	TCHAR		*msg_buf;
	void		*hWnd;
	volatile int *cancel_flg;
	int			ret_v;
};

#endif /* __EDITABLE_MY_EDIT_DATA_H_INCLUDED__ */