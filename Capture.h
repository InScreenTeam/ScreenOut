#ifndef CaptureH
#define CaptureH

#define RGB_PLANES 4
#define COLOUR_BIT_COUNT 32
#define MIN_BIT_COUNT 24
#define CONSOLE_WINDOW_CLASS 49841

using namespace std;

class Capture
{
public:
	DWORD bitmapWidth;

protected:
	static HHOOK mouseHook;
	HDC hdcScreen;
	HDC hdcCompatible; 
	CURSORINFO cursorInfo;
	POINT point;
	HBITMAP hbmScreen;
	PBITMAPINFO pBitmapInfo;
	DWORD height, width;
	WORD colourBitCount;
public:
	Capture(DWORD width, DWORD height, WORD colourBitCount);
	~Capture();

	void TakePic(int top, int left, int bottom, int right, LPVOID buffer, bool cursorDraw);
	void TakePic(int bottom, int right, LPVOID buffer);
	void WriteBMP(LPWSTR filename, LPVOID buffer);
	bool ScreenShot(LPWSTR fileName);
	bool SetBitmapInfo();	
	bool SetEffects();
	static LRESULT CALLBACK MouseFilterFunc(int nCode, WPARAM wParam, LPARAM lParam);
protected:
	void SetBitmapInfo(LONG width, LONG height, WORD planes,
							  WORD bitCount, DWORD compression, DWORD clrImportant, DWORD bitmapWidth);
	inline HCURSOR Capture::GetCurrentCursorHandle();
	inline void CurosorDraw(HDC hDC, int bottom);
	static DWORD WINAPI ThreadFunc(LPVOID lpParam);
};
#endif