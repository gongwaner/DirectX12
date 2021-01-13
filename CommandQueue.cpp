#include "CommandQueue.h"


CommandQueue::CommandQueue()
{
    for (int i = 0; i < FRAME_BUFFER_COUNT; ++i)
    {
        mFenceValues[i] = 0;
    }
}

CommandQueue::~CommandQueue()
{
   /* if(mCommandQueue)
        mCommandQueue->Release();

    if (mCommandAllocator)
        mCommandAllocator->Release();

    if(mCommandList)
        mCommandList->Release();

    if(mFence)
        mFence->Release();

    CloseHandle(mFenceEvent);*/
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
    int inRtvDescriptorSize, D3D12_VERTEX_BUFFER_VIEW inVertexBufferView,
    RenderingContext inRenderingContext)
{
    //command list allocators can only be reset when the associated command lists have finished execution on the GPU
    mCommandAllocator->Reset();

    //however, when ExecuteCommandList() is called on a particular command list, 
    //that command list can then be reset at any time and must be before re-recording
    mCommandList->Reset(mCommandAllocator.Get(), NULL);

    //here we start recording commands into the commandlist
    inRenderingContext.FlushRenderState(mCommandList);

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
    inRenderingContext.Draw(mCommandList, inVertexBufferView, 3, 1, 0, 0);

    //transition the frame index render target from RENDER TARGET to PRESENT state
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(inRenderTarget.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    mCommandList->Close();
}

void CommandQueue::ExecuteCommandList()
{
    ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
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

bool CommandQueue::CreatePipelineState(Microsoft::WRL::ComPtr<ID3D12Device> inDevice, RenderingContext inRenderingContext)
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
    pso_desc.pRootSignature = inRenderingContext.GetRootSignature().Get();
    pso_desc.VS = CD3DX12_SHADER_BYTECODE(vertex_shader.Get());
    pso_desc.PS = CD3DX12_SHADER_BYTECODE(pixel_shader.Get());
    pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso_desc.SampleDesc.Count = 1;

    hr = inDevice->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&mPipelineState));
    if (FAILED(hr))
        return false;

    return true;
}

void CommandQueue::WaitForGPU(SwapChain inSwapchain)
{
    //schedule a Signal command in the queue
    mCommandQueue->Signal(mFence.Get(), mFenceValues[inSwapchain.GetFrameIndex()]);

    //wait until the fence has been processed
    mFence->SetEventOnCompletion(mFenceValues[inSwapchain.GetFrameIndex()], mFenceEvent);
    WaitForSingleObjectEx(mFenceEvent, INFINITE, false);

    //increment the fence value for the current frame.
    mFenceValues[inSwapchain.GetFrameIndex()]++;
}

void CommandQueue::WaitForPreviousFrame(SwapChain inSwapchain)
{
    HRESULT hr;

    //update fence to a specific value
    const UINT64 current_fence_value = mFenceValues[inSwapchain.GetFrameIndex()];
    hr = mCommandQueue->Signal(mFence.Get(), current_fence_value);

    //update the frame index
    inSwapchain.UpdateFrameIndex();

    // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
    // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
    if (mFence->GetCompletedValue() < mFenceValues[inSwapchain.GetFrameIndex()])
    {
        //create an event which is signaled once the fence's current value is "fenceValue"
        hr = mFence->SetEventOnCompletion(mFenceValues[inSwapchain.GetFrameIndex()], mFenceEvent);
        if (FAILED(hr))
        {
            assert("mFence->SetEventOnCompletion() failed");
        }
        //We will wait until the fence has triggered the event that it's current value has reached "fenceValue"
        //once its value has reached "fenceValue", we know the command queue has finished executing
        WaitForSingleObjectEx(mFenceEvent, INFINITE, false);
    }

    //increment fenceValue for next frame
    mFenceValues[inSwapchain.GetFrameIndex()] = current_fence_value + 1;
}