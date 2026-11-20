#pragma once
#include <tessera/pch.h>
#include <tessera/platform/window.h>
#include <tessera/renderer/imgui.h>
#include <tessera/rhi_dx12/device.h>

namespace Tessera {
struct Renderer {
  static Error Create(Ref<Renderer>& renderer, Window& window);
  ~Renderer();
  Ref<Device> device;
  Scope<DearImGui> imgui;

 private:
  Renderer(Window& window, Error& err);
};
}  // namespace Tessera
