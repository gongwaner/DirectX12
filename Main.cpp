#include "stdafx.h"


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

HWND hwnd = NULL;//handle to window
LPCTSTR gWindowName = L"DX12";
LPCTSTR gWindowTitle = L"DX12 window";
bool gIsFullscreen = false;



LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            if (MessageBox(0, L"Are you sure you want to exit?", L"Really?", MB_YESNO | MB_ICONQUESTION)
                == IDYES)
                DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}





bool InitializeWindow(HINSTANCE hInstance, int nShowCmd,int inWidth,int inHeight,bool inIsFullscreen)
{
    if (inIsFullscreen)
    {
        HMONITOR monitor_handle = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        GetMonitorInfo(monitor_handle, &monitor_info);

        inWidth = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
        inHeight = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
    }

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = gWindowName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    hwnd = CreateWindowEx(NULL, gWindowName, gWindowTitle,
        WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, CW_USEDEFAULT,
        inWidth, inHeight, NULL,NULL,hInstance,NULL);

    if (!hwnd)
    {
        MessageBox(NULL, L"Error creating window",L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (gIsFullscreen)
        SetWindowLong(hwnd, GWL_STYLE, 0);

    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);

    return true;
}

void MainLoop() {
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // run game code
        }
    }
}








int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    //enable dx12 debug layer
    {
        using Microsoft::WRL::ComPtr;
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
    }
    

    // create the window
    if (!InitializeWindow(hInstance, nShowCmd, WINDOW_WIDTH, WINDOW_HEIGHT, gIsFullscreen))
    {
        MessageBox(0, L"Window Initialization - Failed",L"Error", MB_OK);
        return 0;
    }

    // start the main loop
    MainLoop();

    return 0;
}