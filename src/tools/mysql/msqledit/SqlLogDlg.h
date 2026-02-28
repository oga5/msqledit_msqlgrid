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

#if !defined(AFX_SQLLOGDLG_H__B1511B22_4B4B_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_SQLLOGDLG_H__B1511B22_4B4B_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SqlLogDlg.h : ヘッダー ファイル
//
#include "EditCtrl.h"
#include "AccelList.h"

/////////////////////////////////////////////////////////////////////////////
// CSqlLogDlg ダイアログ

class CSqlLogDlg : public CDialog
{
private:
	void RelayoutControl();
	int CreateEditCtrl();
	int SetLogList();
	CEditData	m_edit_data;
	CEditCtrl	m_edit_ctrl;

	CAccelList	m_accel_list;

// コンストラクション
public:
	CSqlLogDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ
	CSqlLogArray	*m_sql_log_array;
	CFont			*m_font;
	int				m_selected_row;

// ダイアログ データ
	//{{AFX_DATA(CSqlLogDlg)
	enum { IDD = IDD_SQL_LOG_DLG };
	CButton	m_ok;
	CListCtrl	m_list;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CSqlLogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSqlLogDlg)
	afx_msg void OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SQLLOGDLG_H__B1511B22_4B4B_11D4_B06E_00E018A83B1B__INCLUDED_)
