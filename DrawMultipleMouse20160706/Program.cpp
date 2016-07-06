#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <wchar.h>
#include <string>
#include <vector>
#include <gdiplus.h>
#include "Mouse.h"
using std::wstring;
using namespace Gdiplus;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR szClassName[] = _T("DrawMultipleMouse20160706");

HWND hWnd;

RAWINPUTDEVICE device;
std::vector<Mouse> MouseList;
std::vector<char> rawInputMessageData;

int ClientX;
int ClientY;
int ClientWidth;
int ClientHeight;

GdiplusStartupInput gdiSI;
ULONG_PTR gdiToken;

Bitmap* gOffScreenBitmap;
Graphics* gOffScreenGraphics;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS myProg;

	GdiplusStartup(&gdiToken, &gdiSI, NULL);

	if (!hPreInst) {
		myProg.style			= CS_HREDRAW | CS_VREDRAW;
		myProg.lpfnWndProc		= WndProc;
		myProg.cbClsExtra		= 0;
		myProg.cbWndExtra		= 0;
		myProg.hInstance		= hInstance;
		myProg.hIcon			= NULL;
		myProg.hCursor			= LoadCursor(NULL, IDC_ARROW);
		myProg.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		myProg.lpszMenuName		= NULL;
		myProg.lpszClassName	= szClassName;
		if (!RegisterClass(&myProg)) { return FALSE; }
	}
	hWnd = CreateWindow(szClassName,	// class名
		_T("複数マウスポインタの座標を描画する"),		// タイトル
		WS_OVERLAPPEDWINDOW,		// Style
		CW_USEDEFAULT,				// X
		CW_USEDEFAULT,				// Y
		CW_USEDEFAULT,				// Width
		CW_USEDEFAULT,				// Height
		NULL,						// 親ウィンドウまたはオーナーウィンドウのハンドル
		NULL,						// メニューハンドルまたは子ウィンドウ ID
		hInstance,					// アプリケーションインスタンスのハンドル
		NULL						// ウィンドウ作成データ
	);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiToken);

	return (msg.wParam);
}

void GetClientInfo()
{	
	RECT rectWindow;
	HWND hDeskWnd = GetDesktopWindow();
	GetWindowRect( hDeskWnd, &rectWindow );

	ClientX = rectWindow.left;
	ClientY = rectWindow.top;
	ClientWidth = rectWindow.right - rectWindow.left;
	ClientHeight = rectWindow.bottom - rectWindow.top;
}

void GetDevices()
{
	UINT numDevices;
	GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
	if(numDevices == 0) return;

	std::vector<RAWINPUTDEVICELIST> deviceList(numDevices);
	GetRawInputDeviceList(&deviceList[0], &numDevices, sizeof(RAWINPUTDEVICELIST));
	
	std::vector<TCHAR> deviceNameData;
	std::basic_string<TCHAR> deviceName;
	TCHAR dataStr[1024];
	for(UINT i = 0; i < numDevices; ++i)
	{
		const RAWINPUTDEVICELIST& device = deviceList[i];
		
		memset(dataStr, NULL, 1024);
		if(device.dwType == RIM_TYPEMOUSE) { _stprintf_s(dataStr, 1024, _T("type=Mouse:\n")); }
		else if(device.dwType == RIM_TYPEKEYBOARD) { _stprintf_s(dataStr, 1024, _T("type=Keyboard:\n")); }
		else if(device.dwType == RIM_TYPEHID) { _stprintf_s(dataStr, 1024, _T("type=HID:\n")); }
		else { _stprintf_s(dataStr, 1024, _T("type=?:\n")); }
		OutputDebugString(dataStr);
		
		memset(dataStr, NULL, 1024);
		_stprintf_s(dataStr, 1024, _T("	Handle=0x%08X\n"), device.hDevice);
		OutputDebugString(dataStr);

		UINT dataSize;
		GetRawInputDeviceInfo(device.hDevice, RIDI_DEVICENAME, nullptr, &dataSize);
		if(dataSize)
		{
			deviceNameData.resize(dataSize);
			UINT result = GetRawInputDeviceInfo(device.hDevice, RIDI_DEVICENAME, &deviceNameData[0], &dataSize);
			if(result != UINT_MAX)
			{
				deviceName.assign(deviceNameData.begin(), deviceNameData.end());
				memset(dataStr, NULL, 1024);
				_stprintf_s(dataStr, 1024, _T("	Name=%s\n"), deviceName.c_str());
				OutputDebugString(dataStr);
			}
		}

		RID_DEVICE_INFO deviceInfo;
		deviceInfo.cbSize = sizeof deviceInfo;
		dataSize = sizeof deviceInfo;
		UINT result = GetRawInputDeviceInfo(device.hDevice, RIDI_DEVICEINFO, &deviceInfo, &dataSize);
		if(result != UINT_MAX)
		{
			memset(dataStr, NULL, 1024);
			if(device.dwType == RIM_TYPEMOUSE)
			{
				_stprintf_s(dataStr, 1024, _T("	Id=%u, Buttons=%u, SampleRate=%u, HorizontalWheel=%s\n"), 
					deviceInfo.mouse.dwId,
					deviceInfo.mouse.dwNumberOfButtons,
					deviceInfo.mouse.dwSampleRate,
					deviceInfo.mouse.fHasHorizontalWheel ? L"1" : L"0");

				Mouse mouse;
				mouse.DeviceHandle = device.hDevice;
				mouse.DeviceInfo = deviceInfo;
				mouse.DeviceName = deviceName;
				mouse.SetBrush(Color::Red);
				MouseList.push_back(mouse);
			}
			else if(device.dwType == RIM_TYPEKEYBOARD)
			{
				_stprintf_s(dataStr, 1024, _T("	Type=%u, SubType=%u, KeyboardMode=%u, NumberOfFunctionKeys=%u, NumberOfIndicators=%u, NumberOfKeysTotal=%u\n"), 
					deviceInfo.keyboard.dwType,
					deviceInfo.keyboard.dwSubType,
					deviceInfo.keyboard.dwKeyboardMode,
					deviceInfo.keyboard.dwNumberOfFunctionKeys,
					deviceInfo.keyboard.dwNumberOfIndicators,
					deviceInfo.keyboard.dwNumberOfKeysTotal);
			}
			else if(device.dwType == RIM_TYPEHID)
			{
				_stprintf_s(dataStr, 1024, _T("	VendorId=%u, ProductId=%u, VersionNumber=%u, UsagePage=0x%X, Usage=0x%X, NumberOfKeysTotal=%u\n"), 
					deviceInfo.hid.dwVendorId,
					deviceInfo.hid.dwProductId,
					deviceInfo.hid.dwVersionNumber,
					deviceInfo.hid.usUsagePage,
					deviceInfo.hid.usUsage);
			}
			else {}
			OutputDebugString(dataStr);
		}
	}
}

