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

#if !defined(AFX_INSERTROWSDLG_H__7C436001_568F_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_INSERTROWSDLG_H__7C436001_568F_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InsertRowsDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CInsertRowsDlg ダイアログ

class CInsertRowsDlg : public CDialog
{
// コンストラクション
public:
	CInsertRowsDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CInsertRowsDlg)
	enum { IDD = IDD_INSERT_ROWS_DLG };
	CButton	m_ok;
	int		m_row_cnt;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CInsertRowsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CInsertRowsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditRowCnt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_INSERTROWSDLG_H__7C436001_568F_11D5_8505_00E018A83B1B__INCLUDED_)
