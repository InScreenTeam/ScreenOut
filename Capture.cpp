#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <StrSafe.h>
#include <string>
#include <iostream>
#include <Windows.h>
#include <Windowsx.h>
#include "Capture.h"

using namespace std;

// We need to create a CompatibleDC with 'hdcScreen' because CreateCompatibleBitmap() only accepts this type of handle.
// bitsCount - bits for pixel in image
//it is necessary to call SetBitmapInfo()
Capture::Capture(DWORD width, DWORD height,  WORD colourBitCount)
{
	if ((height == 0 ) || (width == 0)  || (colourBitCount < MIN_BIT_COUNT))
	throw(invalid_argument("Invalid arguments"));

	this->height = height;
	this->width = width;
	colourBitCount > 24 ? this->colourBitCount = 32 : this->colourBitCount = 24;

	hdcScreen = GetDC(NULL);
	hdcCompatible = CreateCompatibleDC(hdcScreen);
	//SetMapMode(hdcScreen, MM_ANISOTROPIC);
	//ScaleViewportExtEx(hdcScreen, 1, 1, -1, 1, NULL);
	//SetViewportOrgEx(hdcScreen, 0, height, NULL);
	SetMapMode(hdcCompatible, MM_ANISOTROPIC);
	ScaleViewportExtEx(hdcCompatible, 1, 1, -1, 1, NULL);
	SetViewportOrgEx(hdcCompatible, 0, height, NULL);
	hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
	
	if (colourBitCount != 24) 
	{
		pBitmapInfo = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << colourBitCount));
		pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << colourBitCount);  
	}
	else
	{ 
		pBitmapInfo = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
		pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	}
	if (!pBitmapInfo)
	{
		//Error
		return;
	}
	cursorInfo.cbSize = sizeof(CURSORINFO);
	//SetEffects();
}
Capture::~Capture()
{
		DeleteObject(hbmScreen);
		DeleteDC(hdcScreen); 
		DeleteDC(hdcCompatible); 
}
//using bitmap information for definition pBitmapInfo
bool Capture::SetBitmapInfo()
{
	if (hbmScreen > 0)
	{
		BITMAP bitmap;
		GetObject(hbmScreen, sizeof(BITMAP), (LPVOID)&bitmap); 
		SetBitmapInfo(bitmap.bmWidth, bitmap.bmHeight, bitmap.bmPlanes,
				  bitmap.bmBitsPixel, BI_RGB, 0, bitmap.bmWidthBytes); 
	}
	else
	{
		TakePic(height, width, NULL);
		if (hbmScreen > 0)
			SetBitmapInfo();
		else
			return false;
	}
	return true;
}
void Capture::SetBitmapInfo(LONG width, LONG height, WORD planes,
				   WORD bitCount, DWORD compression, DWORD clrImportant, DWORD bitmapWidth)
{
	WORD cClrBits = (WORD)(planes * bitCount); 
	if (cClrBits == 1) 
		cClrBits = 1; 
	else if (cClrBits <= 4) 
		cClrBits = 4; 
	else if (cClrBits <= 8) 
		cClrBits = 8; 
	else if (cClrBits <= 16) 
		cClrBits = 16; 
	else if (cClrBits <= 24) 
		cClrBits = 24; 
	else cClrBits = 32; 
	if (colourBitCount != cClrBits)
		throw(invalid_argument("invalid colour's bit count"));

	pBitmapInfo->bmiHeader.biWidth = width; 
	pBitmapInfo->bmiHeader.biHeight = height; 
	pBitmapInfo->bmiHeader.biPlanes = planes; 
	pBitmapInfo->bmiHeader.biBitCount = bitCount;
	pBitmapInfo->bmiHeader.biCompression = compression; 
	pBitmapInfo->bmiHeader.biClrImportant = clrImportant;
	pBitmapInfo->bmiHeader.biSizeImage = (pBitmapInfo->bmiHeader.biWidth + 7) /8 * pBitmapInfo->bmiHeader.biHeight * cClrBits; 
	pBitmapInfo->bmiHeader.biClrUsed = (1 << cClrBits);
	this->bitmapWidth = bitmapWidth;
}

void Capture::TakePic(int top, int left, int bottom, int right, LPVOID buffer, bool cursorDraw)
{
	SelectObject(hdcCompatible, hbmScreen); 
	BitBlt(	hdcCompatible,	
		left,			// x-coordinate of destination rectangle's upper-left corner
		top,			// y-coordinate of destination rectangle's upper-left corner
		right-left,		// width of destination rectangle 
		bottom-left,	// height of destination rectangle 
		hdcScreen,		// handle to source device context 
		left,			// x-coordinate of source rectangle's upper-left corner  
		top,			// y-coordinate of source rectangle's upper-left corner
		SRCCOPY);		// raster operation code
	if (cursorDraw)
		CurosorDraw(hdcCompatible, bottom);
	GetDIBits(hdcScreen, hbmScreen, 0, pBitmapInfo->bmiHeader.biHeight,	buffer, pBitmapInfo, DIB_RGB_COLORS);
}

void Capture::TakePic( int bottom, int right, LPVOID buffer )
{
	SelectObject(hdcCompatible, hbmScreen); 
	BitBlt(hdcCompatible, 0, 0, right, bottom, hdcScreen, 0, 0, SRCCOPY);
	CurosorDraw(hdcCompatible, bottom);
	GetDIBits(hdcScreen, hbmScreen, 0, pBitmapInfo->bmiHeader.biHeight,	buffer, pBitmapInfo, DIB_RGB_COLORS);
}

