# Roadmap

Milestones in recommended order. Each one ends with something you can run and check. The current milestone and its next steps are in `docs/status.md`.

Items tagged *(review)* come from the architecture review in the "Ngin Architecture Report" (written before the rename).

## M0 — Foundation and build

- [x] Rename the project to Tessera (namespace, include paths, macros, premake projects, docs). Sandbox keeps its name.
- [x] Replace `build.ps1`/`build.sh` with premake commands: `premake5 gen|build|run|compdb|clean` (`premake/actions.lua`), DXC bootstrap in Lua (`premake/dxc.lua`); CI, hooks and docs updated.
- [ ] Sanitizers in DEBUG only (they are currently in every config, including RELEASE).
- [ ] Fix `Error::message`: it is a `string_view` filled from `std::format` temporaries, so it dangles. Own the string, or keep messages static.

**Exit:** a fresh clone builds and runs with `premake5` commands only; no wrapper scripts left.

## M1 — Frame ownership and clear color *(review: fix now)*

- [ ] Split `RHI` into **Device** (created once) and **Swapchain** (per window: back buffers, RTVs, resize). The empty `swapchain.h/.cpp` are its home.
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
- [ ] `dev_ui` becomes an optional module; the DX12 ImGui binding moves behind the backend.
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

## M5 — Core rendering for code-only use

- [ ] Buffers and textures with upload paths; descriptor/binding model decision; pipelines from descriptions, with a cache.
- [ ] 2D: batched quads, sprites/atlases, text, debug draw.
- [ ] 3D: meshes, camera/transforms, depth, basic materials.
- [ ] Platform: input and timing.

**Exit:** a Sandbox demo with a 2D scene and a 3D mesh, on both backends.

## M6 — Editor

- [ ] New `Editor` project on `Tessera` + `dev_ui`: docking layout, scene viewport rendered to a texture.
- [ ] Scene and entities (ECS choice is a separate decision), inspector, scene serialization, asset browser, play/stop.

**Exit:** create, save and reload a scene in the editor, rendered in the viewport.

## M7 — Scripting (Luau)

- [ ] Embed Luau; binding layer over the public API; script components.
- [ ] Hot reload; script errors in the editor console; type definitions for luau-lsp.

**Exit:** entity behaviour written in Luau and reloaded live in the editor.

## Later

Linux through Vulkan, a frame graph, a standalone runtime player and packaging, a profiler integration, and a unit-test project for `core`.
