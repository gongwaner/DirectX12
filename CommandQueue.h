#pragma once

#include "stdafx.h"
#include "RenderingContext.h"
#include "SwapChain.h"
#define FRAME_BUFFER_COUNT 2

class CommandQueue
{
public:
	CommandQueue();
	~CommandQueue();


	bool CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	bool CreateCommandAllocator(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	bool CreateCommandList(Microsoft::WRL::ComPtr<ID3D12Device> inDevice, int inFrameIndex);
	bool CreateFence(Microsoft::WRL::ComPtr<ID3D12Device> inDevice, int inFrameIndex);
	bool CreatePipelineState(Microsoft::WRL::ComPtr<ID3D12Device> inDevice, RenderingContext inRenderingContext);

	void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12Resource> inRenderTarget, int inFrameIndex, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> inRtvDescriptorHeap, int inRtvDescriptorSize, D3D12_VERTEX_BUFFER_VIEW inVertexBufferView, RenderingContext& inRenderingContext);
	void WaitForPreviousFrame(SwapChain inSwapchain);
	void WaitForGPU(SwapChain inSwapchain);
	void ExecuteCommandList();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return mCommandQueue; }
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState() const { return mPipelineState; }
	

private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;								// container for command lists
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocators[FRAME_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;							// a command list we can record commands into, then execute them to render the frame
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;

	HANDLE mFenceEvent;						// a handle to an event when our fence is unlocked by the gpu
	UINT64 mFenceValues[FRAME_BUFFER_COUNT];
};