#pragma once
#include <d3d12.h>
#include <tessera/pch.h>

namespace Tessera {
struct Shader {
  Scope<ID3DBlob> shader;
  const WCHAR* filename;
  const char* entrypoint;
  const char* target;
  UINT flags1;
  UINT flags2;
};
};  // namespace Tessera
