#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <StrSafe.h>
#include <string>
#include <iostream>
#include <Windows.h>
#include "Capture.h"

using namespace std;

// We need to create a CompatibleDC with 'hdcScreen' because CreateCompatibleBitmap() only accepts this type of handle.
// bitsCount - bits for pixel in image
Capture::Capture(DWORD height, DWORD width, WORD colourBitCount)
{
	if ((height == 0 ) || (width == 0)  || (colourBitCount < MIN_BIT_COUNT))
	throw(invalid_argument("Invalid arguments"));

	this->height = height;
	this->width = width;
	colourBitCount > 24 ? this->colourBitCount = 32 : this->colourBitCount = 24;

	hdcScreen = CreateDC(L"DISPLAY", // driver name 
						 NULL,	     // device name 
						 NULL,  	 // do not use
						 NULL);		 // printer data 
	hdcCompatible = CreateCompatibleDC(hdcScreen);
	
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
	
}
Capture::~Capture()
{
	if (hbmScreen > 0)
		DeleteObject(hbmScreen);
	if (hdcScreen>0) 
		DeleteDC(hdcScreen); 
	if (hdcCompatible > 0)
		DeleteDC(hdcCompatible); 
}

//using bitmap information for discrabe pBitmapInfo
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

void Capture::TakePic(int top, int left, int bottom, int right)
{
	hbmScreen =  CreateCompatibleBitmap(
		hdcScreen,	
		right-left, // width of bitmap, in pixels  
		bottom);	// height of bitmap, in pixels  
	SelectObject(hdcCompatible, hbmScreen); 

	BitBlt(	hdcCompatible,	
		left,			// x-coordinate of destination rectangle's upper-left corner
		top,			// y-coordinate of destination rectangle's upper-left corner
		right,			// width of destination rectangle 
		bottom,			// height of destination rectangle 
		hdcScreen,		// handle to source device context 
		left,			// x-coordinate of source rectangle's upper-left corner  
		top,			// y-coordinate of source rectangle's upper-left corner
		SRCCOPY 		// raster operation code 
		);
}


void Capture::WriteBMP(LPTSTR filename, HBITMAP hBitmap, HDC hDC)
{
	BITMAP bitmap; 
	HANDLE fileHandle;
	BITMAPFILEHEADER bitmapFileHeader;
	PBITMAPINFOHEADER pBitmapInfoHeader;
	LPBYTE lpBits; // memory pointer 
	DWORD dwTotal; // total count of bytes 
	DWORD cb; // incremental count of bytes 
	BYTE *hp; // byte pointer 
	DWORD dwTmp; 

	if (!GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bitmap)){
		//AfxMessageBox("Could not retrieve bitmap info");
		return;
	}
	
	pBitmapInfoHeader = (PBITMAPINFOHEADER) pBitmapInfo; 
	lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pBitmapInfoHeader->biSizeImage);

	if (!lpBits)
	{
		return;
	}

	// Retrieve the color table (RGBQUAD array) and the bits 
	if (!GetDIBits(hDC, hBitmap, 0, (WORD)pBitmapInfoHeader->biHeight, lpBits, pBitmapInfo, DIB_RGB_COLORS))
	{
			//AfxMessageBox("writeBMP::GetDIB error");
			return;
	}

	// Create the .BMP file. 
	fileHandle = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, (DWORD) 0, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
		(HANDLE) NULL); 
	if (fileHandle == INVALID_HANDLE_VALUE){
		//MessageBox(hWND,CHAR("Could not create file for writing"), NULL, NULL);
		return;
	}
	bitmapFileHeader.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M" 
	// Compute the size of the entire file. 
	bitmapFileHeader.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
		pBitmapInfoHeader->biSize + pBitmapInfoHeader->biClrUsed 
		* sizeof(RGBQUAD) + pBitmapInfoHeader->biSizeImage); 
	bitmapFileHeader.bfReserved1 = 0; 
	bitmapFileHeader.bfReserved2 = 0; 

	// Compute the offset to the array of color indices. 
	bitmapFileHeader.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
		pBitmapInfoHeader->biSize + pBitmapInfoHeader->biClrUsed 
		* sizeof (RGBQUAD); 

	// Copy the BITMAPFILEHEADER into the .BMP file. 
	if (!WriteFile(fileHandle, (LPVOID) &bitmapFileHeader, sizeof(BITMAPFILEHEADER), 
		(LPDWORD) &dwTmp, NULL)) {
			//AfxMessageBox("Could not write in to file");
			return;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
	if (!WriteFile(fileHandle, (LPVOID) pBitmapInfoHeader, sizeof(BITMAPINFOHEADER) 
		+ pBitmapInfoHeader->biClrUsed * sizeof (RGBQUAD), 
		(LPDWORD) &dwTmp, ( NULL))){
			//AfxMessageBox("Could not write in to file");
			return;
	}


	// Copy the array of color indices into the .BMP file. 
	dwTotal = cb = pBitmapInfoHeader->biSizeImage; 
	hp = lpBits; 
	if (!WriteFile(fileHandle, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)){
		// AfxMessageBox("Could not write in to file");
		return;
	}
	
	if (!CloseHandle(fileHandle)){
		//AfxMessageBox("Could not close file");
		return;
	}

	// Free memory. 
	GlobalFree((HGLOBAL)lpBits);
}

