#pragma once

#if defined(_WIN32)

#include <d3d12.h>
#include <dxgi1_6.h>
#include <tessera/pch.h>

#ifdef TESSERA_DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

class DXDebugLayer : Tessera::NonCopyable {
 public:
  bool Init();

  // Call this after D3D12CreateDevice().
  bool InitDevice(ID3D12Device* device);

  void Shutdown();
  void Report();

 public:
  inline static DXDebugLayer& Get() {
    static DXDebugLayer instance;
    return instance;
  }

 private:
  DXDebugLayer() = default;

#ifdef TESSERA_DEBUG
  static void CALLBACK DebugMessageCallback(D3D12_MESSAGE_CATEGORY category,
      D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR description, void* context);

  Tessera::ComScope<ID3D12Debug5> m_d3d12Debug;
  Tessera::ComScope<IDXGIDebug1> m_dxgiDebug;
  Tessera::ComScope<ID3D12InfoQueue1> m_infoQueue;

  DWORD m_callbackCookie = 0;
#endif
};

#endif
