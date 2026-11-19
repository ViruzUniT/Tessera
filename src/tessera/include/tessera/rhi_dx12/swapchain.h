#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <tessera/pch.h>
#include <tessera/rhi_dx12/fence.h>

namespace Tessera {
struct Swapchain : NonCopyable {
  static Error Create(Ref<Swapchain>& swapchain, const bool fullscreen,
    Vec2Rounded windowDimensions, const HWND hwnd, Fence* fence, ID3D12Device10* device,
    IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue);
  ~Swapchain() {
    logTrace("Destructing Swapchain");
    if (fence != nullptr)
      fence->Flush();
  }

  Error Present();
  Error Resize(const uint16_t width, const uint16_t height, ID3D12Device10* device);
  Error GetBuffers(ID3D12Device* device);
  inline ID3D12DescriptorHeap* getRtvHeap() const { return rtvHeap.get(); }
  inline D3D12_CPU_DESCRIPTOR_HANDLE getCurrentRtvHandle() { return rtvHandles[currentBuffer]; }
  inline D3D12_CPU_DESCRIPTOR_HANDLE* getCurrentRtvHandlePtr() {
    return &rtvHandles[currentBuffer];
  }
  inline void setCurrentBuffer() { currentBuffer = swapchain->GetCurrentBackBufferIndex(); }
  inline ID3D12Resource2* getCurrentBuffer() const { return buffers[currentBuffer]; }
  inline void ReleaseBuffers() {
    for (size_t i = 0; i < FRAME_COUNT; i++) {
      buffers[i].reset();
    }
  }

  static constexpr uint8_t FRAME_COUNT = 2;
  static constexpr DXGI_FORMAT COLOR_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
  uint8_t currentBuffer = 0;

 private:
  Swapchain(Error& err, const bool fullscreen, Vec2Rounded windowDimensions, const HWND hwnd,
    IDXGIFactory7* factory, ID3D12CommandQueue* cmdQueue);

  uint32_t SWAPCHAIN_FLAGS =
    DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  // | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

  Error createRtvHeap(ID3D12Device10* device);

  ComScope<IDXGISwapChain4> swapchain;
  ComScope<ID3D12DescriptorHeap> rtvHeap;
  Array<D3D12_CPU_DESCRIPTOR_HANDLE, FRAME_COUNT> rtvHandles;
  Array<ComScope<ID3D12Resource2>, FRAME_COUNT> buffers;
  Fence* fence = nullptr;
};
}  // namespace Tessera
