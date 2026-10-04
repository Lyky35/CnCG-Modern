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

#include "PreRTS.h"

#include "GameNetwork/WOLBrowser/WebBrowserStub.h"

WebBrowserStub *TheWebBrowser = NULL;

WebBrowserStub::WebBrowserStub()
{
}

WebBrowserStub::~WebBrowserStub()
{
}

void WebBrowserStub::init()
{
}

void WebBrowserStub::shutdown()
{
}

bool WebBrowserStub::createBrowserWindow(char *tag, GameWindow *win)
{
	return false;
}

void WebBrowserStub::closeBrowserWindow(GameWindow *win)
{
}

WebBrowserURL *WebBrowserStub::makeNewURL(AsciiString tag)
{
	return NULL;
}

WebBrowserURL *WebBrowserStub::findURL(AsciiString tag)
{
	return NULL;
}
