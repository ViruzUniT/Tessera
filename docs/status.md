# Status

Where the project stands now. Milestones: `docs/roadmap.md`. Update this file whenever a step is completed.

## Current milestone: M0 — Foundation and build

### Next steps

1. Replace `build.ps1`/`build.sh` with premake custom actions (see roadmap M0 for scope and fallback).
2. Sanitizers in DEBUG only.
3. Fix `Error::message` dangling `string_view`.

## Current state

- Win32 window; DX12 device, queue, swapchain, fence; one hard-coded triangle.
- ImGui (docking, multi-viewport) with an example panel, DX12 binding in `renderer/imgui.*`.
- Offline shader build: HLSL → DXIL with pinned DXC (`.tools/dxc/`, SHA-256 checked), currently driven by Sandbox's premake file.
- `build.ps1` handles gen/build/run/compdb/clean; `build.sh` is an empty placeholder.
- CI builds DEBUG and RELEASE on pull requests.
- `renderer/renderer.cpp` and `rhi_dx12/swapchain.*` are empty; `Window` owns RHI, ImGui and the frame loop; one frame in flight.

## Decisions

- 2026-11-17: Engine renamed **Tessera**; Sandbox keeps its name.
- 2026-11-17: Scripting language is **Luau**.
- 2026-11-17: ImGui ships as an optional `dev_ui` module (DEBUG/STAGING), used by Sandbox now and by the editor later.
- 2026-11-17: App model is a library core plus an optional `Application` layer.
- 2026-11-17: Vulkan bring-up (M4) comes before feature work (M5) to validate the RHI early.
- 2026-11-17: Backend is selected at renderer creation; backend differences are exposed through a capability query.
- Root-signature serialization blobs are temporary; no RHI storage unless diagnostics or reflection need them later.

## Known issues

- `core/DXDebugLayer.h` includes DX12/DXGI headers, and Sandbox calls it directly (fixed in M2).
- `platform/window.h` and `renderer/imgui.h` include `rhi_dx12/rhi.h` (fixed in M1/M2).
- `pch.h` pulls `windows.h`, ImGui and the ImGui DX12/Win32 backends into every file (M2).
- Sanitizers apply to all configs, including RELEASE (M0).
- `Error::message` dangles when built from `std::format` (M0).
