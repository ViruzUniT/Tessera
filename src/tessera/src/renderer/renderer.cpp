#include <tessera/renderer/imgui.h>
#include <tessera/renderer/renderer.h>
#include <tessera/rhi_dx12/device.h>

namespace Tessera {
Error Renderer::Create(Ref<Renderer>& renderer, Window& window) {
  Error err = Error{};

  Ref<Renderer> buf(new Renderer(window, err));
  if (err)
    return err;

  renderer = buf;
  return err;
}

Error Renderer::BeginFrame() {
  Error err = device->BeginFrame(swapchain);
  if (err)
    return err;

  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  device->Draw(dimensions);

  return err;
}
Error Renderer::EndFrame() {
  imgui->Render();

  Error err = device->EndFrame(swapchain);
  if (err)
    return err;

  err = device->Submit();
  if (err)
    return err;

  err = swapchain->Present();
  if (err)
    return err;
  return err;
}
Error Renderer::Resize(Vec2Rounded dimensions) {
  logDebug(std::format("New dimensions: x:{} y: {}", dimensions.x, dimensions.y));
  this->dimensions = dimensions;
  return swapchain->Resize(dimensions.x, dimensions.y, device->getDevice());
}

Renderer::Renderer(Window& window, Error& err) : dimensions(window.dimensions) {
  logTrace("Creating Device");
  err = Device::Create(device);
  if (err) {
    Tessera::logError(std::format("Device could not be created: {}", static_cast<int>(err.code)));
    Tessera::logError(err.message);
    std::exit(1);
    return;
  }

  logTrace("Creating Swapchain");
  auto res = device->CreateSwapchain(window.fullscreen, window.dimensions, window.hwnd);
  if (!res) {
    Tessera::logError(
      std::format("Swapchain could not be created: {}", static_cast<int>(err.code)));
    Tessera::logError(err.message);
    std::exit(1);
    return;
  }
  swapchain = *res;

  logTrace("Creating imgui");
  imgui = std::make_unique<DearImGui>(device, window.hwnd);
}

Renderer::~Renderer() { logTrace("Destructing Renderer"); }
}  // namespace Tessera
