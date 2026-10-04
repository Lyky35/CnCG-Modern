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

// FramGrab.cpp: implementation of the FrameGrabClass class.
//
//////////////////////////////////////////////////////////////////////

#include "framgrab.h"
#include <stdio.h>
#include <io.h>
//#include <errno.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FrameGrabClass::FrameGrabClass(const char *filename, MODE mode, int width, int height, int bitcount, float framerate)
{
	Mode = mode;
	Filename = filename;
	FrameRate = framerate;
	Counter = 0;

	Bitmap = 0;

	if(Mode != AVI) return;

	// Legacy vfw32 AVI capture removed - AVI mode is now a no-op stub.
	// Set up bitmap info header for raw frame mode compatibility.
	BitmapInfoHeader.biWidth = width;
	BitmapInfoHeader.biHeight = height;
	BitmapInfoHeader.biBitCount = (unsigned short)bitcount;
	BitmapInfoHeader.biSizeImage = ((((UINT)BitmapInfoHeader.biBitCount * BitmapInfoHeader.biWidth + 31) & ~31) / 8) * BitmapInfoHeader.biHeight;
	BitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfoHeader.biPlanes = 1;
	BitmapInfoHeader.biCompression = BI_RGB;
	BitmapInfoHeader.biXPelsPerMeter = 1;
	BitmapInfoHeader.biYPelsPerMeter = 1;
	BitmapInfoHeader.biClrUsed = 0;
	BitmapInfoHeader.biClrImportant = 0;

	Bitmap = (long *) GlobalAllocPtr(GMEM_MOVEABLE, BitmapInfoHeader.biSizeImage);
}

FrameGrabClass::~FrameGrabClass()
{
	if(Mode == AVI) {
		CleanupAVI();
	}
}

void FrameGrabClass::CleanupAVI() {
	if(Bitmap != 0) { GlobalFreePtr(Bitmap); Bitmap = 0; }
	Mode = RAW;
}

void FrameGrabClass::GrabAVI(void *BitmapPointer)
{
	// Legacy vfw32 AVI capture removed - no-op stub.
	(void)BitmapPointer;
}

void FrameGrabClass::GrabRawFrame(void * /*BitmapPointer*/)
{

}


void FrameGrabClass::ConvertGrab(void *BitmapPointer) 
{
	ConvertFrame(BitmapPointer);
	Grab( Bitmap );
}


void FrameGrabClass::Grab(void *BitmapPointer) 
{
	if(Mode == AVI) 
		GrabAVI(BitmapPointer);
	else
		GrabRawFrame(BitmapPointer);
}


void FrameGrabClass::ConvertFrame(void *BitmapPointer) 
{

	int width = BitmapInfoHeader.biWidth;
	int height = BitmapInfoHeader.biHeight;
	long *image = (long *) BitmapPointer;

	// copy the data, doing a vertical flip & byte re-ordering of the pixel longwords
	int y = height;
	while(y--) {
		int x = width;
		int yoffset = y * width;
		int yoffset2 = (height - y) * width;
		while(x--) {
			long *source = &image[yoffset + x];
			long *dest = &Bitmap[yoffset2 + x];
			*dest = *source;
			unsigned char *c = (unsigned char *) dest;
			c[3] = c[0];
			c[0] = c[2];
			c[2] = c[3];
			c[3] = 0;
		}
	}
}
