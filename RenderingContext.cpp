#include "RenderingContext.h"

RenderingContext::~RenderingContext()
{
	/*if(mRootSignature)
		mRootSignature->Release();*/
}

bool RenderingContext::CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
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

void RenderingContext::SetViewport(float inTopLeftX, float inTopLeftY, float inWidth, float inHeight, float inMinDepth, float inMaxDepth)
{
	assert(inWidth > 0 && inHeight > 0);

	mViewport.TopLeftX = inTopLeftX;
	mViewport.TopLeftY = inTopLeftY;
	mViewport.Width = inWidth;
	mViewport.Height = inHeight;
	mViewport.MinDepth = inMinDepth;
	mViewport.MaxDepth = inMaxDepth;
}

void RenderingContext::SetScissorRect(long inLeft, long inTop, long inRight, long inBottom)
{
	assert(inRight > inLeft && inBottom > inTop);

	mScissorRect.left = inLeft;
	mScissorRect.top = inTop;
	mScissorRect.right = inRight;
	mScissorRect.bottom = inBottom;
}

void RenderingContext::FlushRenderState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> inCommandList)
{
	// Set necessary state.
	inCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	inCommandList->RSSetViewports(1, &mViewport);
	inCommandList->RSSetScissorRects(1, &mScissorRect);
}

void RenderingContext::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> inCommandList,
							D3D12_VERTEX_BUFFER_VIEW inVertexBufferView, UINT inVertexCountPerInstance,
							UINT inInstanceCount, UINT inStartVertexLocation, UINT inStartInstanceLocation)
{
	inCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	inCommandList->IASetVertexBuffers(0, 1, &inVertexBufferView);
	inCommandList->DrawInstanced(inVertexCountPerInstance, inInstanceCount, inStartVertexLocation, inStartInstanceLocation);
}
