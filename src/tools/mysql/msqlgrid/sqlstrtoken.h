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

#if !defined(_SQL_STR_TOKEN_INCLUDED_)
#define _SQL_STR_TOKEN_INCLUDED_

#include "strtoken.h"
#include "mylib.h"

class CSQLStrToken : public CStrToken
{
private:
	CRITICAL_SECTION m_critical_section;
	volatile bool m_is_init_keyword;
	struct _key_word_st	*m_completion_words;
	int m_completion_word_cnt;

	HMyDataset m_dset_table;
	HMyDataset m_dset_column;

	const TCHAR *getObjectName(const TCHAR *p, TCHAR *name, int name_buf_size);

	void initCharTable();
	void freeCompletionKeyword();
	int addDatasetToKeywordList(HMyDataset dataset, const TCHAR *owner, TCHAR *msg_buf,
		int column_idx, int org_column_idx, int type_idx);

	BOOL CheckSQL(const TCHAR *sql, const TCHAR *keyword);

public:
	BOOL isCommand(const TCHAR *sql);
	BOOL isExecuteCommand(const TCHAR *sql);
	BOOL isSelectSQL(const TCHAR *sql);
	BOOL isExplainSQL(const TCHAR *sql);
	BOOL isPLSQL(const TCHAR *sql, TCHAR *object_type, TCHAR *object_name);
	CSQLStrToken();
	virtual ~CSQLStrToken();

	virtual const TCHAR *skipCommentAndSpace(const TCHAR *p);

	int initCompletionWord(HMySession ss, 
		const TCHAR *owner, TCHAR *msg_buf, BOOL b_object_name, BOOL b_column_name,
		CString object_type_list = "");

	void FreeDataset();
};

#endif _SQL_STR_TOKEN_INCLUDED_
