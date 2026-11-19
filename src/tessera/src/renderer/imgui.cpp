#include <tessera/renderer/imgui.h>
#include <tessera/rhi_dx12/descriptorAllocator.h>

namespace Tessera {
DearImGui::DearImGui(WeakRef<Device> wDevice, const HWND hwnd)
    : context(ImGui::CreateContext()), io(ImGui::GetIO()), wDevice(wDevice) {
  IMGUI_CHECKVERSION();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  auto device = wDevice.lock();
  if (!device)
    return;

  ImGui_ImplDX12_InitInfo initInfo = {};
  initInfo.Device = device->getDevice();
  initInfo.CommandQueue = device->getCmdQueue();
  initInfo.NumFramesInFlight = Swapchain::FRAME_COUNT;
  initInfo.RTVFormat = Swapchain::COLOR_FORMAT;
  initInfo.SrvDescriptorHeap = device->getSrvDescriptorHeap();
  initInfo.UserData = device->getSrvHeap();
  initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info,
                                    D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
                                    D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
    static_cast<ExampleDescriptorHeapAllocator*>(info->UserData)
      ->Alloc(out_cpu_handle, out_gpu_handle);
  };
  initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info,
                                   D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
                                   D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
    static_cast<ExampleDescriptorHeapAllocator*>(info->UserData)->Free(cpu_handle, gpu_handle);
  };

  if (!ImGui_ImplDX12_Init(&initInfo)) {
    Tessera::logFatal("Imgui init failed");
    std::exit(1);
    return;
  }

  if (!ImGui_ImplWin32_Init(hwnd)) {
    Tessera::logFatal("Imgui init failed");
    std::exit(1);
    return;
  }
}

DearImGui::~DearImGui() {
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

Error DearImGui::Render() {
  auto device = wDevice.lock();
  if (!device)
    return Error{
      ErrorCode::InvalidArgument, "Could not render Imgui. WeakRef of Device is nullptr"
    };

  ImGui::Render();
  ID3D12DescriptorHeap* descriptorHeaps[] = {device->getSrvDescriptorHeap()};
  device->getCmdList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), device->getCmdList());
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
  return Error{};
}
}  // namespace Tessera
