# Roadmap

Tessera is a **3D-only** engine; there is no 2D renderer on the roadmap. Milestones in recommended order. Each one ends with something you can run and check. The current milestone and its next steps are in `docs/status.md`.

Items tagged *(review)* come from the architecture review in the "Ngin Architecture Report" (written before the rename).

## M0 — Foundation and build

- [x] Rename the project to Tessera (namespace, include paths, macros, premake projects, docs). Sandbox keeps its name.
- [x] Replace `build.ps1`/`build.sh` with premake commands: `premake5 gen|build|run|compdb|clean` (`premake/actions.lua`), DXC bootstrap in Lua (`premake/dxc.lua`); CI, hooks and docs updated.
- [x] Sanitizers only outside RELEASE: ASan + UBSan compiled and linked in DEBUG and STAGING.
- [x] Fix `Error::message`: now an owning `std::string` (`f97452d`).

**Exit:** a fresh clone builds and runs with `premake5` commands only; no wrapper scripts left.

## M1 — Frame ownership and clear color *(review: fix now)*

- [x] Split `RHI` into **Device** (created once) and **Swapchain** (per window: back buffers, RTVs, resize). The empty `swapchain.h/.cpp` are its home.
- [ ] **Renderer owns the frame loop** (begin/end frame, resize forwarding). `Window` shrinks to native handle + events and stops including `rhi_dx12`, `renderer` and ImGui.
- [ ] **Frames in flight**: one command allocator and fence value per back buffer; wait only for the frame being reused.
- [ ] Move content out of the engine *(review)*: triangle geometry and PSO choice go to Sandbox through the renderer API; the ImGui panel goes to Sandbox.
- [ ] Remove the unused second `Shader` in `rhi_dx12/shader.h`.

**Exit:** Sandbox clears and draws the triangle through `renderer`; resize works; CPU and GPU overlap; clean debug-layer live-object report.

## M2 — Backend-neutral public API

- [ ] Public headers contain no `windows.h`, DX12/DXGI or ImGui backend headers. Backend headers become private (`src/tessera/src/rhi_dx12/`). *(review: native objects leak)*
- [ ] RHI contracts with typed handles; backend chosen at renderer creation; capability query for backend differences.
- [ ] Validation/debug layer moves into the backend and is enabled through renderer settings; Sandbox stops calling `DXDebugLayer`.
- [ ] PCH = standard library + `core` only; Win32 and ImGui are included where used. *(review: PCH as dependency channel)*
- [ ] The engine owns and builds its shaders into its own output; Sandbox no longer compiles them. *(review)*
- [ ] `dev_ui` becomes an optional module; the DX12 ImGui binding moves behind the backend. Each app opts in with a premake build option (off by default, any config); Sandbox opts in. `core/config.h` stops rejecting `TESSERA_DEV_UI` in RELEASE.
- [ ] Optional `Application` layer on top of the library API.

**Exit:** Sandbox includes only `<tessera/...>` public headers and contains no backend type; the backend is one setting (only DX12 exists yet).

## M3 — Multi-window

- [ ] One device, N swapchains; window registry with independent create, close and resize.
- [ ] Renderer targets a chosen window; frames in flight per swapchain.
- [ ] Per-window user data instead of the static HWND map; `FRAME_COUNT` moves into the backend. *(review: process-wide state)*
- [ ] ImGui multi-viewport windows go through the same path.

**Exit:** Sandbox opens two windows with different content; closing one leaves the other running; clean shutdown.

## M4 — Vulkan backend (Windows first)

Done before adding features, because a second backend is the only real test of the RHI.

- [ ] `rhi_vulkan` module; validation layers in DEBUG/STAGING. New dependencies (loader, memory allocator) need approval first.
- [ ] DXC compiles the same HLSL to SPIR-V.
- [ ] Vulkan ImGui binding for `dev_ui`.
- [ ] Document backend differences found along the way, and expose them through capabilities.

**Exit:** Sandbox (M1–M3 scope) runs the same on DX12 and Vulkan, with clean validation on both.

## M5 — Graphics pipeline

Replaces today's hard-coded, single-purpose pipeline with a general one that both backends implement. It comes after M4, so the design fits DX12 and Vulkan instead of only DX12. M1 already handles the frame flow (allocators, fences, frames in flight); this milestone covers everything recorded inside a frame.

- [ ] **Resources:** buffers and textures created from descriptions; a real upload path (per-frame staging/upload memory instead of one-off upload buffers); deferred destruction once the GPU is done with a frame.
- [ ] **Binding model:** decide between descriptor tables and bindless; engine-owned descriptor heap allocation; one shared root signature / pipeline layout.
- [ ] **Pipelines:** PSOs built from descriptions (shaders, vertex layout, blend, depth, raster state), with a cache; never created per frame. Shader reflection drives the bindings instead of hand-written root signatures.
- [ ] **Synchronization:** resource state tracking with explicit, batched barriers.
- [ ] **Render passes:** color and depth targets, clear/load/store, resize-aware; offscreen targets (needed by the editor viewport).
- [ ] **Shader workflow:** HLSL compiled for both backends as part of the engine build; optional shader hot reload in DEBUG.
- [ ] **Measuring:** GPU timestamps and debug markers (PIX/RenderDoc) per pass.

**Exit:** Sandbox draws a textured, depth-tested mesh through pipelines and passes built from descriptions, on both backends. No per-frame PSO/heap creation or allocation; validation is clean; GPU time per pass is visible.

## M6 — 3D rendering for code-only use

- [ ] Meshes and model loading (format choice and dependency need approval), camera, transforms.
- [ ] Materials and basic lighting.
- [ ] 3D debug drawing (lines, boxes, grids).
- [ ] Platform: input and timing.

**Exit:** a Sandbox demo with a lit, textured 3D scene and a moving camera, on both backends.

## M7 — Editor

- [ ] New `Editor` project on `Tessera` + `dev_ui`: docking layout, scene viewport rendered to a texture.
- [ ] Scene and entities (ECS choice is a separate decision), inspector, scene serialization, asset browser, play/stop.

**Exit:** create, save and reload a scene in the editor, rendered in the viewport.

## M8 — Scripting (Luau)

- [ ] Embed Luau; binding layer over the public API; script components.
- [ ] Hot reload; script errors in the editor console; type definitions for luau-lsp.

**Exit:** entity behaviour written in Luau and reloaded live in the editor.

## Later

Linux through Vulkan, a frame graph, a standalone runtime player and packaging, a profiler integration, and a unit-test project for `core`.
