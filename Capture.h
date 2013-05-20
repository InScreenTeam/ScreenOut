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
	DWORD bitmapWidth;

protected:
	HDC hdcScreen;
	HDC hdcCompatible; 
	HBITMAP hbmScreen;
	PBITMAPINFO pBitmapInfo;
	DWORD height, width;
	WORD colourBitCount;
public:
	Capture(DWORD width, DWORD height, WORD colourBitCount);
	~Capture();

	void TakePic(int top, int left, int bottom, int right, LPVOID buffer);
	void TakePic(int bottom, int right, LPVOID buffer);
	void WriteBMP(LPTSTR filename, HBITMAP bitmap, HDC hDC);
	bool SetBitmapInfo();
	
protected:
	void SetBitmapInfo(LONG width, LONG height, WORD planes,
							  WORD bitCount, DWORD compression, DWORD clrImportant, DWORD bitmapWidth);
};
#endif