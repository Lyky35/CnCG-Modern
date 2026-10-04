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

// FILE: W3DGameEngine.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2001
// Description:
//   Implementation of the Win32 game engine, this is the highest level of 
//   the game application, it creates all the devices we will use for the game
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <shellscalingapi.h>
#include "Win32Device/Common/Win32GameEngine.h"
#include "Common/PerfTimer.h"

#include "GameNetwork/LANAPICallbacks.h"

extern DWORD TheMessageTime;
extern HWND ApplicationHWnd;
extern Bool ApplicationIsWindowed;

//-------------------------------------------------------------------------------------------------
/** Constructor for Win32GameEngine */
//-------------------------------------------------------------------------------------------------
Win32GameEngine::Win32GameEngine()
{
	// Stop blue screen
	m_previousErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );
	m_borderlessFullscreen = false;
	m_isFullscreen = false;
	m_windowedStyle = 0;
	m_windowedExStyle = 0;
	m_windowedRect = {};
	m_windowedPlacement = {};
}

//-------------------------------------------------------------------------------------------------
/** Destructor for Win32GameEngine */
//-------------------------------------------------------------------------------------------------
Win32GameEngine::~Win32GameEngine()
{
	// restore it (this isn't really necessary, but feels good.)
	SetErrorMode( m_previousErrorMode );
}


//-------------------------------------------------------------------------------------------------
/** Initialize the game engine */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::init( void )
{

	// extending functionality
	GameEngine::init();

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset the system */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::reset( void )
{

	// extending functionality
	GameEngine::reset();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update the game engine by updating the GameClient and 
	* GameLogic singletons. */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::update( void )
{


	// call the engine normal update
	GameEngine::update();

	extern HWND ApplicationHWnd;
	if (ApplicationHWnd && ::IsIconic(ApplicationHWnd)) {
		while (ApplicationHWnd && ::IsIconic(ApplicationHWnd)) {
			// We are alt-tabbed out here.  Sleep a bit, & process windows
			// so that we can become un-alt-tabbed out.
			Sleep(5);
			serviceWindowsOS();

			if (TheLAN != NULL) {
				// BGC - need to update TheLAN so we can process and respond to other
				// people's messages who may not be alt-tabbed out like we are.
				TheLAN->setIsActive(isActive());
				TheLAN->update();
			}

			// If we are running a multiplayer game, keep running the logic.
			// There is code in the client to skip client redraw if we are 
			// iconic.  jba.
			if (TheGameEngine->getQuitting() || TheGameLogic->isInInternetGame() || TheGameLogic->isInLanGame()) {
				break; // keep running.
			}
		}
	}

	// allow windows to perform regular windows maintenance stuff like msgs
	serviceWindowsOS();

}  // end update

//-------------------------------------------------------------------------------------------------
/** This function may be called from within this application to let
  * Microsoft Windows do its message processing and dispatching.  Presumeably
	* we would call this at least once each time around the game loop to keep
  * Windows services from backing up */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::serviceWindowsOS( void )
{
	MSG msg;
  Int returnValue;

	//
	// see if we have any messages to process, a NULL window handle tells the
	// OS to look at the main window associated with the calling thread, us!
	//
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{

		// get the message
		returnValue = GetMessage( &msg, NULL, 0, 0 );

		// this is one possible way to check for quitting conditions as a message
		// of WM_QUIT will cause GetMessage() to return 0
/*
		if( returnValue == 0 )
		{

			setQuitting( true );
			break;

		}
*/

		TheMessageTime = msg.time;
		// translate and dispatch the message
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		TheMessageTime = 0;

	}  // end while

}  // end ServiceWindowsOS

//-------------------------------------------------------------------------------------------------
/** Toggle between fullscreen and windowed mode */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::toggleFullscreen( void )
{
	if (!ApplicationHWnd)
		return;

	if (m_isFullscreen)
	{
		// Restore windowed mode
		SetWindowLong(ApplicationHWnd, GWL_STYLE, m_windowedStyle);
		SetWindowLong(ApplicationHWnd, GWL_EXSTYLE, m_windowedExStyle);
		SetWindowPlacement(ApplicationHWnd, &m_windowedPlacement);
		SetWindowPos(ApplicationHWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		m_isFullscreen = false;
		ApplicationIsWindowed = true;
	}
	else
	{
		// Save current windowed state
		m_windowedStyle = GetWindowLong(ApplicationHWnd, GWL_STYLE);
		m_windowedExStyle = GetWindowLong(ApplicationHWnd, GWL_EXSTYLE);
		m_windowedPlacement.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(ApplicationHWnd, &m_windowedPlacement);
		GetWindowRect(ApplicationHWnd, &m_windowedRect);

		if (m_borderlessFullscreen)
		{
			// Borderless fullscreen
			SetWindowLong(ApplicationHWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			SetWindowLong(ApplicationHWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

			HMONITOR hMonitor = MonitorFromWindow(ApplicationHWnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
			if (GetMonitorInfo(hMonitor, &monitorInfo))
			{
				SetWindowPos(ApplicationHWnd, HWND_TOP,
					monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
					monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE);
			}
		}
		else
		{
			// Exclusive fullscreen
			SetWindowLong(ApplicationHWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			SetWindowLong(ApplicationHWnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_APPWINDOW);

			HMONITOR hMonitor = MonitorFromWindow(ApplicationHWnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
			if (GetMonitorInfo(hMonitor, &monitorInfo))
			{
				SetWindowPos(ApplicationHWnd, HWND_TOPMOST,
					monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
					monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE);
			}
		}
		m_isFullscreen = true;
		ApplicationIsWindowed = false;
	}
}

//-------------------------------------------------------------------------------------------------
/** Enable/disable borderless fullscreen mode */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::setBorderlessFullscreen( Bool enable )
{
	m_borderlessFullscreen = enable;
}

//-------------------------------------------------------------------------------------------------
/** Handle display resolution/monitor changes */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::handleDisplayChange( void )
{
	if (!ApplicationHWnd)
		return;

	// If in fullscreen, re-adjust to new monitor size
	if (m_isFullscreen)
	{
		HMONITOR hMonitor = MonitorFromWindow(ApplicationHWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
		if (GetMonitorInfo(hMonitor, &monitorInfo))
		{
			SetWindowPos(ApplicationHWnd, m_borderlessFullscreen ? HWND_TOP : HWND_TOPMOST,
				monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);
		}
	}

}

//-------------------------------------------------------------------------------------------------
/** Handle window move */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::handleWindowMove( void )
{
	if (!ApplicationHWnd)
		return;

	// Update windowed rect for restoration
	if (!m_isFullscreen)
	{
		GetWindowRect(ApplicationHWnd, &m_windowedRect);
	}
}

//-------------------------------------------------------------------------------------------------
/** Handle window resize */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::handleWindowResize( void )
{
	if (!ApplicationHWnd)
		return;

	// Update windowed rect for restoration
	if (!m_isFullscreen)
	{
		GetWindowRect(ApplicationHWnd, &m_windowedRect);
	}
}

