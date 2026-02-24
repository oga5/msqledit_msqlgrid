/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "sqlparse.h"
#include "global.h"

#include "oregexp.h"

// 行の末尾に';'があるかチェックする(スペースは無視する)
BOOL is_sql_end(CEditData *edit_data, int row, int *semicolon_col, BOOL in_quote)
{
	const TCHAR *p_start = edit_data->get_row_buf(row);
	const TCHAR *p = p_start;
	CStrToken *token = edit_data->get_str_token();

	ASSERT(token == &g_sql_str_token);

	// MySQL does not use dollar-quoting, so in_quote mode is not needed
	(void)in_quote;

	for(; *p != '\0';) {
		if(*p == ';') {
			int char_type = edit_data->check_char_type(row, (int)(p - p_start));
			if(char_type == CHAR_TYPE_NORMAL) {
				if(semicolon_col) *semicolon_col = (int)(p - p_start);
				return TRUE;
			}
		}
		p += token->get_word_len(p);
	}

	return FALSE;
}

