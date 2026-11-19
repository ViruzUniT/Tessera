#include <tessera/rhi_dx12/device.h>

namespace Tessera {
Error Device::Create(Ref<Device>& device) {
  Error err;
  Device* tempDevice = new Device(err);
  if (err) {
    delete tempDevice;
    return err;
  }
  device.reset(tempDevice);
  return Error{};
}
Error Device::CreateSwapchain(bool fullscreen, Vec2Rounded dimensions, HWND hwnd) {
  Ref<Swapchain> swapchain;
  Error err = Swapchain::Create(swapchain, fullscreen, dimensions, hwnd, fence.get(), getDevice(),
    getFactory(), getCmdQueue());
  if (err)
    return err;
  swapchains.push_back(swapchain);
  return err;
}

Device::Device(Error& err) {
  logTrace("Creating Device");
  HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
  if (FAILED(hr)) {
    err = Error{PlatformError, "Device Could not be created"};
    return;
  }

  logTrace("Creating cmdQueue");
  err = createCmdQueue();
  if (err) {
    return;
  }

  logTrace("Creating Fence");
  err = createFence();
  if (err) {
    return;
  }

  logTrace("Creating Cmd Alloc");
  err = createCommandAllocator();
  if (err) {
    return;
  }

  logTrace("Creating Cmd List");
  err = createCommandList();
  if (err)
    return;

  logTrace("Creating Factory");
  hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
  if (FAILED(hr)) {
    err = Error{Unknown, std::format("Creating the factory failed: {}", hr)};
    return;
  }

  logTrace("Creating Upload and Vertex Buffers");
  err = createUploadAndVertexBuffer();
  if (err)
    return;
  // Mby need to add wait for buffers but i think i did that

  logTrace("Creating srv heap");
  err = createSrvHeap();
}
Error Device::ExecuteCommandList() {
  ID3D12CommandList* list[] = {cmdList.get()};
  cmdQueue->ExecuteCommandLists(1, list);
  return fence->SignalAndWait();
}

Error Device::BeginFrame(Swapchain* swapchain) {
  if (swapchain == nullptr)
    return Error{InvalidArgument, "Could not begin frame: swapchain is nullptr"};

  swapchain->setCurrentBuffer();
  HRESULT hr = cmdAlloc->Reset();
  if (FAILED(hr))
    return Error{Unknown, std::format("Command allocator reset failed: {}", hr)};

  hr = cmdList->Reset(cmdAlloc.get(), pipelineState.get());
  if (FAILED(hr))
    return Error{Unknown, std::format("Command list reset failed: {}", hr)};

  D3D12_RESOURCE_BARRIER barr = {};
  barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barr.Transition.pResource = swapchain->getCurrentBuffer();
  barr.Transition.Subresource = 0;
  barr.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barr.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

  cmdList->ResourceBarrier(1, &barr);
  auto currentRtvHandle = swapchain->getCurrentRtvHandle();
  cmdList->ClearRenderTargetView(currentRtvHandle, (f32[4]){.4f, .4f, .8f, 1.f}, 0, nullptr);
  cmdList->OMSetRenderTargets(1, swapchain->getCurrentRtvHandlePtr(), false, nullptr);

  return Error{};
}

Error Device::EndFrame(Swapchain* swapchain) {
  D3D12_RESOURCE_BARRIER barr = {};
  barr.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barr.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barr.Transition.pResource = swapchain->getCurrentBuffer();
  barr.Transition.Subresource = 0;
  barr.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barr.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

  cmdList->ResourceBarrier(1, &barr);

  HRESULT hr = cmdList->Close();
  if (FAILED(hr))
    return Error{Unknown, std::format("Command list close failed: {}", hr)};

  return Error{};
}

Error Device::Submit() {
  Error err;
  err = ExecuteCommandList();
  return err;
}

void Device::ConfigureRasterizer(const Vec2Rounded windowDimensions) {
  D3D12_VIEWPORT vp;
  vp.TopLeftX = vp.TopLeftY = 0;
  vp.Width = windowDimensions.x;
  vp.Height = windowDimensions.y;
  vp.MinDepth = 0.f;
  vp.MaxDepth = 1.f;
  cmdList->RSSetViewports(1, &vp);

  RECT scRect;
  scRect.left = scRect.top = 0;
  scRect.right = windowDimensions.x;
  scRect.bottom = windowDimensions.y;
  cmdList->RSSetScissorRects(1, &scRect);
}

void Device::Draw(const Vec2Rounded windowDimensions) {
  SetPipelineState();
  ConfigureInputAssembler();
  ConfigureRasterizer(windowDimensions);
  ConfigureOutputMerger();
  cmdList->DrawInstanced(_countof(verticies), 1, 0, 0);
}

Error Device::createCmdQueue() {
  D3D12_COMMAND_QUEUE_DESC cmdQueDesc = {};
  cmdQueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  cmdQueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
  cmdQueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cmdQueDesc.NodeMask = 0;
  HRESULT hr = device->CreateCommandQueue(&cmdQueDesc, IID_PPV_ARGS(&cmdQueue));
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("cmdQueue could not be created: {}", hr)};
  return Error{};
}
Error Device::createFence() {
  Error err;
  fence = std::make_unique<Fence>(device.get(), cmdQueue, err);
  if (err)
    return err;
  err = fence->validate();
  if (err)
    return err;
  return Error{};
}

