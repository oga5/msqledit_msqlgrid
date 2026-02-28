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
