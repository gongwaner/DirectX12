#pragma once

#include "stdafx.h"
#define FRAME_BUFFER_COUNT 2

class SwapChain
{
public:
	bool CreateSwapChain(Microsoft::WRL::ComPtr<IDXGIFactory4> inDxgiFactory, Microsoft::WRL::ComPtr<ID3D12CommandQueue> inCommandQueue);
	bool CreateRenderTargetView(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRtvDescriptorHeap() const { return mRtvDescriptorHeap; }
	int GetFrameIndex();
	int GetRtvDescriptorSize() const { return mRtvDescriptorSize; }



private:
	Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapchain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;							// a descriptor heap to hold resources like the render targets
	Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargets[FRAME_BUFFER_COUNT];				// number of render targets equal to buffer count
	
	int mFrameIndex;							// current rtv we are on
	int mRtvDescriptorSize;					// size of the rtv descriptor on the device (all front and back buffers will be the same size)
	int mFrameCount = FRAME_BUFFER_COUNT;
};