void OnRawInput(bool inForeground, HRAWINPUT hRawInput)
{
	UINT dataSize;
	GetRawInputData(hRawInput, RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

	if(dataSize == 0) { return; }
	if(dataSize > rawInputMessageData.size()) {
		rawInputMessageData.resize(dataSize);
	}

	void* dataBuf = &rawInputMessageData[0];
	GetRawInputData(hRawInput, RID_INPUT, dataBuf, &dataSize, sizeof(RAWINPUTHEADER));

	const RAWINPUT *raw = (const RAWINPUT*)dataBuf;
	if (raw->header.dwType == RIM_TYPEMOUSE) {
		HANDLE deviceHandle = raw->header.hDevice;

		const RAWMOUSE& mouseData = raw->data.mouse;

		TCHAR dataStr[1024];
		memset(dataStr, NULL, 1024);

		for (unsigned int i = 0; i < MouseList.size(); i++) {
			if (deviceHandle == MouseList[i].DeviceHandle) {
				MouseList[i].ClientMouseX += mouseData.lLastX;
				MouseList[i].ClientMouseY += mouseData.lLastY;
				
				RECT rectWindow;
				HWND hDeskWnd = GetDesktopWindow();
				GetWindowRect( hDeskWnd, &rectWindow );

				if (MouseList[i].ClientMouseX < 0) { MouseList[i].ClientMouseX = 0; }
				else if (ClientX + ClientWidth < MouseList[i].ClientMouseX) { MouseList[i].ClientMouseX = ClientX + ClientWidth; }
				else {}

				if (MouseList[i].ClientMouseY < 0) { MouseList[i].ClientMouseY = 0; }
				else if (ClientY + ClientHeight < MouseList[i].ClientMouseY) { MouseList[i].ClientMouseY = ClientY + ClientHeight; }
				else {}
				
				memset(dataStr, NULL, 1024);
				_stprintf_s(dataStr, 1024, _T("Device=0x%08X, X=%d, Y=%d\n"), deviceHandle, MouseList[i].ClientMouseX, MouseList[i].ClientMouseY);
				OutputDebugString(dataStr);
				
				InvalidateRect(hWnd, 0, false);
			}
		}
	}
}

void OnPaint(HDC hdc)
{
	Graphics g(hdc);
	
	gOffScreenGraphics->Clear(Color::White);
	for (unsigned int i = 0; i < MouseList.size(); i++) {
		MouseList[i].Draw(gOffScreenGraphics);
	}
	g.DrawImage(gOffScreenBitmap, 0, 0);
	
    return;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	switch (msg) {
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			OnPaint(hdc);
			EndPaint(hWnd, &ps);
			break;
		case WM_INPUT:
			OnRawInput(
				GET_RAWINPUT_CODE_WPARAM(wParam) == RIM_INPUT,
				(HRAWINPUT)lParam
			);
			return(DefWindowProc(hWnd, msg, wParam, lParam));
		case WM_CREATE:
			GetClientInfo();
			GetDevices();
			device.usUsagePage = 0x01;
			device.usUsage = 0x02;
			device.dwFlags = 0;
			device.hwndTarget = 0;
			RegisterRawInputDevices(&device, 1, sizeof device);
			
			gOffScreenBitmap = new Bitmap(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
			gOffScreenGraphics = Graphics::FromImage(gOffScreenBitmap);
			break;
		case WM_DESTROY:
			device.usUsagePage = 0x01;
			device.usUsage = 0x02;
			device.dwFlags = RIDEV_REMOVE;
			device.hwndTarget = 0;
			RegisterRawInputDevices(&device, 1, sizeof device);
			PostQuitMessage(0);
			break;
		default:
			return(DefWindowProc(hWnd, msg, wParam, lParam));
	}
	return (0L);
}
