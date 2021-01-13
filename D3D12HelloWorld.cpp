#include "D3D12HelloWorld.h"

D3D12HelloWorld::D3D12HelloWorld(int inWidth, int inHeight)
{
	mWidth = inWidth;
	mHeight = inHeight;
	mRenderingContext.SetViewport(0.0f, 0.0f, (float)inWidth, (float)inHeight);
	mRenderingContext.SetScissorRect(0, 0, static_cast<LONG>(inWidth), static_cast<LONG>(inHeight));
}


void D3D12HelloWorld::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void D3D12HelloWorld::OnUpdate()
{

}

void D3D12HelloWorld::OnRender()
{
	//record all the commands we need to render the scene into the command list
	mCommandQueue.PopulateCommandList(mSwapchain.GetRenderTarget(), mSwapchain.GetFrameIndex(), mSwapchain.GetRtvDescriptorHeap(), mSwapchain.GetRtvDescriptorSize(), mVertexBufferView, mRenderingContext);

	//execute the command list
	mCommandQueue.ExecuteCommandList();

	//present the frame
	mSwapchain.Present(1, 0);

	mCommandQueue.WaitForPreviousFrame(mSwapchain);
}

void D3D12HelloWorld::OnDestroy()
{
	//ensure that the GPU is no longer referencing resources that are about to be cleaned up by the destructor.
	mCommandQueue.WaitForGPU(mSwapchain);
}

void D3D12HelloWorld::LoadPipeline()
{
	mRenderingDevice.CreateDevice();
	mCommandQueue.CreateCommandQueue(mRenderingDevice.GetDevice());
	mSwapchain.CreateSwapChain(mRenderingDevice.GetFactory(), mCommandQueue.GetCommandQueue(), mWidth, mHeight);
	mSwapchain.CreateDescriptorHeap(mRenderingDevice.GetDevice());
	mSwapchain.CreateRenderTargetView(mRenderingDevice.GetDevice());
	mCommandQueue.CreateCommandAllocator(mRenderingDevice.GetDevice());
}

void D3D12HelloWorld::LoadAssets()
{
	mRenderingContext.CreateRootSignature(mRenderingDevice.GetDevice());
	mCommandQueue.CreatePipelineState(mRenderingDevice.GetDevice(), mRenderingContext);
	mCommandQueue.CreateCommandList(mRenderingDevice.GetDevice());
	CreateVertexBuffer(mRenderingDevice.GetDevice());
	mCommandQueue.CreateFence(mRenderingDevice.GetDevice());
	mCommandQueue.WaitForGPU(mSwapchain);
}

bool D3D12HelloWorld::CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
{
	HRESULT hr;


	float m_aspectRatio = (float)mWidth / (float)mHeight;
	// Define the geometry for a triangle.
	Vertex triangle_vertices[] =
	{
		{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const UINT vertex_buffer_size = sizeof(triangle_vertices);

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	hr = inDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mVertexBuffer));
	if (FAILED(hr))
		return false;

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	hr = mVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	if (FAILED(hr))
		return false;

	memcpy(pVertexDataBegin, triangle_vertices, sizeof(triangle_vertices));
	mVertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(Vertex);
	mVertexBufferView.SizeInBytes = vertex_buffer_size;

	return true;
}