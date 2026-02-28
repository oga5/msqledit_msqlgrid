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
 
 // MSqlEditCtrl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "msqledit.h"
#include "PSqlEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMSqlEditCtrl

CMSqlEditCtrl::CMSqlEditCtrl()
{
	m_paste_lower = FALSE;
	SetCodeAssistListMaker(&m_sql_list_maker);
	m_no_quote_color_char = '"';
}

CMSqlEditCtrl::~CMSqlEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CMSqlEditCtrl, CCodeAssistEditCtrl)
	//{{AFX_MSG_MAP(CMSqlEditCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMSqlEditCtrl メッセージ ハンドラ

void CMSqlEditCtrl::DoCodePaste(TCHAR *paste_str, TCHAR *type)
{
	CString paste_word;
/*
	if(strcmp((char *)type, "KEYWORD") == 0 || strcmp((char *)type, "KEYWORD2") == 0) {
		paste_word = paste_str;
	} else {
		make_object_name(&paste_word, (char *)paste_str, m_paste_lower);
	}
*/
	paste_word = paste_str;

	CString org_str = GetSelectedText();
	if(inline_isupper(org_str.GetBuffer(0)[0]) && paste_word.GetBuffer(0)[0] != '\"') {
		paste_word.MakeUpper();
	}

	// 補完前後のテキストが同じ場合、pasteしない (undoデータを増やさない)
	if(org_str.GetLength() == paste_word.GetLength() &&
		org_str.Compare(paste_word) == 0) {
		ClearSelected();
		return;
	}

	Paste(paste_word.GetBuffer(0));
}


