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

// TableListDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "msqlgrid.h"
#include "TableListDlg.h"

#include "CancelDlg.h"
#include "fileutil.h"
#include "ostrutil.h"
#include "PlacesBarFileDlg.h"

#include "oregexp.h"

#include "mymsg.h"

#include <process.h>
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum Table_list
{
	TABLE_NAME,
	OBJECT_TYPE,
	SCHEMA_NAME,
	OWNER,
};

enum Column_list
{
	TABLE_COLUMN_NAME,
	DATA_TYPE,
	DATA_LENGTH,
	NULLABLE,
	COLUMN_COMMENTS,
	COLUMN_ID,
};

enum Selected_Column_list
{
	SELECTED_COLUMN_NAME,
};

static HMyDataset get_user_list(HMySession ss, TCHAR *msg_buf)
{
	return my_create_dataset(ss,
		_T("SELECT schema_name AS usename \n")
		_T("FROM information_schema.schemata \n")
		_T("ORDER BY schema_name"),
		msg_buf);
}

static int get_object_name_list_for_table_list_dlg(HMySession ss, const TCHAR *owner, 
	HMyDataset *dataset, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	if(!g_option.edit_other_owner && owner != NULL && _tcslen(owner) > 0) {
		_stprintf(sql_buf,
			_T("SELECT table_name AS object_name, table_schema AS nspname, \n")
			_T("       '' AS owner, \n")
			_T("       CASE table_type WHEN 'BASE TABLE' THEN 'table' WHEN 'VIEW' THEN 'view' ELSE 'other' END AS object_type \n")
			_T("FROM information_schema.tables \n")
			_T("WHERE table_schema = '%s' \n")
			_T("AND table_type IN ('BASE TABLE', 'VIEW') \n")
			_T("ORDER BY table_name \n"),
			owner);
	} else {
		_stprintf(sql_buf,
			_T("SELECT table_name AS object_name, table_schema AS nspname, \n")
			_T("       '' AS owner, \n")
			_T("       CASE table_type WHEN 'BASE TABLE' THEN 'table' WHEN 'VIEW' THEN 'view' ELSE 'other' END AS object_type \n")
			_T("FROM information_schema.tables \n")
			_T("WHERE table_schema = DATABASE() \n")
			_T("AND table_type IN ('BASE TABLE', 'VIEW') \n")
			_T("ORDER BY table_name \n"));
	}

	*dataset = my_create_dataset(ss, sql_buf, msg_buf);
	if(*dataset == NULL) return 1;

	return 0;
}

static HMyDataset get_column_list(HMySession ss, const TCHAR *relname, 
	const TCHAR *schema, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	if(schema != NULL && _tcslen(schema) > 0) {
		_stprintf(sql_buf,
			_T("SELECT column_name AS attname, data_type AS typname, \n")
			_T("       COALESCE( \n")
			_T("           CAST(character_maximum_length AS CHAR), \n")
			_T("           IF(numeric_scale IS NOT NULL AND numeric_scale > 0, \n")
			_T("              CONCAT(CAST(numeric_precision AS CHAR), ',', CAST(numeric_scale AS CHAR)), \n")
			_T("              CAST(numeric_precision AS CHAR)), \n")
			_T("           '-1') AS attlen, \n")
			_T("       is_nullable AS attnotnull, \n")
			_T("       column_comment AS description, \n")
			_T("       ordinal_position AS objsubid \n")
			_T("FROM information_schema.columns \n")
			_T("WHERE table_schema = '%s' \n")
			_T("AND table_name = '%s' \n")
			_T("ORDER BY ordinal_position \n"),
			schema, relname);
	} else {
		_stprintf(sql_buf,
			_T("SELECT column_name AS attname, data_type AS typname, \n")
			_T("       COALESCE( \n")
			_T("           CAST(character_maximum_length AS CHAR), \n")
			_T("           IF(numeric_scale IS NOT NULL AND numeric_scale > 0, \n")
			_T("              CONCAT(CAST(numeric_precision AS CHAR), ',', CAST(numeric_scale AS CHAR)), \n")
			_T("              CAST(numeric_precision AS CHAR)), \n")
			_T("           '-1') AS attlen, \n")
			_T("       is_nullable AS attnotnull, \n")
			_T("       column_comment AS description, \n")
			_T("       ordinal_position AS objsubid \n")
			_T("FROM information_schema.columns \n")
			_T("WHERE table_schema = DATABASE() \n")
			_T("AND table_name = '%s' \n")
			_T("ORDER BY ordinal_position \n"),
			relname);
	}

	return my_create_dataset(ss, sql_buf, msg_buf);
}

static HMyDataset get_key_column_list(HMySession ss, const TCHAR *relname, 
	const TCHAR *schema, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	HMyDataset	column_list;

	if(schema != NULL && _tcslen(schema) > 0) {
		_stprintf(sql_buf,
			_T("SHOW INDEX FROM `%s`.`%s` WHERE Key_name = 'PRIMARY'"),
			schema, relname);
	} else {
		_stprintf(sql_buf,
			_T("SHOW INDEX FROM `%s` WHERE Key_name = 'PRIMARY'"),
			relname);
	}

	column_list = my_create_dataset(ss, sql_buf, msg_buf);
	if(column_list == NULL) return NULL;

	if(my_dataset_row_cnt(column_list) == 0) {
		my_free_dataset(column_list);
		_stprintf(msg_buf,
			_T("Primary Keyの存在しないテーブルでは利用できません(%s)"),
			relname);
		return NULL;
	}

	return column_list;
}


/////////////////////////////////////////////////////////////////////////////
// CTableListDlg ダイアログ


CTableListDlg::CTableListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTableListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTableListDlg)
	m_data_lock = FALSE;
	//}}AFX_DATA_INIT

	m_dataset = NULL;
	m_pkey_dataset = NULL;
	m_auto_start = FALSE;
	m_auto_start_timer = 0;
	m_init = FALSE;
	m_osg_file_name = _T("");
	m_alias_name = _T("");
//	m_column_name_list = &m_null_column_name_list;

	m_keyword_is_ok = TRUE;
	m_table_dataset = NULL;
	m_grid_swap_row_col_mode = FALSE;
}

CTableListDlg::~CTableListDlg()
{
	my_free_dataset(m_table_dataset);
}

void CTableListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTableListDlg)
	DDX_Control(pDX, IDC_SPIN_SELECTED_COLUMN, m_spin_selected_column);
	DDX_Control(pDX, IDC_BUTTON_CLEAR_COLUMN_LIST, m_btn_clear_column);
	DDX_Control(pDX, IDC_BUTTON_DEL, m_btn_del);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_btn_add);
	DDX_Control(pDX, IDC_SELECTED_COLUMN_LIST, m_selected_column_list);
	DDX_Control(pDX, IDC_STATIC_FILTER, m_static_filter);
	DDX_Control(pDX, IDC_EDIT_FILTER, m_edit_filter);
	DDX_Control(pDX, IDC_BTN_FILTER, m_btn_filter);
	DDX_Control(pDX, IDC_BTN_LOAD, m_btn_load);
	DDX_Control(pDX, IDC_BTN_SAVE, m_btn_save);
	DDX_Control(pDX, IDC_CHECK_DATA_LOCK, m_check_data_lock);
	DDX_Control(pDX, IDC_STATIC_WHERE, m_static_where);
	DDX_Control(pDX, IDC_STATIC_TABLE, m_static_table);
	DDX_Control(pDX, IDC_STATIC_COLUMN, m_static_column);
	DDX_Control(pDX, IDC_BUTTON_SELECT_ALL_COLUMN, m_sel_all_btn);
	DDX_Control(pDX, IDCANCEL, m_cancel);
	DDX_Control(pDX, IDC_TABLE_LIST, m_table_list);
	DDX_Control(pDX, IDC_COLUMN_LIST, m_column_list);
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Check(pDX, IDC_CHECK_DATA_LOCK, m_data_lock);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_COMBO_OWNER, m_combo_owner);
	DDX_Control(pDX, IDC_STATIC_OWNER, m_static_owner);
}


BEGIN_MESSAGE_MAP(CTableListDlg, CDialog)
	//{{AFX_MSG_MAP(CTableListDlg)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TABLE_LIST, OnItemchangedTableList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_COLUMN_LIST, OnItemchangedColumnList)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_ALL_COLUMN, OnButtonSelectAllColumn)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_LOAD, OnBtnLoad)
	ON_BN_CLICKED(IDC_BTN_SAVE, OnBtnSave)
	ON_BN_CLICKED(IDC_BTN_FILTER, OnBtnFilter)
	ON_EN_CHANGE(IDC_EDIT_FILTER, OnChangeEditFilter)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_COLUMN_LIST, OnButtonClearColumnList)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnButtonDel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SELECTED_COLUMN_LIST, OnItemchangedSelectedColumnList)
	ON_NOTIFY(NM_DBLCLK, IDC_COLUMN_LIST, OnDblclkColumnList)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SELECTED_COLUMN, OnDeltaposSpinSelectedColumn)
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_COMBO_OWNER, &CTableListDlg::OnSelchangeComboOwner)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTableListDlg メッセージ ハンドラ

struct _thr_create_dataset_st {
	HMySession	ss;
	HMyDataset	dataset;
	TCHAR		*sql;
	TCHAR		*msg_buf;
	void		*hWnd;
	volatile int *cancel_flg;
	int			ret_v;
};

unsigned int _stdcall create_dataset_thr(void *lpvThreadParam)
{
	struct _thr_create_dataset_st *p_st;

	p_st = (struct _thr_create_dataset_st *)lpvThreadParam;

	if(p_st->hWnd != NULL) {
		SendMessage((HWND)p_st->hWnd, WM_OCI_DLG_STATIC, 0,
			(LPARAM)_T("SQL実行中"));
	}

	p_st->ret_v = my_create_dataset_ex(p_st->ss, p_st->sql, p_st->msg_buf,
		p_st->cancel_flg, p_st->hWnd, &(p_st->dataset));

	/* キャンセルされたか調べる */
	if(p_st->cancel_flg != NULL) {
		for(; *(p_st->cancel_flg) == 2;) Sleep(500);
		if(*(p_st->cancel_flg) == 1) {
			p_st->ret_v = MYERR_CANCEL;
			_stprintf(p_st->msg_buf, MYERR_CANCEL_MSG);
		}
	}
	if(p_st->hWnd != NULL) {
		SendMessage((HWND)p_st->hWnd, WM_OCI_DLG_ENABLE_CANCEL, FALSE, 0);
		PostMessage((HWND)p_st->hWnd, WM_OCI_DLG_EXIT, p_st->ret_v, 0);
	}

	return p_st->ret_v;
}

