#pragma once

#include "stdafx.h"
#define FRAME_BUFFER_COUNT 2

class CommandQueue
{
public:
	bool CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return mCommandQueue; }

	bool CreateCommandAllocator(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	bool CreateCommandList(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	bool CreateFence(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	

	void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12Resource> inRenderTarget, int inFrameIndex, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> inRtvDescriptorHeap, int inRtvDescriptorSize);
	void WaitForPreviousFrame();
	

private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;								// container for command lists
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;	
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;							// a command list we can record commands into, then execute them to render the frame
	Microsoft::WRL::ComPtr<ID3D12Fence> fence[FRAME_BUFFER_COUNT];							// an object that is locked while our command list is being executed by the gpu. We need as many 
																							//as we have allocators (more if we want to know when the gpu is finished with an asset)
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

	HANDLE mFenceEvent;						// a handle to an event when our fence is unlocked by the gpu
	UINT64 mFenceValue;
};