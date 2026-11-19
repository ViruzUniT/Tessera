# Status

Where the project stands now. Milestones: `docs/roadmap.md`. Update this file whenever a step is completed.

## Current milestone: M1 — Frame ownership and clear color

M0 is complete (2026-11-17).

### Next steps

1. Split `RHI` into Device (created once) and Swapchain (per window: back buffers, RTVs, resize).
2. Renderer owns the frame loop; `Window` shrinks to native handle + events.
3. Frames in flight: one command allocator and fence value per back buffer.

## Current state

- Win32 window; DX12 device, queue, swapchain, fence; one hard-coded triangle.
- ImGui (docking, multi-viewport) with an example panel, DX12 binding in `renderer/imgui.*`.
- Offline shader build: HLSL → DXIL with pinned DXC (`.tools/dxc/`, SHA-256 checked), currently driven by Sandbox's premake file.
- Build workflow is premake-only: `premake5 gen|build|run|compdb|clean` (`premake/actions.lua`), DXC bootstrap in `premake/dxc.lua`. `premake5 run` also writes all output to `run.log`.
- ASan + UBSan instrument both projects in DEBUG and STAGING (Clang/GCC); RELEASE has none.
- CI builds DEBUG and RELEASE on pull requests.
- `renderer/renderer.cpp` and `rhi_dx12/swapchain.*` are empty; `Window` owns RHI, ImGui and the frame loop; one frame in flight.

## Decisions

- 2026-11-17: Engine renamed **Tessera**; Sandbox keeps its name.
- 2026-11-17: Scripting language is **Luau**.
- 2026-11-17: ImGui ships as an optional `dev_ui` module (DEBUG/STAGING), used by Sandbox now and by the editor later.
- 2026-11-17: App model is a library core plus an optional `Application` layer.
- 2026-11-17: Vulkan bring-up (M4) comes before the pipeline work (M5) and features (M6) to validate the RHI early.
- 2026-11-17: Tessera is **3D-only**; no 2D renderer.
- 2026-11-17: The graphics pipeline gets its own milestone (M5) after Vulkan, so its design fits both backends.
- 2026-11-17: Backend is selected at renderer creation; backend differences are exposed through a capability query.
- 2026-11-17: Sanitizers run in DEBUG and STAGING (STAGING catches bugs that only appear with optimizations), not RELEASE.
- Root-signature serialization blobs are temporary; no RHI storage unless diagnostics or reflection need them later.

## Known issues

- `core/DXDebugLayer.h` includes DX12/DXGI headers, and Sandbox calls it directly (fixed in M2).
- `platform/window.h` and `renderer/imgui.h` include `rhi_dx12/rhi.h` (fixed in M1/M2).
- `pch.h` pulls `windows.h`, ImGui and the ImGui DX12/Win32 backends into every file (M2).
- UBSan reports "upcast of misaligned address … for type `ID3D12Resource`" once when ImGui creates a platform window. Cause: `vendor/imgui/backends/imgui_impl_dx12.cpp:1166` passes an uninitialized local to `IID_PPV_ARGS`. It's harmless in practice (the value is overwritten right after) and lives in vendor code. Options: ignore it, exclude `vendor/imgui` from UBSan, or report it upstream.