HCURSOR Capture::GetCurrentCursorHandle()
{
	GetCursorPos(&point);
	HWND hWnd = WindowFromPoint(point);
	if (GetClassWord(hWnd, GCW_ATOM) == CONSOLE_WINDOW_CLASS)
		return LoadCursorW(NULL, IDC_ARROW);
			
	DWORD dwThreadID, dwCurrentThreadID;

	dwThreadID = GetWindowThreadProcessId(hWnd, NULL);
	dwCurrentThreadID = GetCurrentThreadId();

	if (dwCurrentThreadID != dwThreadID)
	{
		if (AttachThreadInput(dwCurrentThreadID, dwThreadID, TRUE))
		{
			return GetCursor();
			AttachThreadInput(dwCurrentThreadID, dwThreadID, FALSE);
		}
	} 
	else
		return GetCursor();

	return NULL;
}

void Capture::CurosorDraw(HDC hDC, int bottom)
{
	ICONINFO ii;
	
	HCURSOR cursor =  GetCurrentCursorHandle();
	GetIconInfo(cursor, &ii);
	DrawIcon(hDC,point.x-ii.xHotspot, point.y-ii.yHotspot, cursor);
}

void Capture::WriteBMP(LPWSTR filename, LPVOID buffer )
{
	HANDLE fileHandle;
	BITMAPFILEHEADER bitmapFileHeader;
	PBITMAPINFOHEADER pBitmapInfoHeader;
	DWORD dwTotal; // total count of bytes 
	DWORD cb; // incremental count of bytes 
	BYTE *hp; // byte pointer 
	DWORD dwTmp; 

	pBitmapInfoHeader = (PBITMAPINFOHEADER) pBitmapInfo; 
	LPBYTE lpBits = (LPBYTE)buffer;

	// Create the .BMP file. 
	fileHandle = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, (DWORD) 0, 
							NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL); 
	if (fileHandle == INVALID_HANDLE_VALUE){
		MessageBox(NULL, TEXT("Could not create file for writing"), NULL, NULL);
		return;
	}
	bitmapFileHeader.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M" 
	// Compute the size of the entire file. 
	bitmapFileHeader.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + pBitmapInfoHeader->biSize + 
							  pBitmapInfoHeader->biClrUsed * sizeof(RGBQUAD) + pBitmapInfoHeader->biSizeImage); 
	bitmapFileHeader.bfReserved1 = 0; 
	bitmapFileHeader.bfReserved2 = 0; 

	// Compute the offset to the array of color indices. 
	bitmapFileHeader.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
		pBitmapInfoHeader->biSize + pBitmapInfoHeader->biClrUsed * sizeof(RGBQUAD); 

	// Copy the BITMAPFILEHEADER into the .BMP file. 
	if (!WriteFile(fileHandle, (LPVOID) &bitmapFileHeader, sizeof(BITMAPFILEHEADER), (LPDWORD) &dwTmp, NULL))
	{
		return;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
	if (!WriteFile(fileHandle, (LPVOID) pBitmapInfoHeader, sizeof(BITMAPINFOHEADER) 
		+ pBitmapInfoHeader->biClrUsed * sizeof (RGBQUAD), (LPDWORD) &dwTmp, ( NULL)))
	{
			return;
	}

	// Copy the array of color indices into the .BMP file. 
	dwTotal = cb = pBitmapInfoHeader->biSizeImage; 
	hp = lpBits; 
	if (!WriteFile(fileHandle, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL))
	{
		return;
	}
	
	if (!CloseHandle(fileHandle))
	{
		return;
	}

	GlobalFree((HGLOBAL)lpBits);
}

bool Capture::ScreenShot( LPWSTR fileName )
{
	RECT desktop;   
	HWND hDesktop = GetDesktopWindow();   
	GetWindowRect(hDesktop, &desktop);	   
	LPVOID buffer = GlobalAlloc(GMEM_FIXED, pBitmapInfo->bmiHeader.biSizeImage);
	ScaleViewportExtEx(hdcCompatible, 1, 1, 1, 1, NULL);
	SetViewportOrgEx(hdcCompatible, 0, 0, NULL);
	TakePic(desktop.right, desktop.bottom, buffer);
	WriteBMP(fileName, buffer);
	ScaleViewportExtEx(hdcCompatible, 1, 1, -1, 1, NULL);
	SetViewportOrgEx(hdcCompatible, 0, height, NULL);
	GlobalFree(buffer);
	return true;
}


//HHOOK Capture::mouseHook;
//HANDLE HThread[3] ;
//DWORD DwThreadId[3];
//
//DWORD WINAPI Capture::ThreadFunc(LPVOID lpParam)
//{
//
//	mouseHook = SetWindowsHookEx (WH_MOUSE_LL, MouseFilterFunc, GetModuleHandle(NULL), 0);
//	return 1;
//}
//
//bool Capture::SetEffects()
//{
//	CreateThread(NULL, 0,ThreadFunc,   0, 	1, &DwThreadId[0]);  
//	
//	return true;
//}
//
//
//LRESULT CALLBACK Capture::MouseFilterFunc( int nCode, WPARAM wParam, LPARAM lParam )
//{
//	switch (wParam)
//	{
//	cout<<"WORKs";
//	case WM_LBUTTONDOWN:
//	
//		int xPos, yPos;
//		xPos = GET_X_LPARAM(lParam); 
//		yPos = GET_Y_LPARAM(lParam); 
//		cout<<"WORKs";
//	break;
//	}
//	return CallNextHookEx(mouseHook, nCode, wParam, lParam);
//}
//
