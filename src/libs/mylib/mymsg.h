/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _MYLIB_MY_MSG_H_INCLUDE_
#define _MYLIB_MY_MSG_H_INCLUDE_

#define MYERR_MEM_ALLOC             100
#define MYERR_MEM_ALLOC_MSG         _T("memory allocation error.")

#define MYERR_OPEN_FILE             101
#define MYERR_OPEN_FILE_MSG         _T("ファイルを開けません(%s)。")

#define MYERR_CLOSE_FILE            102
#define MYERR_CLOSE_FILE_MSG        _T("ファイルのクローズに失敗しました。")

#define MYERR_LOAD_LIBMYSQL         103
#define MYERR_LOAD_LIBMYSQL_MSG     _T("libmysql.dllが見つかりません")

#define MYERR_INIT_LIBMYSQL         104
#define MYERR_INIT_LIBMYSQL_MSG     _T("libmysql.dllの初期化に失敗しました。\nlibmysql.dllのバージョンが古い可能性があります。")

#define MYERR_FREE_LIBMYSQL         105
#define MYERR_FREE_LIBMYSQL_MSG     _T("libmysql.dllの開放に失敗しました。")

#define MYERR_CANCEL                110
#define MYERR_CANCEL_MSG            _T("キャンセルされました。")

#define MYERR_NOT_LOGIN             200
#define MYERR_NOT_LOGIN_MSG         _T("ログインできませんでした。")

#define MY_ERR_FATAL                199

#define MYMSG_SELECT_MSG                    _T("%d件選択されました。")
#define MYMSG_NROW_OK_MSG                   _T("%s行処理されました。")
#define MYMSG_SQL_OK                        _T("SQLが実行されました。")

#define MYMSG_DB_SEARCH_MSG                 _T("検索実行中")
#define MYMSG_DISP_SEARCH_RESULT_MSG        _T("検索結果表示中")

/* Cancel dialog window messages */
#ifdef WIN32
#include <windows.h>
#define WM_OCI_DLG_EXIT             (WM_USER + 100)
#define WM_OCI_DLG_STATIC           (WM_USER + 101)
#define WM_OCI_DLG_ENABLE_CANCEL    (WM_USER + 102)
#define WM_OCI_DLG_ROW_CNT          (WM_USER + 103)
#endif

#endif /* _MYLIB_MY_MSG_H_INCLUDE_ */
