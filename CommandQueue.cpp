#include "CommandQueue.h"

CommandQueue::CommandQueue()
{
    for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
    {
        mFenceValues[i] = 0;
    }
}

bool CommandQueue::CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
{
    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HRESULT hr = inDevice->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&mCommandQueue));
    if (FAILED(hr))
        return false;

    return true;
}

bool CommandQueue::CreateCommandAllocator(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
{
    HRESULT hr = inDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator));
    if (FAILED(hr))
        return false;

    return true;
}

bool CommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
{
    HRESULT hr = inDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), NULL, IID_PPV_ARGS(&mCommandList));
    if (FAILED(hr))
        return false;

    //command lists are created in the recording state, but there is nothing to record yet. 
    //the main loop  will set it up for recording again so close it now
    mCommandList->Close();

    return true;
}

void CommandQueue::PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12Resource> inRenderTarget, int inFrameIndex,
                                       Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> inRtvDescriptorHeap, 
                                       int inRtvDescriptorSize, D3D12_VERTEX_BUFFER_VIEW inVertexBufferView)
{
    //command list allocators can only be reset when the associated command lists have finished execution on the GPU
    mCommandAllocator->Reset();

    //however, when ExecuteCommandList() is called on a particular command list, 
    //that command list can then be reset at any time and must be before re-recording
    mCommandList->Reset(mCommandAllocator.Get(), NULL);

    //here we start recording commands into the commandlist

    //transition the frame index render target from PRESENT to RENDER TARGGET state 
    //so the command list draws to it
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(inRenderTarget.Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    //we again get the handle to our current render target view so we can set it as the render target in the output
    //merger stage of the pipeline
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(inRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), inFrameIndex, inRtvDescriptorSize);

    //set and clear the render target for the output merge stage
    mCommandList->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);

    const float clear_color[] = { 0.0f, 0.2f, 0.5f, 1.0f };
    mCommandList->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
    Draw(inVertexBufferView,3,1,0,0);

    //transition the frame index render target from RENDER TARGET to PRESENT state
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(inRenderTarget.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    mCommandList->Close();
}



bool CommandQueue::CreateFence(Microsoft::WRL::ComPtr<ID3D12Device> inDevice)
{
    //create synchronization objects and wait until assets have been uploaded to the GPU
    HRESULT hr = inDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
    if (FAILED(hr))
        return false;

    //create an event handle to use for frame synchronization.
    mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (mFenceEvent == nullptr)
    {
        HRESULT_FROM_WIN32(GetLastError());
        return false;
    }

    return true;
}


void CommandQueue::Draw(D3D12_VERTEX_BUFFER_VIEW inVertexBufferView, UINT inVertexCountPerInstance, 
                        UINT inInstanceCount, UINT inStartVertexLocation, UINT inStartInstanceLocation)
{
    mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->IASetVertexBuffers(0, 1, &inVertexBufferView);
    mCommandList->DrawInstanced(inVertexCountPerInstance, inInstanceCount, inStartVertexLocation, inStartInstanceLocation);
}

void CommandQueue::WaitForPreviousFrame(Microsoft::WRL::ComPtr<IDXGISwapChain3> inSwapchain, int inFrameIndex)
{
    HRESULT hr;

    //update fence to a specific value
    const UINT64 current_fence_value = mFenceValues[inFrameIndex];
    hr = mCommandQueue->Signal(mFence.Get(), current_fence_value);

    //update the frame index
    int frame_index = inSwapchain->GetCurrentBackBufferIndex();

    // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
    // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
    if (mFence->GetCompletedValue() < mFenceValues[frame_index])
    {
        //create an event which is signaled once the fence's current value is "fenceValue"
        hr = mFence->SetEventOnCompletion(mFenceValues[frame_index], mFenceEvent);
        if (FAILED(hr))
        {
            assert("mFence->SetEventOnCompletion() failed");
        }
        //We will wait until the fence has triggered the event that it's current value has reached "fenceValue"
        //once its value has reached "fenceValue", we know the command queue has finished executing
        WaitForSingleObjectEx(mFenceEvent, INFINITE, false);
    }

    //increment fenceValue for next frame
    mFenceValues[frame_index]++;
}