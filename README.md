# Tessera

A 3D-only C++23 engine with DirectX 12 and Vulkan behind one backend-neutral API, designed for multiple windows from the start.

- **Now:** a code-only engine library, with Dear ImGui built in (use it or not), and the Sandbox sample app.
- **Later:** an editor and Luau scripting.

Status: early. DX12 renders a triangle with ImGui in one window. See [status](docs/status.md) and [roadmap](docs/roadmap.md).

## Quick start (Windows)

```powershell
premake5 gen          # inits submodules, downloads DXC, generates gmake files
premake5 run          # DEBUG by default
```

Requirements and all commands: [docs/build.md](docs/build.md).

## Docs

- [Architecture](docs/architecture.md): products, layers, backend policy, multi-window and app model.
- [Roadmap](docs/roadmap.md): milestones in order.
- [Status](docs/status.md): current state, next steps, decisions, known issues.
- [Build](docs/build.md): toolchain, commands, configurations, shaders, CI.
- [Improvements](docs/improvements.md): known shortcomings and better approaches, revisited over time.