int CTableListDlg::CreateOciDataset()
{
	CString		sql;
	CString		col_list;
	CString		col;
	POSITION	pos;
	CWaitCursor	wait_cursor;
	CCancelDlg	dlg;
	int			i;

	if(m_alias_name.IsEmpty()) m_alias_name = _T("a");

	for(pos = m_column_name_list.GetHeadPosition(); pos != NULL; m_column_name_list.GetNext(pos)) {
		if(pos != m_column_name_list.GetHeadPosition()) {
			col_list += _T(", ");
		}
		col.Format(_T("%s.`%s`"), m_alias_name, m_column_name_list.GetAt(pos));
		col_list += col;
	}

	m_pkey_dataset = get_key_column_list(m_ss, GetTableName(), GetSchemaName(), m_msg_buf);
	if(m_pkey_dataset == NULL) {
		MessageBox(m_msg_buf, _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	for(i = 0; i < my_dataset_row_cnt(m_pkey_dataset); i++) {
		col.Format(_T(", %s.`%s`"), m_alias_name, my_dataset_data2(m_pkey_dataset, i, _T("COLUMN_NAME")));
		col_list += col;
	}

	if(m_data_lock) {
		sql.Format(_T("select %s \n")
			_T("from %s %s \n")
			_T("%s for update \n"), 
			col_list, m_table_name, m_alias_name, m_where_clause);
	} else {
		sql.Format(_T("select %s \n")
			_T("from %s %s \n")
			_T("%s"), 
			col_list, m_table_name, m_alias_name, m_where_clause);
	}

	struct _thr_create_dataset_st st;
	st.ss = m_ss;
	st.dataset = NULL;
	st.sql = sql.GetBuffer(0);
	st.msg_buf = m_msg_buf;
	st.cancel_flg = &(dlg.m_cancel_flg);
	st.ret_v = 0;
	dlg.m_cancel_flg = 0;
	dlg.m_p_hWnd = (HWND *)&(st.hWnd);

	// SQL実行用スレッドを作成
	UINT			thrdaddr;
	dlg.m_hThread = _beginthreadex(NULL, 0, create_dataset_thr, &st, CREATE_SUSPENDED, &thrdaddr);
	if(dlg.m_hThread == NULL) {
		_stprintf(m_msg_buf, _T("スレッド作成エラー"));
		goto ERR1;
	}
	dlg.m_timer_flg = FALSE;
	dlg.DoModal();

	WaitForSingleObject((void *)dlg.m_hThread, INFINITE);
	CloseHandle((void *)dlg.m_hThread);

//	if(st.ret_v != 0 && st.ret_v != MYERR_CANCEL) goto ERR1;
	if(st.ret_v != 0) goto ERR1;

	m_dataset = st.dataset;
	m_sql = st.sql;

	return 0;

ERR1:
	MessageBox(m_msg_buf, _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
	my_free_dataset(st.dataset);
	my_free_dataset(m_pkey_dataset);

	return st.ret_v;
}

void CTableListDlg::OnOK()
{
	UpdateData(TRUE);

	if(m_selected_column_list.GetItemCount() == 0) {
		OnButtonAdd();
	}

	BOOL close_on_error = m_auto_start;

	m_auto_start = FALSE;

	m_owner = GetOwner();

	m_table_name = GetAllTableName();
	if(m_table_name == _T("")) return;

	m_where_clause = m_edit_data.get_all_text();

	if(!MakeColumnNameList(TRUE)) {
		return;
	}

	// 前のデータをロールバックする
	int ret_v;
	ret_v = my_rollback(m_ss, m_msg_buf);
	if(ret_v != 0) {
		// FIXME: 致命的なエラー
		MessageBox(m_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	if(CreateOciDataset() != 0) {
		if(close_on_error) {
			EndDialog(IDCANCEL);
			return;
		}
		m_table_name == _T("");
		return;
	}

	CDialog::OnOK();
}

void CTableListDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CTableListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_table_list.InsertColumn(TABLE_NAME, _T("table_name"), LVCFMT_LEFT, 200);
	m_table_list.InsertColumn(OBJECT_TYPE, _T("type"), LVCFMT_LEFT, 70);
	m_table_list.InsertColumn(SCHEMA_NAME, _T("schema"), LVCFMT_LEFT, 70);
	m_table_list.InsertColumn(OWNER, _T("owner"), LVCFMT_LEFT, 70);
	ListView_SetExtendedListViewStyle(m_table_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_table_list.SetFont(m_font);

	m_column_list.InsertColumn(COLUMN_ID, _T("no"), LVCFMT_RIGHT, 25);
	m_column_list.InsertColumn(TABLE_COLUMN_NAME, _T("column name"), LVCFMT_LEFT, 120);
	m_column_list.InsertColumn(DATA_TYPE, _T("data type"), LVCFMT_LEFT, 70);
	m_column_list.InsertColumn(DATA_LENGTH, _T("length"), LVCFMT_RIGHT, 30);
	m_column_list.InsertColumn(NULLABLE, _T("nullable"), LVCFMT_LEFT, 30);
	m_column_list.InsertColumn(COLUMN_COMMENTS, _T("comments"), LVCFMT_LEFT, 100);
	ListView_SetExtendedListViewStyle(m_column_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_column_list.SetFont(m_font);

	m_selected_column_list.InsertColumn(SELECTED_COLUMN_NAME, _T("column name"), LVCFMT_LEFT, 120);
	ListView_SetExtendedListViewStyle(m_selected_column_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_selected_column_list.SetFont(m_font);

	CreateEditCtrl();

	if(SetUserList(m_owner) != 0) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	m_owner = GetOwner();

	if(SetTableList(m_owner) != 0) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	if(SetColumnList(m_owner, GetSchemaName(), GetTableName(), GetObjectType()) != 0) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	{
		for(POSITION pos = m_column_name_list.GetHeadPosition(); pos != NULL; m_column_name_list.GetNext(pos)) {
			AddColumn(m_column_name_list.GetAt(pos), TRUE);
		}
	}

	if(m_osg_file_name != _T("")) {
		if(!LoadFile(m_osg_file_name.GetBuffer(0))) {
			EndDialog(IDCANCEL);
			return TRUE;
		}
	}
	
	CheckButtons();

	if(m_auto_start && m_ok.IsWindowEnabled()) {
		m_auto_start_timer = SetTimer(1, 1, NULL);
		m_ok.EnableWindow(FALSE);
		m_edit_filter.SetReadOnly(TRUE);
	}

	m_init = TRUE;

	int width = AfxGetApp()->GetProfileInt(_T("TABLE_DLG"), _T("WIDTH"), 0);
	int height = AfxGetApp()->GetProfileInt(_T("TABLE_DLG"), _T("HEIGHT"), 0);
	if(width > 200 && height > 200) {
		SetWindowPos(NULL, 0, 0, width, height, 
			SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOZORDER);
	} else {
		TRACE(_T("検索条件設定ダイアログを，デフォルトサイズで表示\n"));
	}

	UpdateData(FALSE);

	RelayoutControl();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CTableListDlg::CheckOKButton()
{
	if(m_table_list.GetSelectedCount() == 0 || 
		(m_column_list.GetSelectedCount() == 0 && m_selected_column_list.GetItemCount() == 0)) {
		m_ok.EnableWindow(FALSE);
		m_btn_save.EnableWindow(FALSE);
	} else {
		m_ok.EnableWindow(TRUE);
		m_btn_save.EnableWindow(TRUE);
	}
}

void CTableListDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == m_auto_start_timer) {
		KillTimer(m_auto_start_timer);
		OnOK();
	}
	
	CDialog::OnTimer(nIDEvent);
}

int CTableListDlg::SetTableList(const TCHAR *owner)
{
	int			ret_v = 0;

	my_free_dataset(m_table_dataset);

	//ret_v = get_object_list(m_ss, owner, "TABLE", &m_table_dataset, m_msg_buf);
	ret_v = get_object_name_list_for_table_list_dlg(m_ss, owner, &m_table_dataset,
		m_msg_buf);
	if(ret_v != 0) goto ERR1;

	ret_v = SetTableList(m_table_dataset);
	return ret_v;

ERR1:
	MessageBox(m_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return ret_v;
}

int CTableListDlg::SetTableList(HMyDataset dataset)
{
	CWaitCursor	wait_cursor;
	int			r;
	LV_ITEM		item;
	CListCtrl	*p_list = &m_table_list;

	p_list->SetRedraw(FALSE);

	p_list->DeleteAllItems();
	p_list->SetItemCount(my_dataset_row_cnt(dataset));

	item.mask = LVIF_PARAM;
	item.iSubItem = 0;

	CString search_text;
	m_edit_filter.GetWindowText(search_text);

	HREG_DATA hreg = NULL;
	if(search_text != _T("")) {
		hreg = oreg_comp(search_text, 1);
		if(hreg == NULL) {
			SetKeywordOK(FALSE);
		} else {
			SetKeywordOK(TRUE);
		}
	}
	CString data_str;
	int item_idx;

	CString table_name;
	CString schema_name;

	SplitSchemaAndTable(m_table_name, schema_name, table_name);

	for(r = 0, item_idx = 0; r < my_dataset_row_cnt(dataset); r++) {
		if(hreg != NULL) {
			OREG_DATASRC data_src;
			data_str = my_dataset_data2(dataset, r, _T("OBJECT_NAME"));
			data_str.MakeLower();
			oreg_make_str_datasrc(&data_src, data_str);
			if(oreg_exec2(hreg, &data_src) != OREGEXP_FOUND) continue;
		}

		item.iItem = item_idx;
		item.lParam = r;
		item.iItem = p_list->InsertItem(&item);
		if(item.iItem == -1) goto ERR1;

		p_list->SetItemText(item.iItem, TABLE_NAME, my_dataset_data2(dataset, r, _T("OBJECT_NAME")));
		p_list->SetItemText(item.iItem, OBJECT_TYPE, my_dataset_data2(dataset, r, _T("OBJECT_TYPE")));
		p_list->SetItemText(item.iItem, SCHEMA_NAME, my_dataset_data2(dataset, r, _T("NSPNAME")));
		p_list->SetItemText(item.iItem, OWNER, my_dataset_data2(dataset, r, _T("OWNER")));

		if(schema_name.IsEmpty()) {
			if(table_name.Compare(my_dataset_data2(dataset, r, _T("OBJECT_NAME"))) == 0) {
				p_list->SetItemState(item.iItem, LVIS_SELECTED | LVIS_FOCUSED, 
					LVIS_SELECTED | LVIS_FOCUSED);
				// アイテムを可視にする
				p_list->EnsureVisible(item.iItem, FALSE);
			}
		} else {
			if(table_name.Compare(my_dataset_data2(dataset, r, _T("OBJECT_NAME"))) == 0 &&
				schema_name.Compare(my_dataset_data2(dataset, r, _T("NSPNAME"))) == 0) {
				p_list->SetItemState(item.iItem, LVIS_SELECTED | LVIS_FOCUSED, 
					LVIS_SELECTED | LVIS_FOCUSED);
				// アイテムを可視にする
				p_list->EnsureVisible(item.iItem, FALSE);
			}
		}

		item_idx++;
	}

	if(hreg != NULL) oreg_free(hreg);
	p_list->SetRedraw(TRUE);

	return 0;

ERR1:
	if(hreg != NULL) oreg_free(hreg);
	MessageBox(_T(""), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	p_list->SetRedraw(TRUE);
	return 1;
}

int CTableListDlg::SetColumnList(const TCHAR *owner, const TCHAR *schema_name, 
	const TCHAR *table_name, const TCHAR *type, BOOL b_all_select)
{
	m_selected_column_list.DeleteAllItems();

	int		ret_v;
	ret_v = SetColumnList_Table(owner, schema_name, table_name, b_all_select);

	CheckButtons();

	return ret_v;
}

int CTableListDlg::SetColumnList_Table(const TCHAR *owner, const TCHAR *schema_name, 
	const TCHAR *table_name, BOOL b_all_select)
{
	if(table_name == NULL || _tcslen(table_name) == 0) return 0;

	int			ret_v = 0;
	HMyDataset	dataset = NULL;

	dataset = get_column_list(m_ss, table_name, schema_name, m_msg_buf);
	if(dataset == NULL) {
		ret_v = 1;
		goto ERR1;
	}

	ret_v = SetColumnList_Table(dataset, b_all_select);

	my_free_dataset(dataset);

	return ret_v;

ERR1:
	MessageBox(m_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return ret_v;
}

int CTableListDlg::SetColumnList_Table(HMyDataset dataset, BOOL b_all_select)
{
	CWaitCursor	wait_cursor;
	int			r;
	LV_ITEM		item;
	CListCtrl	*p_list = &m_column_list;

	p_list->DeleteAllItems();
	p_list->SetItemCount(my_dataset_row_cnt(dataset));

	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.iSubItem = 0;

	for(r = 0; r < my_dataset_row_cnt(dataset); r++) {
		item.iItem = r;
		item.lParam = r;
		item.pszText = (TCHAR *)my_dataset_data2(dataset, r, _T("ATTNAME"));
		item.iItem = p_list->InsertItem(&item);
		if(item.iItem == -1) goto ERR1;

		p_list->SetItemText(r, DATA_TYPE, my_dataset_data2(dataset, r, _T("TYPNAME")));
		p_list->SetItemText(r, DATA_LENGTH, my_dataset_data2(dataset, r, _T("ATTLEN")));
		p_list->SetItemText(r, NULLABLE, my_dataset_data2(dataset, r, _T("ATTNOTNULL")));
		p_list->SetItemText(r, COLUMN_COMMENTS, my_dataset_data2(dataset, r, _T("DESCRIPTION")));
		p_list->SetItemText(r, COLUMN_ID, my_dataset_data2(dataset, r, _T("OBJSUBID")));

		if(b_all_select) {
			p_list->SetItemState(item.iItem, LVIS_SELECTED | LVIS_FOCUSED, 
				LVIS_SELECTED | LVIS_FOCUSED);
		}
	}

	int selected_row;
	selected_row = p_list->GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row != -1) {
		// アイテムを可視にする
		p_list->EnsureVisible(selected_row, FALSE);
	}

	return 0;

ERR1:
	MessageBox(_T(""), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

void CTableListDlg::OnItemchangedTableList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	*pResult = 0;

	CheckButtons();

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;
	if((pNMListView->uNewState & (LVIS_SELECTED | LVNI_ALL)) != 0) {
		SetColumnList(m_owner, GetSchemaName(), GetTableName(), GetObjectType(), TRUE);
	}
}

void CTableListDlg::OnItemchangedColumnList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	*pResult = 0;

	CheckButtons();
}

CString CTableListDlg::GetAllTableName()
{
	int selected_row = m_table_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return _T("");

	CString table_name;

	if(m_table_list.GetItemText(selected_row, SCHEMA_NAME).IsEmpty()) {
		table_name.Format(_T("`%s`"),
			m_table_list.GetItemText(selected_row, TABLE_NAME));
	} else {
		table_name.Format(_T("`%s`.`%s`"),
			m_table_list.GetItemText(selected_row, SCHEMA_NAME),
			m_table_list.GetItemText(selected_row, TABLE_NAME));
	}
	return table_name;
}

CString CTableListDlg::GetTableName()
{
	int selected_row = m_table_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return _T("");

	return m_table_list.GetItemText(selected_row, TABLE_NAME);
}

CString CTableListDlg::GetObjectType()
{
	int selected_row = m_table_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return _T("");

	return m_table_list.GetItemText(selected_row, OBJECT_TYPE);
}

CString CTableListDlg::GetSchemaName()
{
	int selected_row = m_table_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return _T("");

	return m_table_list.GetItemText(selected_row, SCHEMA_NAME);
}

CString CTableListDlg::GetColumnDataType(const TCHAR *col_name)
{
	int		i;
	
	for(i = 0; i < m_column_list.GetItemCount(); i++) {
		if(m_column_list.GetItemText(i, TABLE_COLUMN_NAME).Compare(col_name) == 0) {
			return m_column_list.GetItemText(i, DATA_TYPE);
		}
	}

	return "";
}

BOOL CTableListDlg::MakeColumnNameList(BOOL b_check)
{
	m_column_name_list.RemoveAll();

	for(int i = 0; i < m_selected_column_list.GetItemCount(); i++) {
		if(b_check) {
			CString data_type = GetColumnDataType(m_selected_column_list.GetItemText(i, 0));
			if(data_type == _T("") || data_type == _T("LONG RAW")) {
				CString msg;
				msg.Format(_T("%sはサポートされていません"), data_type);
				MessageBox(msg, _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
		}

		m_column_name_list.AddTail(m_selected_column_list.GetItemText(i, 0));
	}

	return TRUE;
}

void CTableListDlg::OnButtonSelectAllColumn() 
{
	int		i;
	for(i = 0; i < m_column_list.GetItemCount(); i++) {
		m_column_list.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, 
			LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void CTableListDlg::CreateEditCtrl()
{
	RECT column_list_rect;
	GetDlgItem(IDC_COLUMN_LIST)->GetWindowRect(&column_list_rect);
	ScreenToClient(&column_list_rect);

	RECT selected_column_list_rect;
	GetDlgItem(IDC_SELECTED_COLUMN_LIST)->GetWindowRect(&column_list_rect);
	ScreenToClient(&selected_column_list_rect);
	
	RECT ok_button_rect;
	GetDlgItem(IDOK)->GetWindowRect(&ok_button_rect);
	ScreenToClient(&ok_button_rect);

	m_edit_data.set_undo_cnt(INT_MAX);
	m_edit_data.replace_all(m_where_clause.GetBuffer(0));
	m_edit_data.reset_undo();
	m_edit_data.set_str_token(m_str_token);

	m_edit_ctrl.SetEditData(&m_edit_data);
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, 
		CRect(column_list_rect.left, column_list_rect.bottom + 5,
			selected_column_list_rect.right, ok_button_rect.top - 10), 
		this, NULL);
	m_edit_ctrl.SetExStyle2(ECS_ON_DIALOG);
	m_edit_ctrl.SetFont(m_font);

	m_edit_ctrl.SetWindowPos(GetDlgItem(IDC_SELECTED_COLUMN_LIST), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE| SWP_NOACTIVATE);

//	SetEditColor();
//	SetEditorOption();
}

void CTableListDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	RelayoutControl();
}

void CTableListDlg::RelayoutControl()
{
	if(m_init == FALSE) return;

	CRect	win_rect;
	CRect	ctrl_rect;
	CRect	ok_rect;
	CRect	static_column_rect;

	GetClientRect(&win_rect);
	m_ok.GetClientRect(&ok_rect);
	m_static_column.GetClientRect(&static_column_rect);

	int		ctrl_left = static_column_rect.right + 10;
	int		y_top, height;
	y_top = 10;

	m_combo_owner.GetClientRect(&ctrl_rect);
	m_static_owner.SetWindowPos(NULL, 10, y_top, 0, 0,
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);
	m_combo_owner.SetWindowPos(NULL, ctrl_left, y_top,
		ctrl_rect.Width(), ctrl_rect.Height(),
		SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOZORDER);

	y_top = y_top + ctrl_rect.Height() + 5;
	m_btn_filter.GetClientRect(&ctrl_rect);
	m_static_filter.SetWindowPos(NULL, 10, y_top, 0, 0, 
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);
	m_edit_filter.SetWindowPos(NULL, ctrl_left, y_top,
		win_rect.right - (ctrl_left + 15) - ctrl_rect.Width(), ctrl_rect.Height(),
		SWP_NOOWNERZORDER | SWP_NOCOPYBITS| SWP_NOZORDER);
	m_btn_filter.SetWindowPos(NULL, win_rect.right - ctrl_rect.Width() - 10, y_top, 0, 0,
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS| SWP_NOZORDER);

	y_top = y_top + ctrl_rect.Height() + 5;
	height = (win_rect.bottom - (y_top + ok_rect.Height() + 20)) / 3;
	m_static_table.SetWindowPos(NULL, 10, y_top, 0, 0, 
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS| SWP_NOZORDER);
	m_table_list.SetWindowPos(NULL, ctrl_left, y_top, win_rect.right - (ctrl_left + 10), height,
		SWP_NOOWNERZORDER | SWP_NOCOPYBITS| SWP_NOZORDER);

	y_top = y_top + height + 5;
	m_static_column.SetWindowPos(NULL, 10, y_top, 0, 0, 
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);

	{
		CRect add_btn_rect;
		m_btn_add.GetClientRect(add_btn_rect);

		int column_list_left = ctrl_left;
		int column_list_right = column_list_left + ((win_rect.Width() - ctrl_left) / 5) * 3;

		m_column_list.SetWindowPos(NULL, column_list_left, y_top, 
			column_list_right - column_list_left, height, 
			SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOZORDER);

		int selected_column_list_left = column_list_right + add_btn_rect.Width() + 10;
		int selected_column_list_right = win_rect.Width() - 10;

		m_selected_column_list.SetWindowPos(NULL, selected_column_list_left, y_top, 
			selected_column_list_right - selected_column_list_left, height, 
			SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOZORDER);

		m_btn_add.SetWindowPos(NULL, column_list_right + 5, y_top + 5, 
			0, 0, 
			SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOSIZE);
		m_btn_del.SetWindowPos(NULL, column_list_right + 5, y_top + add_btn_rect.Height() + 10, 
			0, 0, 
			SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOSIZE);
		m_btn_clear_column.SetWindowPos(NULL, column_list_right + 5, y_top + add_btn_rect.Height() * 2 + 15, 
			0, 0, 
			SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOSIZE);

		m_spin_selected_column.SetWindowPos(NULL, column_list_right + 15, 
			y_top + add_btn_rect.Height() * 3 + 20, 
			0, 0, 
			SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOSIZE);
	}

	m_static_column.GetClientRect(&ctrl_rect);
	m_sel_all_btn.SetWindowPos(NULL, 10, y_top + (ctrl_rect.bottom - ctrl_rect.top) + 10,
		0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);

	y_top = y_top + height + 5;
	m_static_where.SetWindowPos(NULL, 10, y_top, 0, 0, 
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);
	m_edit_ctrl.SetWindowPos(NULL, 60, y_top, win_rect.right - 70, height, 
		SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOZORDER);

	y_top = y_top + height + 5;

	int		x;
	m_ok.GetClientRect(&ctrl_rect);
	x = (win_rect.right - win_rect.left) / 2 - (ctrl_rect.right - ctrl_rect.left + 5);
	m_ok.SetWindowPos(NULL, x, y_top, 0, 0,
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);
	x = (win_rect.right - win_rect.left) / 2 + 5;
	m_cancel.SetWindowPos(NULL, x, y_top, 0, 0,
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);

	x = x + ctrl_rect.Width() + 5;
	m_check_data_lock.SetWindowPos(NULL, x, y_top, 0, 0,
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);

	m_btn_save.GetClientRect(&ctrl_rect);
	x = 10;
	m_btn_save.SetWindowPos(NULL, x, y_top, 0, 0, 
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);
	x = x + ctrl_rect.Width() + 5;
	m_btn_load.SetWindowPos(NULL, x, y_top, 0, 0,
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOZORDER);
}

void CTableListDlg::OnDestroy() 
{
	CRect	win_rect;
	GetWindowRect(&win_rect);

	AfxGetApp()->WriteProfileInt(_T("TABLE_DLG"), _T("WIDTH"), win_rect.Width());
	AfxGetApp()->WriteProfileInt(_T("TABLE_DLG"), _T("HEIGHT"), win_rect.Height());

	CDialog::OnDestroy();
}

BOOL CTableListDlg::SelectListData(CListCtrl *list, TCHAR *key, BOOL b_visible)
{
	int		i;
	for(i = 0; i < list->GetItemCount(); i++) {
		if(list->GetItemText(i, 0) == key) {
			list->SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, 
				LVIS_SELECTED | LVIS_FOCUSED);
			if(b_visible) list->EnsureVisible(i, FALSE);
			return TRUE;
		}
	}

	return FALSE;
}

void CTableListDlg::ClearListSelected(CListCtrl *list)
{
	list->SetItemState(-1, 0, LVIS_SELECTED | LVIS_FOCUSED);
}

void CTableListDlg::SplitSchemaAndTable(const TCHAR *name, CString &schema_name, CString &table_name)
{
	CString table_name1 = name;

	if(table_name1.Find(_T(".")) == -1) {
		schema_name = _T("");
		table_name = table_name1;
		table_name.Replace(_T("\""), _T(""));
		table_name.Replace(_T("`"), _T(""));
		return;
	}

	schema_name = table_name1.Left(table_name1.Find(_T(".")));
	table_name = table_name1.Right(table_name1.GetLength() - table_name1.Find(_T(".")) - 1);

	schema_name.Replace(_T("\""), _T(""));
	table_name.Replace(_T("\""), _T(""));
	schema_name.Replace(_T("`"), _T(""));
	table_name.Replace(_T("`"), _T(""));
}

CString CTableListDlg::GetTableOwner(const TCHAR *name)
{
	CString table_name;
	CString schema_name;
	TCHAR	sql_buf[4096];
	HMyDataset dataset = NULL;
	CString owner = _T("");

	SplitSchemaAndTable(name, schema_name, table_name);

	_stprintf(sql_buf,
		_T("SELECT table_schema \n")
		_T("FROM information_schema.tables \n")
		_T("WHERE table_name = '%s' \n"),
		table_name.GetBuffer(0));

	dataset = my_create_dataset(m_ss, sql_buf, m_msg_buf);

	if(dataset != NULL && my_dataset_row_cnt(dataset) >= 1) {
		owner = my_dataset_data(dataset, 0, 0);
	}

	my_free_dataset(dataset);

	return owner;
}

BOOL CTableListDlg::SelectTable(TCHAR *table_name)
{
	CString table_name2;
	CString schema_name;

	SplitSchemaAndTable(table_name, schema_name, table_name2);

	if(schema_name.IsEmpty()) {
		return SelectListData(&m_table_list, table_name, TRUE);
	}

	int		i;
	CListCtrl *list = &m_table_list;
	for(i = 0; i < list->GetItemCount(); i++) {
		if(list->GetItemText(i, TABLE_NAME) == table_name2 &&
			list->GetItemText(i, SCHEMA_NAME) == schema_name) {
			list->SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, 
				LVIS_SELECTED | LVIS_FOCUSED);
			list->EnsureVisible(i, FALSE);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CTableListDlg::LoadFile(TCHAR *file_name)
{
	TCHAR	buf[1024];
	FILE	*fp = NULL;
	fp = _tfopen(file_name, _T("r"));
	if(fp == NULL) {
		MessageBox(_T("ファイルが開けません"), _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	int		i;

	// テーブル名 or オーナー名
	for(i = 0;; i++) {
		if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) goto ERR1;
		ostr_chomp(buf, '\0');

		if(_tcscmp(buf, _T("[OWNER_NAME]")) == 0) break;
		if(_tcscmp(buf, _T("[TABLE_NAME]")) == 0) break;
	}
	// スキーマ名の処理
	if(_tcscmp(buf, _T("[OWNER_NAME]")) == 0) {
		if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) goto ERR1;
		ostr_chomp(buf, '\0');
		m_owner = buf;
	}
	if(SetUserList(m_owner) != 0 || m_owner.IsEmpty()) {
		MessageBox(_T("オーナーが見つかりません"), _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
		fclose(fp);
		return FALSE;
	}

	SetTableList(m_owner);

	// テーブル名
	if(_tcscmp(buf, _T("[TABLE_NAME]")) != 0) {
		for(i = 0;; i++) {
			if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) goto ERR1;
			ostr_chomp(buf, '\0');

			if(_tcscmp(buf, _T("[TABLE_NAME]")) == 0) break;
		}
	}
	if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) goto ERR1;
	ostr_chomp(buf, '\0');

	if(SelectTable(buf) == FALSE) {
		CString table_owner = GetTableOwner(buf);
		if(!table_owner.IsEmpty()) {
			SetUserList(table_owner);
			SetTableList(table_owner);
		}

		if(table_owner.IsEmpty() || SelectTable(buf) == FALSE) {
			CString msg;
			msg.Format(_T("テーブルが見つかりません (%s)"), buf);
			MessageBox(msg, _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
			fclose(fp);
			return FALSE;
		}
	}

	// エイリアス名
	if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) goto ERR1;
	ostr_chomp(buf, '\0');
	if(_tcscmp(buf, _T("[ALIAS_NAME]")) == 0) {
		if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) goto ERR1;
		ostr_chomp(buf, '\0');
		m_alias_name = buf;

		// カラム名のヘッダを読む
		if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) goto ERR1;
		ostr_chomp(buf, '\0');
	}

	// カラム名
	if(_tcscmp(buf, _T("[COLUMN_NAME]")) != 0) goto ERR1;

	ClearListSelected(&m_column_list);
	for(i = 0;; i++) {
		if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) goto ERR1;
		ostr_chomp(buf, '\0');		

		if(_tcscmp(buf, _T("[WHERE]")) == 0) break;
		if(AddColumn(buf, TRUE) == FALSE) {
			MessageBox(_T("カラムが見つかりません"), _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
			fclose(fp);
			return FALSE;
		}
	}

	// where句
	m_edit_data.del_all();
	for(i = 0;; i++) {
		if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) break;
		m_edit_data.paste(buf, FALSE);
	}
	m_edit_data.recalc_disp_size();
	m_edit_ctrl.Redraw();

	UpdateData(FALSE);

	fclose(fp);
	return TRUE;

ERR1:
	MessageBox(_T("ファイルフォーマットが違います"), _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
	fclose(fp);
	return FALSE;
}

void CTableListDlg::OnBtnLoad() 
{
	TCHAR file_name[_MAX_PATH];

	if(DoFileDlg(_T("検索条件を読込み"), TRUE, _T("osg"), NULL,
		OFN_FILEMUSTEXIST | OFN_EXPLORER,
		_T("PSqlGrid Files (*.psg)|*.psg|All Files (*.*)|*.*||"),
		AfxGetMainWnd(), file_name) == FALSE) return;

	LoadFile(file_name);
	CheckButtons();
}

void CTableListDlg::OnBtnSave() 
{
	TCHAR file_name[_MAX_PATH];

	if(DoFileDlg(_T("検索条件を保存"), FALSE, _T("osg"), NULL,
		OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("PSqlGrid Files (*.psg)|*.psg|All Files (*.*)|*.*||"),
		AfxGetMainWnd(), file_name) == FALSE) return;

	UpdateData(TRUE);

	if(m_selected_column_list.GetItemCount() == 0) {
		OnButtonAdd();
	}

	m_owner = GetOwner();
	m_table_name = GetAllTableName();
	if(m_table_name == _T("")) return;
	m_where_clause = m_edit_data.get_all_text();

	MakeColumnNameList(FALSE);

	FILE	*fp = NULL;
	fp = _tfopen(file_name, _T("w"));
	if(fp == NULL) {
		MessageBox(_T("ファイルが開けません"), _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	_ftprintf(fp, _T("OSqlGrid 検索条件設定ファイル\n"));
	_ftprintf(fp, _T("--- DO NOT EDIT THIS FILE ---\n"));
	_ftprintf(fp, _T("[OWNER_NAME]\n"));
	_ftprintf(fp, _T("%s\n"), m_owner.GetBuffer(0));
	_ftprintf(fp, _T("[TABLE_NAME]\n"));
	_ftprintf(fp, _T("%s\n"), m_table_name.GetBuffer(0));
	_ftprintf(fp, _T("[COLUMN_NAME]\n"));
	POSITION	pos;
	for(pos = m_column_name_list.GetHeadPosition(); pos != NULL; m_column_name_list.GetNext(pos)) {
		_ftprintf(fp, _T("%s\n"), m_column_name_list.GetAt(pos).GetBuffer(0));
	}
	_ftprintf(fp, _T("[WHERE]\n"));
	_ftprintf(fp, _T("%s\n"), m_where_clause.GetBuffer(0));

	fclose(fp);
}

void CTableListDlg::OnBtnFilter() 
{
	m_edit_filter.SetWindowText(_T(""));
}

void CTableListDlg::OnChangeEditFilter() 
{
	m_table_name = GetAllTableName();

	SetTableList(m_table_dataset);

	if(GetAllTableName() == _T("")) {
		m_column_list.DeleteAllItems();
		m_selected_column_list.DeleteAllItems();
	}

	POINT pt;
	::GetCursorPos(&pt);
	::SetCursorPos(pt.x, pt.y);
}

void CTableListDlg::SetKeywordOK(BOOL b_ok)
{
	if(m_keyword_is_ok == b_ok) return;

	m_keyword_is_ok = b_ok;
	m_edit_filter.RedrawWindow();
}

HBRUSH CTableListDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(pWnd == &m_edit_filter && m_keyword_is_ok == FALSE) {
		pDC->SetTextColor(RGB(255, 0, 0));
	}
	
	return hbr;
}

void CTableListDlg::CheckAddDelBtn()
{
	m_btn_add.EnableWindow(m_column_list.GetSelectedCount() > 0);
	m_btn_del.EnableWindow(m_selected_column_list.GetSelectedCount() > 0);
	m_btn_clear_column.EnableWindow(m_selected_column_list.GetItemCount() > 0);
}

BOOL CTableListDlg::AddColumn(const TCHAR *column_name, BOOL b_check)
{
	LV_ITEM		item;
	int			i;

	if(b_check) {
		for(i = 0; i < m_column_list.GetItemCount(); i++) {
			if(m_column_list.GetItemText(i, TABLE_COLUMN_NAME).Compare(column_name) == 0) break;
		}
		if(i == m_column_list.GetItemCount()) return FALSE;
	}

	for(i = 0; i < m_selected_column_list.GetItemCount(); i++) {
		if(m_selected_column_list.GetItemText(i, SELECTED_COLUMN_NAME).Compare(column_name) == 0) {
			m_selected_column_list.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, 
				LVIS_SELECTED | LVIS_FOCUSED);
			return TRUE;
			//m_selected_column_list.DeleteItem(i);
			//break;
		}
	}

	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.iSubItem = 0;

	item.iItem = m_selected_column_list.GetItemCount();
	item.pszText = (TCHAR *)column_name;
	int nItem = m_selected_column_list.InsertItem(&item);

	m_selected_column_list.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, 
		LVIS_SELECTED | LVIS_FOCUSED);

	return TRUE;
}

void CTableListDlg::OnButtonAdd() 
{
	int nItem = -1;

	ClearListSelected(&m_selected_column_list);

	for(;;) {
		nItem = m_column_list.GetNextItem(nItem, LVNI_SELECTED | LVNI_ALL);
		if(nItem == -1) break;

		AddColumn(m_column_list.GetItemText(nItem, TABLE_COLUMN_NAME), FALSE);
	}

	CheckButtons();
}

void CTableListDlg::OnButtonClearColumnList() 
{
	m_selected_column_list.DeleteAllItems();

	CheckButtons();
}

void CTableListDlg::OnButtonDel() 
{
	for(;;) {
		int nItem = m_selected_column_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
		if(nItem == -1) break;

		m_selected_column_list.DeleteItem(nItem);
	}

	CheckButtons();
}

void CTableListDlg::OnItemchangedSelectedColumnList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	CheckButtons();
	
	*pResult = 0;
}

void CTableListDlg::OnDblclkColumnList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnButtonAdd();
	
	*pResult = 0;
}

void CTableListDlg::SwapSelectedColumn(int iDelta)
{
	if(iDelta != -1 && iDelta != 1) return;

	int nItem = m_selected_column_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(nItem == -1) return;

	int sp_nItem = nItem + iDelta;
	if(sp_nItem < 0 || sp_nItem >= m_selected_column_list.GetItemCount()) return;

	CString col_name = m_selected_column_list.GetItemText(nItem, 0);
	m_selected_column_list.SetItemText(nItem, 0, m_selected_column_list.GetItemText(sp_nItem, 0));
	m_selected_column_list.SetItemText(sp_nItem, 0, col_name);

	ClearListSelected(&m_selected_column_list);
	m_selected_column_list.SetItemState(sp_nItem, LVIS_SELECTED | LVIS_FOCUSED, 
		LVIS_SELECTED | LVIS_FOCUSED);
	m_selected_column_list.EnsureVisible(sp_nItem, FALSE);
}

void CTableListDlg::OnDeltaposSpinSelectedColumn(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	SwapSelectedColumn(pNMUpDown->iDelta);
	
	*pResult = 0;
}

void CTableListDlg::CheckSpinBtn()
{
	m_spin_selected_column.EnableWindow(m_selected_column_list.GetSelectedCount() == 1);
}

void CTableListDlg::CheckButtons()
{
	CheckOKButton();
	CheckAddDelBtn();
	CheckSpinBtn();
}

int CTableListDlg::SetUserList(const TCHAR *user_name)
{
	CWaitCursor	wait_cursor;
	int			ret_v = 0;
	int			i;
	int			item;
	HMyDataset	dataset = NULL;
	CComboBox	*p_combo = &m_combo_owner;

	if(!IsWindow(p_combo->GetSafeHwnd())) goto ERR1;

	for(;;) {
		if(p_combo->DeleteString(0) == CB_ERR) break;
	}

	dataset = get_user_list(g_ss, g_msg_buf);
	if(dataset == NULL) {
		MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		goto ERR1;
	}

	for(i = 0; i < my_dataset_row_cnt(dataset); i++) {
		item = p_combo->AddString(my_dataset_data(dataset, i, 0));
		if(item == CB_ERR) {
			ret_v = 1;
			goto ERR1;
		}
		const TCHAR *user = my_dataset_data(dataset, i, 0);
		if(_tcscmp(user_name, user) == 0) {
			p_combo->SetCurSel(item);
		}
	}

	if(p_combo->GetCurSel() == CB_ERR) {
		const TCHAR *cur_db = my_db(g_ss);
		for(i = 0; i < p_combo->GetCount(); i++) {
			CString str;
			p_combo->GetLBText(i, str);
			if(_tcscmp(cur_db, str) == 0) {
				p_combo->SetCurSel(i);
				break;
			}
		}
	}
/*
	item = p_combo->AddString(ALL_USERS);
	if(item == CB_ERR) {
		ret_v = 1;
		goto ERR1;
	} else {
		if(_tcscmp(user_name, ALL_USERS) == 0) {
			p_combo->SetCurSel(item);
		}
	}
*/
	my_free_dataset(dataset);
	return 0;

ERR1:
	my_free_dataset(dataset);
	return ret_v;
}

CString CTableListDlg::GetOwner()
{
	if(m_combo_owner.GetCurSel() == CB_ERR) {
		return _T("");
	}

	CString owner;
	m_combo_owner.GetLBText(m_combo_owner.GetCurSel(), owner);
	return owner;
}

void CTableListDlg::OnSelchangeComboOwner()
{
	m_owner = GetOwner();
	SetTableList(m_owner);
}
