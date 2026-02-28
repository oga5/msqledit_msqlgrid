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
 
 // LineJumpDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "LineJumpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLineJumpDlg ダイアログ


CLineJumpDlg::CLineJumpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLineJumpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLineJumpDlg)
	m_line_no = 0;
	//}}AFX_DATA_INIT
}


void CLineJumpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLineJumpDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Text(pDX, IDC_EDIT_LINE_NO, m_line_no);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLineJumpDlg, CDialog)
	//{{AFX_MSG_MAP(CLineJumpDlg)
	ON_EN_CHANGE(IDC_EDIT_LINE_NO, OnChangeEditLineNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLineJumpDlg メッセージ ハンドラ

void CLineJumpDlg::OnChangeEditLineNo() 
{
	UpdateData(TRUE);

	if(m_line_no > 0) {
		m_ok.EnableWindow(TRUE);
	} else {
		m_ok.EnableWindow(FALSE);
	}
}
