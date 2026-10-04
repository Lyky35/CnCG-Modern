/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef COPYPROTECTION_H
#define COPYPROTECTION_H

class CopyProtect
	{
	public:
		static Bool isLauncherRunning(void) { return TRUE; }
		static Bool notifyLauncher(void) { return TRUE; }
		static void checkForMessage(UINT message, LPARAM lParam) { (void)message; (void)lParam; }
		static Bool validate(void) { return TRUE; }
		static void shutdown(void) {}
	};

#endif // COPYPROTECTION_H
