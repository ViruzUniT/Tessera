# Build

Premake5 generates the project files, and `build.ps1` wraps the common commands. The wrapper is going away in favour of premake commands (roadmap M0); update this file when that happens.

## Requirements (Windows)

- Windows 10/11 with the Windows SDK (DX12 headers and libraries).
- Clang (llvm-mingw) as the primary compiler; keep MSVC 2022 compatible. No compiler-specific behaviour unless it's isolated and documented.
- GNU Make for the default `gmake` generator.
- `premake5` (5.0.0-beta8) on `PATH`.
- `compiledb` (`pip install compiledb`) for the clangd database.
- Submodules: `git submodule update --init --recursive` (Dear ImGui in `vendor/imgui`).

## Commands

```powershell
./build.ps1 gen [gmake|vs2022|vs2026]        # default gmake
./build.ps1 build [DEBUG|STAGING|RELEASE]    # default DEBUG
./build.ps1 run [DEBUG|STAGING|RELEASE]      # builds, then runs Sandbox
./build.ps1 compdb [DEBUG|STAGING|RELEASE]   # compile_commands.json for clangd
./build.ps1 clean                            # removes build/, bin/, bin-int/
```

`build.sh` is an empty placeholder.

## Configurations

Configurations are `DEBUG`, `STAGING` and `RELEASE`; what each one enables is in `docs/architecture.md`. The defines are `TESSERA_DEBUG`/`TESSERA_STAGING`/`TESSERA_RELEASE`, plus `TESSERA_DEV_UI` in DEBUG and STAGING.

## Outputs

- `build/`: generated projects (`Tessera.make`, `Sandbox.make`, `Makefile` or `.sln`).
- `bin/<CONFIG>-windows-x86_64/<Project>/` and `bin-int/...`: binaries and intermediates.
- `compile_commands.json` at the repo root, generated from a GNU Make dry run.

None of these are committed.

## Shaders and DXC

`gen` and `build` download the pinned DXC into `.tools/dxc/` when it's missing, and check the archive's SHA-256 before extracting it. Shader build steps use that copy, so DXC doesn't need to be installed or on `PATH`. HLSL lives in `src/tessera/shaders/` and is compiled to DXIL during the build; it is currently driven by Sandbox's premake file, and moves to the engine in M2.

## CI

`.github/workflows/` builds DEBUG and RELEASE with Clang + Make on pull requests, through `build.ps1`.
