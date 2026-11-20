#include <tessera/renderer/renderer.h>

namespace Tessera {
Error Renderer::Create(Ref<Renderer>& renderer, Window& window) {
  Error err = Error{};

  Renderer* buf = new Renderer(window, err);
  if (err) {
    delete buf;
    return err;
  }

  return err;
}
Renderer::Renderer(Window& window, Error& err) {}

Renderer::~Renderer() { logTrace("Destructing Renderer"); }
}  // namespace Tessera
