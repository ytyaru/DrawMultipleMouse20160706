#pragma once
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <wchar.h>
#include <string>
#include <vector>
#include <gdiplus.h>

using std::wstring;
using namespace Gdiplus;

class Mouse
{
public:
	Mouse(void);
	~Mouse(void);
	
	void SetClient(int x, int y, int width, int height);
	void SetBrush(Color c);
	void Draw(Graphics* g);

	HANDLE DeviceHandle;
	RID_DEVICE_INFO DeviceInfo;
	std::basic_string<TCHAR> DeviceName;

	int ClientMouseX;
	int ClientMouseY;
	
	Gdiplus::Font* MyFont;
	Gdiplus::SolidBrush* BlackBrush;
	Gdiplus::StringFormat* MyFormat;
	SolidBrush* Brush;
};
