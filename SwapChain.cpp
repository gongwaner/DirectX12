#include "SwapChain.h"
#include "Win32App.h"

using Microsoft::WRL::ComPtr;

SwapChain::~SwapChain()
{
    /*mSwapchain->Release();
    mRtvDescriptorHeap->Release();

    for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
    {
        mRenderTargets[i]->Release();
    }*/
}

bool SwapChain::CreateSwapChain(ComPtr<IDXGIFactory4> inDxgiFactory, ComPtr<ID3D12CommandQueue> inCommandQueue, int inWidth, int inHeight)
{
    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
    swapchain_desc.BufferCount = mFrameCount;
    swapchain_desc.Width = inWidth;
    swapchain_desc.Height = inHeight;
    swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchain_desc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> temp_swapchain;
    HRESULT hr = inDxgiFactory->CreateSwapChainForHwnd(
        inCommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32App::GetHwnd(),
        &swapchain_desc,
        nullptr, nullptr,
        &temp_swapchain);

    if (FAILED(hr))
        return false;

    temp_swapchain.As(&mSwapchain);
    mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();

    return true;
}

bool SwapChain::CreateDescriptorHeap(ComPtr<ID3D12Device> inDevice)
{
    //describe and create a render target view (RTV) descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
    rtv_heap_desc.NumDescriptors = mFrameCount;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = inDevice->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&mRtvDescriptorHeap));
    if (FAILED(hr))
        return false;

    //Once we create the RTV descriptor heap, we need to get the size of the RTV descriptor type size on the GPU. 
    //There is no guarentee that a descriptor type on one GPU is the same size as a descriptor on another GPU, 
    //which is why we need to ask the device for the size of a descriptor type size.
    //We need the size of the descriptor type so we can iterate over the descriptors in the descriptor heap.
    mRtvDescriptorSize = inDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    return true;
}

bool SwapChain::CreateRenderTargetView(ComPtr<ID3D12Device> inDevice)
{
    //create frame resources

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each frame.
    for (int n = 0; n < mFrameCount; n++)
    {
        HRESULT hr = mSwapchain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n]));
        if (FAILED(hr))
            return false;

        //We can loop through the RTV descriptors in the heap by offsetting the current handle we have by the
        //descriptor heap size we got from the GetDescriptorHandleIncrementSize() function.
        inDevice->CreateRenderTargetView(mRenderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, mRtvDescriptorSize);
    }

    return true;
}

void SwapChain::UpdateFrameIndex()
{
    mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();
}

bool SwapChain::Present(UINT inSyncInterval, UINT inFlags)
{
    HRESULT hr = mSwapchain->Present(inSyncInterval, inFlags);
    if (FAILED(hr))
        return false;

    return true;
}