#include <tessera/rhi_dx12/fence.h>

namespace Tessera {
Fence::Fence(ID3D12Device10* device, ID3D12CommandQueue* cmdQueue, Error& err,
  uint32_t timeBeforeTimeoutMS)
    : timeBeforeTimeoutMS(timeBeforeTimeoutMS), cmdQueue(cmdQueue) {
  if (cmdQueue == nullptr) {
    err = Error{InvalidArgument, "Creating fence Failed. cmdQueue was a nullptr"};
  }
  HRESULT hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
  if (FAILED(hr)) {
    err = Error{InvalidArgument, std::format("Creating fence failed: {}", hr)};
    return;
  }

  fenceEvent = CreateEvent(nullptr, false, false, nullptr);
  if (fenceEvent == NULL) {
    err =
      Error{FenceError, std::format("fenceEvent is null, mby this helps: {}", GetLastError())};
  }
}
Fence::~Fence() {
  logTrace("Destructing Fence");
  Flush();
  if (fenceEvent)
    CloseHandle(fenceEvent);
}

Error Fence::SignalAndWait() {
  auto err = validate();
  if (err)
    return err;

  HRESULT hr = cmdQueue->Signal(fence.get(), ++fenceValue);
  if (FAILED(hr))
    return Error{InvalidArgument, std::format("Signaling the Command Queue failed: {}", hr)};

  hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
  if (FAILED(hr))
    return Error{FenceError, std::format("Setting the event failed: {}", hr)};

  DWORD res = WaitForSingleObject(fenceEvent, timeBeforeTimeoutMS);
  if (res != WAIT_OBJECT_0)
    return Error{FenceTimeout, std::format("Fence timed out: {}", res)};

  return Error{};
}

Error Fence::validate() const noexcept {
  if (fence == nullptr)
    return Error{FenceError, "Fence was not created correctly"};
  if (fenceEvent == NULL)
    return Error{FenceError, "FenceEvent was not created correctly"};
  if (cmdQueue == nullptr)
    return Error{InvalidArgument, "cmdQueue is nullptr"};

  return Error{};
}

Fence& Fence::operator=(Fence&& other) {
  if (this == &other)
    return *this;

  fenceValue = other.fenceValue;
  timeBeforeTimeoutMS = other.timeBeforeTimeoutMS;
  fenceEvent = other.fenceEvent;
  fence = std::move(other.fence);
  cmdQueue = other.cmdQueue;

  other.fenceEvent = nullptr;

  return *this;
}
};  // namespace Tessera
