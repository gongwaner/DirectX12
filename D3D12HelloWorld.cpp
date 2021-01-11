#include "D3D12HelloWorld.h"

D3D12HelloWorld::D3D12HelloWorld(int inWidth, int inHeight)
{

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

}

void D3D12HelloWorld::OnDestroy()
{

}

void D3D12HelloWorld::LoadPipeline()
{
	mRenderingDevice.CreateDevice();
	mCommandQueue.CreateCommandQueue(mRenderingDevice.GetDevice());
	mSwapchain.CreateSwapChain(mRenderingDevice.GetFactory(), mCommandQueue.GetCommandQueue());
	mSwapchain.CreateDescriptorHeap(mRenderingDevice.GetDevice());
	mSwapchain.CreateRenderTargetView(mRenderingDevice.GetDevice());
	mCommandQueue.CreateCommandAllocator(mRenderingDevice.GetDevice());
}

void D3D12HelloWorld::LoadAssets()
{
	CreateRootSignature(mRenderingDevice.GetDevice());
	CreatePipelineState(mRenderingDevice.GetDevice());
	mCommandQueue.CreateCommandList(mRenderingDevice.GetDevice());
	CreateVertexBuffer(mRenderingDevice.GetDevice());
	mCommandQueue.CreateFence(mRenderingDevice.GetDevice());
	//WaitForPreviousFrame();
}

bool D3D12HelloWorld::CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
{
	HRESULT hr;

	// Define the geometry for a triangle.
	Vertex triangle_vertices[] =
	{
		{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
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

bool D3D12HelloWorld::CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
{
	HRESULT hr;

	CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
	root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (FAILED(hr))
		return false;

	hr = inDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
	if (FAILED(hr))
		return false;

	return true;
}

bool D3D12HelloWorld::CreatePipelineState(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
{
	// Create the pipeline state, which includes compiling and loading shaders.
	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader;
	Microsoft::WRL::ComPtr<ID3DBlob> error_buffer; //a buffer holding the error data 

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compile_flags = 0;
#endif

	hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0, &vertex_shader, &error_buffer);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)error_buffer->GetBufferPointer());
		return false;
	}

	hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0, &pixel_shader, &error_buffer);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)error_buffer->GetBufferPointer());
		return false;
	}

	//define the vertex input layout
	D3D12_INPUT_ELEMENT_DESC input_element_descs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } //12 = 3 vertices * 4 bytes(1 float is 4 bytes)
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
	pso_desc.pRootSignature = mRootSignature.Get();
	pso_desc.VS = CD3DX12_SHADER_BYTECODE(vertex_shader.Get());
	pso_desc.PS = CD3DX12_SHADER_BYTECODE(pixel_shader.Get());
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	hr = inDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&mPipelineState));
	if (FAILED(hr))
		return false;

	return true;
}