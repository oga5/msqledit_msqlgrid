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

#if !defined(_ACCEL_LIST_INCLUDED_)
#define _ACCEL_LIST_INCLUDED_

class CAccelList
{
private:
	ACCEL	*m_accel_list;
	int		m_accel_cnt;
	int		m_accel_alloc_cnt;

	BOOL realloc_accel();
	void free_accel();

	BOOL add_accel(WORD cmd, WORD key, BYTE fVirt);
	BOOL delete_accel(int idx);
	BOOL delete_accel(WORD key, BYTE fVirt);

	int search_accel_from_key(WORD key, BYTE fVirt);
	int search_accel_from_cmd(WORD cmd, int pos);

	static TCHAR *get_key_name(WORD key);
	static WORD get_key_code(TCHAR *key_name);
	static BOOL get_accel_info(TCHAR *accel_str, WORD &key, BYTE &fVirt);

	void make_reg_key(int idx, TCHAR *cmd_name, TCHAR *key_name, TCHAR *virt_name);

public:
	CAccelList();
	~CAccelList();

	int search_accel_str(WORD cmd, int pos, CString &accel_str);
	int search_accel_str2(WORD cmd, CString &accel_str);
	int search_cmd(TCHAR *accel_str);
	BOOL add_accel(WORD cmd, TCHAR *accel_str);
	BOOL delete_accel(TCHAR *accel_str);
	BOOL delete_accel_from_cmd(WORD cmd);

	BOOL load_accel_list();
	BOOL load_default_accel_list();
	BOOL save_accel_list();

	ACCEL *get_accel_list() { return m_accel_list; }
	int get_accel_cnt() { return m_accel_cnt; }

	static CString get_accel_str(WORD key, BOOL alt, BOOL ctrl, BOOL shift);

	void operator=(CAccelList &accel_list);
	void make_no_allow_key_list(CAccelList &accel_list);

	CString GetCommandName(UINT nID);
	CString GetCommandInfo(UINT nID);
};

#endif _ACCEL_LIST_INCLUDED_
