#include <tessera/rhi_dx12/swapchain.h>

namespace Tessera {
Swapchain::Swapchain(Error& err, const bool fullscreen, Vec2Rounded windowDimensions,
  const HWND hwnd, IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue) {
  BOOL isTearingSupported = false;
  HRESULT hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
    &isTearingSupported, sizeof(isTearingSupported));
  if (SUCCEEDED(hr) && isTearingSupported)
    SWAPCHAIN_FLAGS |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

  DXGI_SWAP_CHAIN_DESC1 desc = {};
  desc.Width = windowDimensions.x;
  desc.Height = windowDimensions.y;
  desc.Stereo = false;
  desc.Format = COLOR_FORMAT;
  desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.BufferCount = FRAME_COUNT;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  desc.Scaling = DXGI_SCALING_STRETCH;
  desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
  desc.Flags = SWAPCHAIN_FLAGS;

  DXGI_SWAP_CHAIN_FULLSCREEN_DESC descFull = {};
  descFull.Windowed = !fullscreen;

  ComScope<IDXGISwapChain1> tempSwapChain;
  hr = factory->CreateSwapChainForHwnd(cmdQueue, hwnd, &desc, &descFull, nullptr, &tempSwapChain);
  if (FAILED(hr)) {
    err = Error{InvalidArgument, std::format("Temp. SwapChain could not be created: {}", hr)};
    return;
  }

  hr = tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapchain));
  if (FAILED(hr))
    err =
      Error{Unknown, std::format("Ok how tf did the real swapchain creation failed???: {}", hr)};
  return;
}

Error Swapchain::Create(Ref<Swapchain>& swapchain, const bool fullscreen,
  Vec2Rounded windowDimensions, const HWND hwnd, Fence* fence, ID3D12Device10* device,
  IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue) {
  Error err = Error{};
  Swapchain* buf = new Swapchain(err, fullscreen, windowDimensions, hwnd, factory, cmdQueue);
  if (err) {
    delete buf;
    return err;
  }

  err = buf->createRtvHeap(device);
  if (err) {
    delete buf;
    return err;
  }

  err = buf->GetBuffers(device);
  if (err) {
    delete buf;
    return err;
  }

  buf->fence = fence;
  swapchain.reset(buf);
  return err;
}

Error Swapchain::Present() {
  HRESULT hr = swapchain->Present(1, 0);
  if (FAILED(hr)) {
    return Error{Unknown, std::format("Presentation of the SwapChain failed: {}", hr)};
  }
  return Error{};
}

Error Swapchain::Resize(const uint16_t width, const uint16_t height, ID3D12Device10* device) {
  if (device == nullptr)
    return Error{InvalidArgument, "Could not resize the swapchain: device is nullptr"};

  fence->Flush();
  ReleaseBuffers();
  HRESULT hr =
    swapchain->ResizeBuffers(FRAME_COUNT, width, height, DXGI_FORMAT_UNKNOWN, SWAPCHAIN_FLAGS);
  if (FAILED(hr)) {
    return Error{Unknown, std::format("SwapChain could not be resized: {}", hr)};
  }
  Error err = GetBuffers(device);
  if (err)
    return err;

  return err;
}

Error Swapchain::GetBuffers(ID3D12Device* device) {
  if (device == nullptr)
    return Error{InvalidArgument, "GetBuffers failed: device is nullptr"};
  D3D12_RENDER_TARGET_VIEW_DESC desc{};
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  desc.Texture2D.MipSlice = 0;
  desc.Texture2D.PlaneSlice = 0;

  for (size_t i = 0; i < FRAME_COUNT; i++) {
    HRESULT hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&buffers[i]));
    if (FAILED(hr))
      return Error{Unknown, std::format("Getting the following buffer failed: nr.{}, {}", i, hr)};
    device->CreateRenderTargetView(buffers[i].get(), &desc, rtvHandles[i]);
  }
  return Error{};
}

Error Swapchain::createRtvHeap(ID3D12Device10* device) {
  if (device == nullptr)
    return Error{InvalidArgument, "Could not create rtv heap: device is nullptr"};

  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  desc.NumDescriptors = Swapchain::FRAME_COUNT;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  desc.NodeMask = 0;

  HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap));
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("Creating the rtv Heap failed: {}", hr)};

  auto firstHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
  auto handleIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  for (size_t i = 0; i < FRAME_COUNT; i++) {
    rtvHandles[i] = firstHandle;
    rtvHandles[i].ptr += handleIncrement * i;
  }
  return Error{};
}
}  // namespace Tessera
