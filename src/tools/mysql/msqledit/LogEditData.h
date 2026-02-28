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

#ifndef _LOG_EDIT_DATA_H_INCLUDE
#define _LOG_EDIT_DATA_H_INCLUDE

#include "editdata.h"
#include "UnicodeArchive.h"

class CLogEditData : public CEditData
{
public:
	CLogEditData();
	~CLogEditData();

	virtual int paste(const TCHAR *pstr, BOOL recalc = TRUE);

	void log_start(CUnicodeArchive *ar);
	void log_end();
	BOOL is_logging() { return m_logging; }

private:
	BOOL	m_logging;
	CUnicodeArchive	*m_ar;
	TCHAR	*m_buf;
	int		m_buf_size;

	void puts_logfile(const TCHAR *pstr, CUnicodeArchive *ar);
};

#endif _LOG_EDIT_DATA_H_INCLUDE
