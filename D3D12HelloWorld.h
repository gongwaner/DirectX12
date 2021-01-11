#pragma once

#include "RenderingDevice.h"
#include "CommandQueue.h"
#include "SwapChain.h"


class D3D12HelloWorld
{
public :
	D3D12HelloWorld(int inWidth, int inHeight);


	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();

private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};


	RenderingDevice mRenderingDevice;
	CommandQueue mCommandQueue;
	SwapChain mSwapchain;

	void LoadPipeline();
	void LoadAssets();

	// App resources.
	Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	bool CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	bool CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
	bool CreatePipelineState(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);
};