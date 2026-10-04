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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// Win32OSDisplay.cpp //////////////////////////////////////
// John McDonald, December 2002
////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellscalingapi.h>
#include "Common/OSDisplay.h"

#include "Common/SubsystemInterface.h"
#include "Common/STLTypeDefs.h"
#include "Common/AsciiString.h"
#include "Common/SystemInfo.h"
#include "Common/UnicodeString.h"
#include "GameClient/GameText.h"

#ifdef _INTERNAL
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif


extern HWND ApplicationHWnd;

//-------------------------------------------------------------------------------------------------
static void RTSFlagsToOSFlags(UnsignedInt buttonFlags, UnsignedInt otherFlags, UnsignedInt& outWindowsFlags)
{
	outWindowsFlags = 0;

	if (BitTest(buttonFlags, OSDBT_OK)) {
		outWindowsFlags |= MB_OK;
	}
	
	if (BitTest(buttonFlags, OSDBT_CANCEL)) {
		outWindowsFlags |= MB_OKCANCEL;
	}

	//-----------------------------------------------------------------------------------------------
	if (BitTest(otherFlags, OSDOF_SYSTEMMODAL)) {
		outWindowsFlags |= MB_SYSTEMMODAL;
	}

	if (BitTest(otherFlags, OSDOF_APPLICATIONMODAL)) {
		outWindowsFlags |= MB_APPLMODAL;
	}

	if (BitTest(otherFlags, OSDOF_TASKMODAL)) {
		outWindowsFlags |= MB_TASKMODAL;
	}

	if (BitTest(otherFlags, OSDOF_EXCLAMATIONICON)) {
		outWindowsFlags |= MB_ICONEXCLAMATION;
	}

	if (BitTest(otherFlags, OSDOF_INFORMATIONICON)) {
		outWindowsFlags |= MB_ICONINFORMATION;
	}

	if (BitTest(otherFlags, OSDOF_ERRORICON)) {
		outWindowsFlags |= MB_ICONERROR;
	}

	if (BitTest(otherFlags, OSDOF_STOPICON)) {
		outWindowsFlags |= MB_ICONSTOP;
	}

}

//-------------------------------------------------------------------------------------------------
OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m, UnsignedInt buttonFlags, UnsignedInt otherFlags)
{
	if (!TheGameText) {
		return OSDBT_ERROR;
	}

	UnicodeString promptStr = TheGameText->fetch(p);
	UnicodeString mesgStr = TheGameText->fetch(m);

	UnsignedInt windowsOptionsFlags = 0;
	RTSFlagsToOSFlags(buttonFlags, otherFlags, windowsOptionsFlags);
	
	// @todo Make this return more than just ok/cancel - jkmcd
	// (we need a function to translate back the other way.)
	Int returnResult = 0;
	if (TheSystemIsUnicode) 
	{
		returnResult = ::MessageBoxW(NULL, mesgStr.str(), promptStr.str(), windowsOptionsFlags);
	} 
	else 
	{
		// However, if we're using the default version of the message box, we need to 
		// translate the string into an AsciiString
		AsciiString promptA, mesgA;
		promptA.translate(promptStr);
		mesgA.translate(mesgStr);
		//Make sure main window is not TOP_MOST
		::SetWindowPos(ApplicationHWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		returnResult = ::MessageBoxA(NULL, mesgA.str(), promptA.str(), windowsOptionsFlags);
	}

	if (returnResult == IDOK) {
		return OSDBT_OK;
	}

	return OSDBT_CANCEL;
}

//-------------------------------------------------------------------------------------------------
static UINT GetWindowDpi(HWND hWnd)
{
	UINT dpi = 96;
	if (hWnd)
	{
		typedef UINT(WINAPI *GetDpiForWindow_t)(HWND);
		GetDpiForWindow_t pGetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "GetDpiForWindow");
		if (pGetDpiForWindow)
		{
			dpi = pGetDpiForWindow(hWnd);
		}
		else
		{
			HDC hdc = GetDC(hWnd);
			if (hdc)
			{
				dpi = GetDeviceCaps(hdc, LOGPIXELSX);
				ReleaseDC(hWnd, hdc);
			}
		}
	}
	return dpi;
}

//-------------------------------------------------------------------------------------------------
static void ScaleRectForDpi(RECT* rect, UINT dpi)
{
	if (!rect || dpi == 0 || dpi == 96)
		return;

	rect->left = MulDiv(rect->left, dpi, 96);
	rect->top = MulDiv(rect->top, dpi, 96);
	rect->right = MulDiv(rect->right, dpi, 96);
	rect->bottom = MulDiv(rect->bottom, dpi, 96);
}

//-------------------------------------------------------------------------------------------------
static void UnscaleRectForDpi(RECT* rect, UINT dpi)
{
	if (!rect || dpi == 0 || dpi == 96)
		return;

	rect->left = MulDiv(rect->left, 96, dpi);
	rect->top = MulDiv(rect->top, 96, dpi);
	rect->right = MulDiv(rect->right, 96, dpi);
	rect->bottom = MulDiv(rect->bottom, 96, dpi);
}

//-------------------------------------------------------------------------------------------------
void HandleDpiChanged(HWND hWnd, UINT newDpi, RECT* suggestedRect)
{
	if (!hWnd || !suggestedRect)
		return;

	SetWindowPos(hWnd, NULL, suggestedRect->left, suggestedRect->top,
		suggestedRect->right - suggestedRect->left,
		suggestedRect->bottom - suggestedRect->top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

//-------------------------------------------------------------------------------------------------
UINT GetDpiForApplicationWindow(HWND hWnd)
{
	return GetWindowDpi(hWnd);
}

//-------------------------------------------------------------------------------------------------
void ScaleWindowRect(RECT* rect, HWND hWnd)
{
	if (!rect || !hWnd)
		return;

	UINT dpi = GetWindowDpi(hWnd);
	ScaleRectForDpi(rect, dpi);
}

//-------------------------------------------------------------------------------------------------
void UnscaleWindowRect(RECT* rect, HWND hWnd)
{
	if (!rect || !hWnd)
		return;

	UINT dpi = GetWindowDpi(hWnd);
	UnscaleRectForDpi(rect, dpi);
}
