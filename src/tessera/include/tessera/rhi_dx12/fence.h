#pragma once

#include <tessera/pch.h>

namespace Tessera {
struct Fence : NonCopyable {
  Fence() = delete;
  Fence(ID3D12Device10* device, ID3D12CommandQueue* cmdQueue, Error& err,
    uint32_t timeBeforeTimeoutMS = 20000);

  ~Fence();

  inline void Flush() { SignalAndWait(); }

  inline ID3D12Fence1* getFence() const noexcept { return fence.get(); }

  Error SignalAndWait();

  Error validate() const noexcept;
  Fence& operator=(Fence&& other);

 private:
  uint64_t fenceValue = 0;
  DWORD timeBeforeTimeoutMS;
  HANDLE fenceEvent;
  ComScope<ID3D12Fence1> fence;

  ID3D12CommandQueue* cmdQueue;
};
}  // namespace Tessera
