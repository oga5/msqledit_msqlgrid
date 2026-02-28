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
 
 // DownloadDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "msqledit.h"
#include "DownloadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDownloadDlg ダイアログ


CDownloadDlg::CDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDownloadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDownloadDlg)
	m_file_name = _T("");
	m_save_select_result_only = FALSE;
	//}}AFX_DATA_INIT
}


void CDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDownloadDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Text(pDX, IDC_EDIT_FILE_NAME, m_file_name);
	DDX_Check(pDX, IDC_CHECK_SAVE_SELECT_RESULT_ONLY, m_save_select_result_only);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDownloadDlg, CDialog)
	//{{AFX_MSG_MAP(CDownloadDlg)
	ON_BN_CLICKED(IDC_BTN_FILE_SELECT, OnBtnFileSelect)
	ON_EN_CHANGE(IDC_EDIT_FILE_NAME, OnChangeEditFileName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDownloadDlg メッセージ ハンドラ

BOOL CDownloadDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	LoadSetup();
	UpdateData(FALSE);

	CheckBtn();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CDownloadDlg::OnBtnFileSelect() 
{
	CFileDialog		file_dlg(FALSE, _T("csv"), NULL,
		OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("CSV Files (*.csv)|*.csv|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"),
		AfxGetMainWnd());
	file_dlg.m_ofn.lpstrTitle = _T("SQL実行結果の保存");

	// デフォルトディレクトリを設定
	TCHAR	cur_dir[_MAX_PATH];
	GetCurrentDirectory(sizeof(cur_dir)/sizeof(cur_dir[0]), cur_dir);
	file_dlg.m_ofn.lpstrInitialDir = cur_dir;

	if(file_dlg.DoModal() != IDOK) {
		return;
	}

	m_file_name = file_dlg.GetPathName().GetBuffer(0);
	UpdateData(FALSE);

	CheckBtn();
}

void CDownloadDlg::OnChangeEditFileName() 
{
	CheckBtn();
}

void CDownloadDlg::CheckBtn()
{
	UpdateData(TRUE);

	if(m_file_name == _T("")) {
		m_ok.EnableWindow(FALSE);
	} else {
		m_ok.EnableWindow(TRUE);
	}
}

void CDownloadDlg::OnOK() 
{
	UpdateData(TRUE);

	SaveSetup();
	
	CDialog::OnOK();
}

void CDownloadDlg::LoadSetup()
{
	m_file_name = AfxGetApp()->GetProfileString(_T("DOWNLOAD_DLG"), _T("FILE_NAME"), _T(""));
	m_save_select_result_only = AfxGetApp()->GetProfileInt(_T("DOWNLOAD_DLG"), _T("SAVE_SELECT_RESULT_ONLY"), 0);
}

void CDownloadDlg::SaveSetup()
{
	AfxGetApp()->WriteProfileString(_T("DOWNLOAD_DLG"), _T("FILE_NAME"), m_file_name);
	AfxGetApp()->WriteProfileInt(_T("DOWNLOAD_DLG"), _T("SAVE_SELECT_RESULT_ONLY"), m_save_select_result_only);
}

