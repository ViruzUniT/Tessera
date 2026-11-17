#pragma once
#include <tessera/pch.h>

namespace Tessera {
namespace Renderer {
void Initialize(HWND handle, uint16_t width, uint16_t height);
void RenderFrame();
void Resize(uint16_t width, uint16_t height);
}  // namespace Renderer
}  // namespace Tessera