Error Device::createCommandAllocator() {
  HRESULT hr =
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
  if (FAILED(hr)) {
    return Error{InvalidArgument, std::format("Command Allocator could not be created: {}", hr)};
  }
  return Error{};
}

Error Device::createCommandList() {
  HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, nullptr,
    IID_PPV_ARGS(&cmdList));
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("Command list could not be created: {}", hr)};
  hr = cmdList->Close();
  if (FAILED(hr))
    return Error{Unknown, std::format("Command list couldn't be closed: {}", hr)};
  return Error{};
}

Error Device::createUploadAndVertexBuffer() {
  D3D12_HEAP_PROPERTIES hpDefault{
    .Type = D3D12_HEAP_TYPE_DEFAULT,
    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask = 0,
    .VisibleNodeMask = 0
  };
  D3D12_HEAP_PROPERTIES hpUpload{
    .Type = D3D12_HEAP_TYPE_UPLOAD,
    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask = 0,
    .VisibleNodeMask = 0
  };

  D3D12_RESOURCE_DESC rd{
    .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
    .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
    .Width = 1024,
    .Height = 1,
    .DepthOrArraySize = 1,
    .MipLevels = 1,
    .Format = DXGI_FORMAT_UNKNOWN,
    .SampleDesc = {.Count = 1, .Quality = 0},
    .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
    .Flags = D3D12_RESOURCE_FLAG_NONE
  };

  D3D12_INPUT_ELEMENT_DESC vertexLayout[] = {
    {"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
      0},
  };

  Shader rootSignatureShader("rootSignature.dxil");
  Shader vertexShader("vertex.dxil");
  Shader pixelShader("pixel.dxil");

  HRESULT hr = device->CreateRootSignature(0, rootSignatureShader.GetBuffer(),
    rootSignatureShader.GetSize(), IID_PPV_ARGS(&rootSignature));
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("Creating root signature failed: {}", hr)};

  D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod{};
  gfxPsod.pRootSignature = rootSignature;
  gfxPsod.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
  gfxPsod.InputLayout.NumElements = _countof(vertexLayout);
  gfxPsod.InputLayout.pInputElementDescs = vertexLayout;
  gfxPsod.VS.pShaderBytecode = vertexShader.GetBuffer();
  gfxPsod.VS.BytecodeLength = vertexShader.GetSize();
  gfxPsod.PS.pShaderBytecode = pixelShader.GetBuffer();
  gfxPsod.PS.BytecodeLength = pixelShader.GetSize();
  gfxPsod.RasterizerState = {
    .FillMode = D3D12_FILL_MODE_SOLID,
    .CullMode = D3D12_CULL_MODE_NONE,
    .FrontCounterClockwise = false,
    .DepthBias = 0,
    .DepthBiasClamp = 0,
    .SlopeScaledDepthBias = 0,
    .DepthClipEnable = false,
    .MultisampleEnable = false,
    .AntialiasedLineEnable = false,
    .ForcedSampleCount = 0,
    .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
  };
  gfxPsod.StreamOutput = {
    .pSODeclaration = nullptr,
    .NumEntries = 0,
    .pBufferStrides = nullptr,
    .NumStrides = 0,
    .RasterizedStream = 0,
  };
  gfxPsod.NumRenderTargets = 1;
  gfxPsod.RTVFormats[0] = Swapchain::COLOR_FORMAT;
  gfxPsod.DSVFormat = DXGI_FORMAT_UNKNOWN;
  gfxPsod.BlendState.AlphaToCoverageEnable = false;
  gfxPsod.BlendState.IndependentBlendEnable = false;
  gfxPsod.BlendState.RenderTarget[0] = {
    .BlendEnable = false,
    .LogicOpEnable = false,
    .SrcBlend = D3D12_BLEND_ZERO,
    .DestBlend = D3D12_BLEND_ZERO,
    .BlendOp = D3D12_BLEND_OP_ADD,
    .SrcBlendAlpha = D3D12_BLEND_ZERO,
    .DestBlendAlpha = D3D12_BLEND_ZERO,
    .BlendOpAlpha = D3D12_BLEND_OP_ADD,
    .LogicOp = D3D12_LOGIC_OP_NOOP,
    .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
  };
  gfxPsod.DepthStencilState = {
    .DepthEnable = false,
    .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
    .DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS,
    .StencilEnable = false,
    .StencilReadMask = 0,
    .StencilWriteMask = 0,
    .FrontFace =
      {
        .StencilFailOp = D3D12_STENCIL_OP_ZERO,
        .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
        .StencilPassOp = D3D12_STENCIL_OP_KEEP,
        .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS,
      },
    .BackFace = {
      .StencilFailOp = D3D12_STENCIL_OP_ZERO,
      .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
      .StencilPassOp = D3D12_STENCIL_OP_KEEP,
      .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS,
    },
  };
  gfxPsod.SampleMask = 0xFFFFFFFF;
  gfxPsod.SampleDesc.Count = 1;
  gfxPsod.SampleDesc.Quality = 0;
  gfxPsod.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  gfxPsod.DS.BytecodeLength = 0;
  gfxPsod.DS.pShaderBytecode = nullptr;
  gfxPsod.HS.BytecodeLength = 0;
  gfxPsod.HS.pShaderBytecode = nullptr;
  gfxPsod.GS.BytecodeLength = 0;
  gfxPsod.GS.pShaderBytecode = nullptr;
  gfxPsod.NodeMask = 0;
  gfxPsod.CachedPSO.CachedBlobSizeInBytes = 0;
  gfxPsod.CachedPSO.pCachedBlob = nullptr;
  gfxPsod.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

  hr = device->CreateGraphicsPipelineState(&gfxPsod, IID_PPV_ARGS(&pipelineState));
  if (FAILED(hr))
    return Error{
      InvalidArgument, std::format("Creating the graphics pipelineState failed: {}", hr)
    };

  hr = device->CreateCommittedResource(&hpUpload, D3D12_HEAP_FLAG_NONE, &rd,
    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("Creating an upload resource failed: {}", hr)};
  hr = device->CreateCommittedResource(&hpDefault, D3D12_HEAP_FLAG_NONE, &rd,
    D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertexBuffer));
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("Creating a default resource failed: {}", hr)};

  void* uploadBufferAddr;
  D3D12_RANGE readRange{0, 0};
  hr = uploadBuffer->Map(0, &readRange, &uploadBufferAddr);
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("Mapping the upload buffer failed: {}", hr)};

  D3D12_RANGE writtenRange{0, sizeof(verticies)};
  memcpy(uploadBufferAddr, verticies, sizeof(verticies));
  uploadBuffer->Unmap(0, &writtenRange);

  hr = cmdAlloc->Reset();
  if (FAILED(hr)) {
    return Error{
      Unknown, std::format("Resetting the command allocator for the vertex upload failed: {}", hr)
    };
  }

  hr = cmdList->Reset(cmdAlloc, nullptr);
  if (FAILED(hr)) {
    return Error{
      Unknown, std::format("Resetting the command list for the vertex upload failed: {}", hr)
    };
  }

  cmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, sizeof(verticies));

  D3D12_RESOURCE_BARRIER vertexBufferBarrier{};
  vertexBufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  vertexBufferBarrier.Transition.pResource = vertexBuffer;
  vertexBufferBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  vertexBufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
  vertexBufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  cmdList->ResourceBarrier(1, &vertexBufferBarrier);

  hr = cmdList->Close();
  if (FAILED(hr)) {
    return Error{Unknown, std::format("Closing the vertex upload command list failed: {}", hr)};
  }

  logDebug(std::format("Vertex Buffer: {}\nUpload Buffer: {}",
    static_cast<void*>(vertexBuffer.get()), static_cast<void*>(uploadBuffer.get())));

  ID3D12CommandList* list[] = {cmdList};
  cmdQueue->ExecuteCommandLists(1, list);

  Error err = fence->SignalAndWait();
  if (err)
    return err;

  vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
  vbv.SizeInBytes = sizeof(verticies);
  vbv.StrideInBytes = sizeof(Vertex);

  return Error{};
}

Error Device::createSrvHeap() {
  D3D12_DESCRIPTOR_HEAP_DESC desc{};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  desc.NumDescriptors = 64;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  desc.NodeMask = 0;

  HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvDescriptorHeap));
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("Creating the SRV heap failed: {}", hr)};

  srvHeap = std::make_unique<ExampleDescriptorHeapAllocator>();
  srvHeap->Create(device.get(), srvDescriptorHeap.get());
  return Error{};
}
};  // namespace Tessera
