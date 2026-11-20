#pragma once
#include <tessera/pch.h>
#include <tessera/platform/window.h>

namespace Tessera {
class Device;
struct DearImGui;
struct Swapchain;

struct Renderer {
  static Error Create(Ref<Renderer>& renderer, Window& window);
  ~Renderer();
  Ref<Device> device;
  Scope<DearImGui> imgui;
  Swapchain* swapchain;
  Vec2Rounded dimensions;

  Error BeginFrame();
  Error EndFrame();
  Error Resize(Vec2Rounded dimensions);

 private:
  Renderer(Window& window, Error& err);
};
}  // namespace Tessera
