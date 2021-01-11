#pragma once

#include "stdafx.h"
#define FRAME_BUFFER_COUNT 2

class CommandQueue
{
public:
	CommandQueue();

	bool CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	bool CreateCommandAllocator(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	bool CreateCommandList(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	bool CreateFence(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);

	void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12Resource> inRenderTarget, int inFrameIndex, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> inRtvDescriptorHeap, int inRtvDescriptorSize, D3D12_VERTEX_BUFFER_VIEW inVertexBufferView);
	void WaitForPreviousFrame(Microsoft::WRL::ComPtr<IDXGISwapChain3> inSwapchain, int inFrameIndex);
	void Draw(D3D12_VERTEX_BUFFER_VIEW inVertexBufferView, UINT inVertexCountPerInstance, UINT inInstanceCount, UINT inStartVertexLocation, UINT inStartInstanceLocation);

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return mCommandQueue; }


private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;								// container for command lists
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;							// a command list we can record commands into, then execute them to render the frame
	Microsoft::WRL::ComPtr<ID3D12Fence> fence[FRAME_BUFFER_COUNT];							// an object that is locked while our command list is being executed by the gpu. We need as many 
																							//as we have allocators (more if we want to know when the gpu is finished with an asset)
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

	HANDLE mFenceEvent;						// a handle to an event when our fence is unlocked by the gpu
	UINT64 mFenceValues[FRAME_BUFFER_COUNT];
};