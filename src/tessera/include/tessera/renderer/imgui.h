#pragma once

#include <tessera/pch.h>
#include <tessera/rhi_dx12/device.h>

namespace Tessera {

struct DearImGui {
  DearImGui(WeakRef<Device> wDevice, const HWND hwnd);
  ~DearImGui();

  Error Render();

  ImGuiContext* context;
  ImGuiIO& io;
  WeakRef<Device> wDevice;
};
}  // namespace Tessera
