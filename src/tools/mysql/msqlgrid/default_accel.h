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

#include "resource.h"

static const ACCEL default_accel[] = {
	{FVIRTKEY | FCONTROL | FNOINVERT, 'Z', ID_EDIT_UNDO},
	{FVIRTKEY | FALT | FNOINVERT, VK_BACK, ID_EDIT_UNDO},
	{FVIRTKEY | FCONTROL | FNOINVERT, 'Y', ID_EDIT_REDO},

	{FVIRTKEY | FCONTROL | FNOINVERT, 'C', ID_EDIT_COPY},
	{FVIRTKEY | FCONTROL | FNOINVERT, VK_INSERT, ID_EDIT_COPY},
	{FVIRTKEY | FCONTROL | FNOINVERT, 'X', ID_EDIT_CUT},
	{FVIRTKEY | FSHIFT | FNOINVERT, VK_DELETE, ID_EDIT_CUT},
	{FVIRTKEY | FCONTROL | FNOINVERT, 'V', ID_EDIT_PASTE},
	{FVIRTKEY | FSHIFT | FNOINVERT, VK_INSERT, ID_EDIT_PASTE},

	{FVIRTKEY | FCONTROL | FNOINVERT, 'A', ID_EDIT_SELECT_ALL},

	{FVIRTKEY | FNOINVERT, VK_F2, ID_ENTER_EDIT},
	{FVIRTKEY | FALT | FNOINVERT, VK_RETURN, ID_INPUT_ENTER},

	{FVIRTKEY | FCONTROL | FNOINVERT, 'F', ID_SEARCH},
	{FVIRTKEY | FCONTROL | FNOINVERT, 'H', ID_REPLACE},
	{FVIRTKEY | FNOINVERT, VK_F3, ID_SEARCH_NEXT},
	{FVIRTKEY | FALT | FNOINVERT, VK_F3, ID_CLEAR_SEARCH_TEXT},
	{FVIRTKEY | FSHIFT | FNOINVERT, VK_F3, ID_SEARCH_PREV},
};

