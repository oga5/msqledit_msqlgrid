/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_PSQLEDITCTRL_H__873A1B04_1844_4532_A8B7_E4AF743960BD__INCLUDED_)
#define AFX_PSQLEDITCTRL_H__873A1B04_1844_4532_A8B7_E4AF743960BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MSqlEditCtrl.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CMSqlEditCtrl ウィンドウ

#include "CodeAssistEditCtrl.h"
#include "CodeAssistWnd.h"
#include "StrGridData.h"
#include "SqlListMaker.h"

class CMSqlEditCtrl : public CCodeAssistEditCtrl
{
// コンストラクション
public:
	CMSqlEditCtrl();

// アトリビュート
public:

// オペレーション
public:
	void SetPasteLower(BOOL paste_lower) { m_paste_lower = paste_lower; }

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CMSqlEditCtrl)
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CMSqlEditCtrl();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CMSqlEditCtrl)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL			m_paste_lower;
	CSqlListMaker	m_sql_list_maker;

	virtual void DoCodePaste(TCHAR *paste_str, TCHAR *type);
	virtual BOOL IsCodeAssistOnChar(unsigned int nChar) { return (nChar == '.'); }
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_PSQLEDITCTRL_H__873A1B04_1844_4532_A8B7_E4AF743960BD__INCLUDED_)
