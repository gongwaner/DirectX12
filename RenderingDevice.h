#pragma once
#include "stdafx.h"

class RenderingDevice
{
public:
	RenderingDevice() = default;
	~RenderingDevice();

	bool CreateFactory();
	bool CreateDevice();
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return mDevice; }
	Microsoft::WRL::ComPtr<IDXGIFactory4> GetFactory() const { return mDxgiFactory; }

private:
	Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
	Microsoft::WRL::ComPtr<IDXGIFactory4> mDxgiFactory;
};

extern RenderingDevice gRenderingDevice;


