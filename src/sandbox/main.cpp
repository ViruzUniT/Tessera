#include <tessera/core/DXDebugLayer.h>
#include <tessera/core/base.h>
#include <tessera/platform/window.h>
#include <tessera/renderer/renderer.h>

int main() {
  DXDebugLayer::Get().Init();
  Tessera::logInfo("Sandbox starting.");
  {
    Tessera::Error err;
    auto window = Tessera::Window(1200, 720, "test", "test");

    Tessera::Ref<Tessera::Renderer> renderer;
    err = Tessera::Renderer::Create(renderer, window);

    if (err) {
      Tessera::logFatal(std::format("Renderer could not be created {}\n\r{}",
        static_cast<int>(err.code), err.message));
      return 1;
    }

    window.SetShow(Tessera::Window::CmdShow::ShowNormal);
    Tessera::logTrace("Sandbox started.");
    DXDebugLayer::Get().Report();

    Tessera::logTrace("Entering main loop");
    bool showDemoWindow = false;
    while (!err) {
      err = window.Update();
      if (err)
        break;

      if (window.hasResized) {
        err = renderer->Resize(window.dimensions);
        if (err)
          break;
        window.hasResized = false;
      }
      err = renderer->BeginFrame();
      if (err)
        break;

      if (showDemoWindow)
        ImGui::ShowDemoWindow();
      {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Fuck yeah");

        ImGui::Text("Text ig");
        ImGui::SliderFloat("floaty boi", &f, 0.f, 100.1f);

        if (ImGui::Button("Count my ass"))
          counter++;
        ImGui::SameLine();
        ImGui::Text("counter: = %d", counter);

        ImGui::Checkbox("Demo Window", &showDemoWindow);
        ImGui::End();
      }

      err = renderer->EndFrame();
    }
    Tessera::logTrace("Sandbox terminating.");
  }
  DXDebugLayer::Get().Shutdown();

  return 0;
}
