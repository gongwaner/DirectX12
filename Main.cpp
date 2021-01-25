#include "stdafx.h"
#include "Win32App.h"
#include "D3D12HelloWorld.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

void EnableDebugLayer()
{
    //enable dx12 debug layer
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }
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
    EnableDebugLayer();
  
    // create the window
    if (!Win32App::InitializeWindow(hInstance, nShowCmd, WINDOW_WIDTH, WINDOW_HEIGHT, false))
    {
        MessageBox(0, L"Window Initialization - Failed",L"Error", MB_OK);
        return 0;
    }

    D3D12HelloWorld sample(WINDOW_WIDTH, WINDOW_HEIGHT);

    sample.OnInit();

    // start the main loop
    //MainLoop();
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
			sample.OnUpdate();
			sample.OnRender();
		}
	}

	sample.OnDestroy();

    return 0;
}