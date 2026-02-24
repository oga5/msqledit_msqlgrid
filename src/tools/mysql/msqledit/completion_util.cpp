/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "global.h"

TCHAR *completion_object_type_list[COT_COUNT] = {_T("TABLE"), _T("VIEW")};
TCHAR *completion_object_type_list_sql[COT_COUNT] = {_T("'BASE TABLE'"), _T("'VIEW'")};

CString make_completion_object_type()
{
	BOOL	first = TRUE;
	CString	result = "";

	for(int i = 0; i < COT_COUNT; i++) {
		if(g_option.completion_object_type[i] != FALSE) {
			if(first) {
				first = FALSE;
			} else {
				result += _T(",");
			}
			result += completion_object_type_list_sql[i];
		}
	}

	return result;
}
