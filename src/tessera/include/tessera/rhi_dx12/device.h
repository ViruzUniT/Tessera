#pragma once

#include <tessera/core/assert.h>
#include <tessera/pch.h>
#include <tessera/rhi/shader.h>
#include <tessera/rhi_dx12/fence.h>
#include <tessera/rhi_dx12/swapchain.h>

#include "tessera/rhi_dx12/descriptorAllocator.h"

namespace Tessera {
class Device : public NonCopyableNonMovable {
 public:
  Device() = delete;
  ~Device() {
    swapchains.clear();
    TESSERA_ASSERT_MSG(swapchains.size() == 0, "Error not all swapchains got deleted");
    logTrace("Destructing Device");
    fence.reset();
  }
  static Error Create(Ref<Device>& device);
  Error CreateSwapchain(bool fullscreen, Vec2Rounded dimensions, HWND hwnd);

 public:
  Error Submit();
  Error ExecuteCommandList();
  Error BeginFrame(Swapchain* swapchain);
  Error EndFrame(Swapchain* swapchain);

  inline void SetPipelineState() {
    cmdList->SetPipelineState(pipelineState);
    cmdList->SetGraphicsRootSignature(rootSignature);
  }
  inline void ConfigureInputAssembler() {
    cmdList->IASetVertexBuffers(0, 1, &vbv);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  }
  void ConfigureRasterizer(Vec2Rounded windowDimensions);
  // currently not working cuz of blend settings
  inline void ConfigureOutputMerger() {
    f32 bf[] = {0.1f, 0.1f, 0.1f, 0.1f};
    cmdList->OMSetBlendFactor(bf);
  }

  void Draw(Vec2Rounded windowDimensions);

 public:
  // currently only used for imgui
  inline ID3D12Device10* getDevice() const { return device.get(); }
  inline size_t getSwapchainSize() const { return swapchains.size(); }
  inline Swapchain* getSwapchain(const size_t i = 0) { return swapchains.at(i).get(); }
  inline IDXGIFactory7* getFactory() const { return factory; }
  inline ID3D12CommandQueue* getCmdQueue() const { return cmdQueue.get(); }
  inline ID3D12DescriptorHeap* getSrvDescriptorHeap() const { return srvDescriptorHeap.get(); }
  inline ID3D12GraphicsCommandList7* getCmdList() const { return cmdList.get(); }
  inline ExampleDescriptorHeapAllocator* getSrvHeap() const { return srvHeap.get(); }

 private:
  Device(Error& err);

 private:
  // Temp
  constexpr static Vertex verticies[] = {
    // T1
    {-1.f, -1.f}, {0.f, 1.f}, {1.f, -1.f}
  };

 private:
  ComScope<ID3D12Device10> device;
  ComScope<ID3D12CommandQueue> cmdQueue;
  Scope<Fence> fence;
  ComScope<ID3D12CommandAllocator> cmdAlloc;
  ComScope<ID3D12GraphicsCommandList7> cmdList;
  ComScope<IDXGIFactory7> factory;
  ComScope<ID3D12DescriptorHeap> srvDescriptorHeap;
  ComScope<ID3D12Resource2> uploadBuffer, vertexBuffer;
  ComScope<ID3D12RootSignature> rootSignature;
  ComScope<ID3D12PipelineState> pipelineState;
  D3D12_VERTEX_BUFFER_VIEW vbv;
  Scope<ExampleDescriptorHeapAllocator> srvHeap;
  List<Ref<Swapchain>> swapchains;

 private:
  // Creation of the attributes
  Error createCmdQueue();
  Error createFence();
  Error createCommandAllocator();
  Error createCommandList();
  Error createUploadAndVertexBuffer();
  Error createSrvHeap();
};
}  // namespace Tessera
