#pragma once

#include "RenderingDevice.h"
#include "CommandQueue.h"
//#include "SwapChain.h"
//#include "RenderingContext.h"


class D3D12HelloWorld
{
public:
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
	RenderingContext mRenderingContext;

	int mWidth;
	int mHeight;

	void LoadPipeline();
	void LoadAssets();

	// App resources.
	Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	bool CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);



	
};