#pragma once

#include "stdafx.h"

class RenderingContext
{
public:
	RenderingContext() = default;
	~RenderingContext();

	bool CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> inDevice);

	void SetViewport(float inTopLeftX, float inTopLeftY, float inWidth, float inHeight, float inMinDepth = D3D12_MIN_DEPTH, float inMaxDepth = D3D12_MAX_DEPTH);
	void SetScissorRect(long inLeft, long inTop, long inRight, long inBottom);
	void FlushRenderState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> inCommandList);
	void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> inCommandList, D3D12_VERTEX_BUFFER_VIEW inVertexBufferView, UINT inVertexCountPerInstance, UINT inInstanceCount, UINT inStartVertexLocation, UINT inStartInstanceLocation);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return mRootSignature; }
	

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	CD3DX12_VIEWPORT mViewport;
	CD3DX12_RECT mScissorRect;
};
