#pragma once
#include "stdafx.h"

class Win32App
{
public:
	static HWND GetHwnd() { return mHwnd; }
	static bool InitializeWindow(HINSTANCE hInstance, int nShowCmd, int inWidth, int inHeight, bool inIsFullscreen);

private:
	static HWND mHwnd;//handle to window
};
