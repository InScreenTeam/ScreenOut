#ifndef CaptureH
#define CaptureH

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <StrSafe.h>
#include <string>
#include <iostream>


#define RGB_PLANES 4
#define COLOUR_BIT_COUNT 32
#define MIN_BIT_COUNT 24

using namespace std;

class Capture
{
public:
	HBITMAP hbmScreen;
	HDC hdcScreen;
	PBITMAPINFO pBitmapInfo;
	DWORD bitmapWidth;

protected:
	HDC hdcCompatible; 
	DWORD height, width;
	WORD colourBitCount;
public:
	Capture(DWORD width, DWORD height, WORD colourBitCount);
	~Capture();

	void TakePic(int top, int left, int bottom, int right);
	void WriteBMP(LPTSTR filename, HBITMAP bitmap, HDC hDC);
	bool SetBitmapInfo();
	
protected:
	void SetBitmapInfo(LONG width, LONG height, WORD planes,
							  WORD bitCount, DWORD compression, DWORD clrImportant, DWORD bitmapWidth);
};
#endif