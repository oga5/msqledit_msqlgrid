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

#ifndef __CONNECT_INFO_H_INCLUDED__
#define __CONNECT_INFO_H_INCLUDED__

enum connect_info_list {
	LIST_USER,
	LIST_PASSWD,
	LIST_DBNAME,
	LIST_HOST,
	LIST_PORT,
	LIST_CHARSET,
	LIST_CONNECT_NAME,
};

class CConnectInfo
{
public:
	CConnectInfo(TCHAR *user, TCHAR *passwd, TCHAR *dbname, TCHAR *host, TCHAR *port, TCHAR *charset,
		TCHAR *connect_name);

	CString		m_user;
	CString		m_passwd;
	CString		m_dbname;
	CString		m_host;
	CString		m_port;
	CString		m_charset;
	CString		m_connect_name;
};

class CConnectList
{
public:
	CConnectList();
	CConnectList(const TCHAR *opt_prof_name, const TCHAR *opt_registry_key);
	~CConnectList();

	BOOL load_list();
	BOOL save_list();
	BOOL add_list(TCHAR *user, TCHAR *passwd, TCHAR *dbname, TCHAR *host, TCHAR *port, TCHAR *charset,
		TCHAR *connect_name);
	CPtrList	*GetList() { return &m_connect_list; }

private:
	CPtrList	m_connect_list;
	CString		m_opt_prof_name;
	CString		m_opt_registry_key;

	void clear_list();
};

#endif // __CONNECT_INFO_H_INCLUDED__
