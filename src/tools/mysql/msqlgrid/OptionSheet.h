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

#if !defined(AFX_OPTIONSHEET_H__71AC5183_A085_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_OPTIONSHEET_H__71AC5183_A085_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OptionSheet.h : ヘッダー ファイル
//
#include "SetupPage.h"
#include "GridOptionPage.h"

/////////////////////////////////////////////////////////////////////////////
// COptionSheet

class COptionSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionSheet)

private:

// コンストラクション
public:
	COptionSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	COptionSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// アトリビュート
public:
	CSetupPage			m_setup_page;
	CGridOptionPage		m_grid_page;

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COptionSheet)
public:
	virtual INT_PTR DoModal();
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~COptionSheet();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(COptionSheet)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OPTIONSHEET_H__71AC5183_A085_11D4_B06E_00E018A83B1B__INCLUDED_)
