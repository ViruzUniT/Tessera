#include <tessera/core/DXDebugLayer.h>
#include <tessera/core/base.h>
#include <tessera/platform/window.h>

int main() {
  DXDebugLayer::Get().Init();
  Tessera::logInfo("Sandbox starting.");
  {
    Tessera::Error err;
    auto window = Tessera::Window(1200, 720, "test", "test");

    if (err.code > Tessera::ErrorCode::None) {
      Tessera::logFatal(std::format("Window could not be created {}\n\r{}",
        static_cast<int>(err.code), err.message));
      return 1;
    }

    window.SetShow(Tessera::Window::CmdShow::ShowNormal);
    Tessera::logTrace("Sandbox started.");
    DXDebugLayer::Get().Report();
    Tessera::logTrace("Updating Sandbox");
    err = window.Update();
    Tessera::logTrace("Entering main loop");

    while (!err) {
      err = window.Update();
    }
    Tessera::logTrace("Sandbox terminating.");
    Tessera::Window::Close(window);
  }
  DXDebugLayer::Get().Shutdown();

  return 0;
}
