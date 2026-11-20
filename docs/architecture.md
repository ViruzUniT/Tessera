# Architecture

Tessera is a **3D-only** C++23 engine built in two parts: a **code-only engine library** first, and an **editor** on top of it later. It renders through DirectX 12 and Vulkan behind one backend-neutral API, and supports several windows from the start.

## Products

| Product | What it is | When |
|---|---|---|
| `Tessera` | Static library: core, platform, renderer, RHI, backends, and Dear ImGui (always compiled in; apps use it or not). Everything user code needs. | Now |
| `Sandbox` | Sample and test app that uses only the public API. Not renamed; it's a test harness. | Now |
| `Editor` | ImGui docking editor built on `Tessera`. | M7 |
| Scripting | Luau scripts driving entities in the editor and runtime. | M8 |

Milestones: `docs/roadmap.md`.

## Goals

- **Production quality for library users.** The public API, its documentation, error reporting and code quality aim at a level other people could ship with: clear ownership, no surprising behaviour, failures that say what went wrong, nothing undefined.
- **Production-grade features and performance, reached by iterating.** That's the target, not the bar for every step. A simpler version that works is fine first; known shortcomings and better approaches go in `docs/improvements.md` and get revisited, not forced in up front.
- **Backend-independent user code.** Apps include only Tessera's public headers and never see DX12/Vulkan types, `windows.h` or ImGui backends. The backend is a startup option.
- **Honest feature differences.** Where one backend lacks a feature, the API reports it through a capability query (`device.Supports(Feature::...)`). It doesn't emulate the feature silently or hide it.
- **Multi-window by design.** One device, any number of windows, each with its own swapchain. ImGui multi-viewport windows use the same path.
- **Performance stays visible.** The RHI doesn't hide concepts (frames in flight, barriers, descriptors) when hiding them would make both backends worse.

## Layers and dependency direction

```txt
App / Sandbox / Editor
  -> renderer                  incl. ImGui context/frame
    -> rhi                     backend-neutral contracts
      -> rhi_dx12 | rhi_vulkan  native objects, private
platform, core: usable by every layer above
```

| Module | Owns | Must not |
|---|---|---|
| `core` | Types, `Error`, logging, asserts, config checks, memory helpers. | Depend on any other module or OS/graphics header. |
| `platform` | OS windows, events, input, timing, files. Window = native handle + events. | Know about rendering, RHI or ImGui. |
| `rhi` | Contracts: device, swapchain, queues, command lists, buffers, textures, descriptors, pipelines, sync, capabilities. Typed handles. | Include native graphics headers. |
| `rhi_dx12`, `rhi_vulkan` | Native objects, lifetime, synchronization, translation of descriptions, ImGui renderer binding, debug/validation layers. | Leak through public headers. |
| `renderer` | Engine-facing 3D API: frame loop, frames in flight, render targets per window, passes, meshes, materials; the ImGui context and frame (through the backend's ImGui binding). | Name a specific backend. |

## App model

The **library core** works without owning `main()`. The user creates windows and a renderer, and drives the loop themselves.

An optional **`Application`** layer (`OnInit`/`OnUpdate`/`OnRender`, loop and event dispatch) sits on top of the same public API. The editor and scripted runtime use it later, and simple apps can too.

## Backend policy

**Shared** (written once): resource/pipeline/shader descriptions, material and shader metadata, frame scheduling concepts, 3D submission, ImGui integration.

**Per backend**: device and swapchain creation, allocation, descriptor model, command recording, synchronization, pipeline objects, shader bytecode loading, ImGui renderer binding, validation layer setup.

**Shaders**: HLSL is the single source. DXC compiles it offline to DXIL (DX12) and SPIR-V (Vulkan). The pinned DXC is fetched by the build. The engine owns and ships its own shaders.

**Selecting a backend**: chosen when the renderer is created (e.g. `RendererDesc::backend = Backend::DX12 | Vulkan | Auto`). Code that only exists for one backend is compiled only where that backend is available.

## Multi-window model

- One device per process; windows are created and destroyed independently.
- Each window gets a swapchain (back buffers, views, resize) created by the device.
- The renderer records per window/target; frames in flight are per swapchain.
- Window lookup uses per-window user data, not a global map.

## Configurations

| Config | Optimized | Diagnostics, validation |
|---|---|---|
| `DEBUG` | no | yes |
| `STAGING` | yes | yes |
| `RELEASE` | yes | no |

`core/config.h` turns invalid combinations into build errors.

**ImGui is not tied to a config.** It's compiled into the library in every config, RELEASE included; the editor uses it, and a game uses it or not.

## Principles to keep

These come from the architecture review and stay as the code grows:

- **Single owner per GPU object** (`ComScope`, `Scope`), with a visible release point; GPU work completes before release.
- **Error values, no exceptions**: failures surface at the call that caused them.
- **Compile-time config checks** in `core/config.h`.
- **Offline, pinned shader compiler**: reproducible builds, no runtime compiler.
- **Linear, explicit creation**: construction reads top to bottom in dependency order.

## Folder layout

Lowercase folders with underscores. Public headers and implementation live in parallel trees.

```txt
src/
  tessera/
    include/tessera/<module>/   public headers, included as <tessera/<module>/...>
    src/<module>/               implementations (backend headers move here in M2)
    shaders/                    HLSL sources
  sandbox/                      sample app
  editor/                       later (M7)
vendor/                         third-party submodules (imgui, ...)
build/                          generated project files
bin/, bin-int/                  build outputs
```

Module names in docs (`renderer`, `rhi_dx12`) are the folder names.

## Rendering scope

Tessera renders **3D only**. There is no 2D renderer (no sprites, batched quads or 2D text); on-screen UI is ImGui (the editor, and games that use it).

- **Pipeline (M5)**: resources from descriptions with a real upload path, an engine-owned binding model, PSOs from descriptions + shader reflection with a cache, tracked resource states and batched barriers, render passes with depth and offscreen targets, GPU timing per pass.
- **3D (M6)**: meshes and model loading, camera/transforms, materials, basic lighting, 3D debug drawing. A frame graph comes later if passes outgrow manual ordering.
