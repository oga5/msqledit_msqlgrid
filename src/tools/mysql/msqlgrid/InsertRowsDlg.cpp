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

// InsertRowsDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "msqlgrid.h"
#include "InsertRowsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInsertRowsDlg ダイアログ


CInsertRowsDlg::CInsertRowsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInsertRowsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsertRowsDlg)
	m_row_cnt = 0;
	//}}AFX_DATA_INIT
}


void CInsertRowsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsertRowsDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Text(pDX, IDC_EDIT_ROW_CNT, m_row_cnt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsertRowsDlg, CDialog)
	//{{AFX_MSG_MAP(CInsertRowsDlg)
	ON_EN_CHANGE(IDC_EDIT_ROW_CNT, OnChangeEditRowCnt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsertRowsDlg メッセージ ハンドラ

BOOL CInsertRowsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_row_cnt = 1;
	UpdateData(FALSE);
	m_ok.EnableWindow(TRUE);
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CInsertRowsDlg::OnChangeEditRowCnt() 
{
	UpdateData(TRUE);

	if(m_row_cnt > 0) {
		m_ok.EnableWindow(TRUE);
	} else {
		m_ok.EnableWindow(FALSE);
	}
}
