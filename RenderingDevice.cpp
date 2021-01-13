#include "RenderingDevice.h"
using Microsoft::WRL::ComPtr;


RenderingDevice::~RenderingDevice()
{
	/*if(mDevice)
		mDevice->Release();*/
}

bool RenderingDevice::CreateFactory()
{
	UINT dxgi_factory_flags = DXGI_CREATE_FACTORY_DEBUG;// Enable additional debug layers

	HRESULT hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&mDxgiFactory));
	if (FAILED(hr))
		return false;

	return true;
}

bool RenderingDevice::CreateDevice()
{
	CreateFactory();

	HRESULT hr;
	IDXGIAdapter1* adapter;
	int adapter_index = 0;//we'll start looking for dx12 compatible graphics devices starting at 0
	bool adapter_found = false;

	//find the first hardware gpu that supports dx12
	while (mDxgiFactory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			//we don't want a software device
			adapter_index++;
			continue;
		}

		//don't create the actual device yet
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapter_found = true;
			break;
		}

		adapter_index++;
	}

	if (!adapter_found)
		return false;

	// Create the device
	hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
	if (FAILED(hr))
		return false;

	return true;
}