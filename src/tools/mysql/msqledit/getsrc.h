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


#ifndef __GET_SRC_H_INCLUDED__
#define __GET_SRC_H_INCLUDED__

#include "editdata.h"

int create_object_source(HMySession ss, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name, const TCHAR *schema,
	CEditData *edit_data, TCHAR *msg_buf, BOOL recalc_disp_size = TRUE);

// getsrc2.cpp
int dumpTableSchema(HMySession ss, const TCHAR *oid, CEditData *edit_data, TCHAR *msg_buf);
int dumpIndexSchema(HMySession ss, const TCHAR* table_oid, const TCHAR* index_oid,
	CEditData* edit_data, TCHAR* msg_buf);

#endif  // __GET_SRC_H_INCLUDED__