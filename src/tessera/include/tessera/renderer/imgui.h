#pragma once

#include <tessera/pch.h>
#include <tessera/rhi_dx12/rhi.h>

namespace Tessera {

struct DearImGui {
  DearImGui(WeakRef<RHI> wRhi, const HWND hwnd);
  ~DearImGui();

  Error Render();

  ImGuiContext* context;
  ImGuiIO& io;
  WeakRef<RHI> wRhi;
};
}  // namespace Tessera
