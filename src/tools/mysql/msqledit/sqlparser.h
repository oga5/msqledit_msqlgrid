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

#ifndef __SQL_PAESER_H_INCLUDED__
#define __SQL_PAESER_H_INCLUDED__

#include "EditData.h"
#include "sqlstrtoken.h"

typedef enum {
	SQL_NORMAL,
	SQL_PLSQL,
	SQL_SELECT,
	SQL_COMMAND,
	SQL_FILE_RUN,
	SQL_BLANK,
} SQL_KIND;

class CSqlParser
{
public:
	CSqlParser();
	~CSqlParser();

	void set_str_token(CSQLStrToken *token) { m_str_token = token; }

	const TCHAR *GetSqlBuf() {
		if(m_sql_buf == NULL) return _T("");
		return m_sql_buf;
	}
	const TCHAR *GetSkipSpaceSql() {
		if(m_skip_space_sql == NULL) return _T("");
		return m_skip_space_sql;
	}
	SQL_KIND GetSqlKind() { return m_sql_kind; }
	int GetSQL(int start_row, CEditData *edit_data);

	void FreeSqlBuf();

private:
	TCHAR *AllocSqlBuf(int size);
	BOOL is_sql_end2(const TCHAR *p, int *semicolon_col);

	CSQLStrToken	*m_str_token;
	TCHAR			*m_sql_buf;
	int				m_sql_buf_alloc_cnt;
	TCHAR			*m_skip_space_sql;
	SQL_KIND		m_sql_kind;
};

// global function
BOOL is_sql_end(CEditData *edit_data, int row, CStrToken *token);

#endif __SQL_PAESER_H_INCLUDED__
