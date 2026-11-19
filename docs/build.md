# Build

Everything goes through `premake5`. Besides its built-in generators, the workspace defines custom actions in `premake/actions.lua`, and `premake/dxc.lua` handles the pinned shader compiler. `premake5 --help` lists them.

## Requirements (Windows)

- Windows 10/11 with the Windows SDK (DX12 headers and libraries).
- Clang (llvm-mingw) as the primary compiler; keep MSVC 2022 compatible. No compiler-specific behaviour unless it's isolated and documented.
- GNU Make for the default `gmake` generator.
- `premake5` (5.0.0-beta8) on `PATH`.
- `compiledb` (`pip install compiledb`) for the clangd database.
- Git, for the submodules (Dear ImGui in `vendor/imgui`); `gen` initializes them.

## Commands

Run from the repo root. Configuration names are case-insensitive.

```powershell
premake5 gen [gmake|vs2022|vs2026]        # init submodules, fetch DXC, generate into build/ (default gmake)
premake5 build [DEBUG|STAGING|RELEASE]    # default DEBUG; make for gmake, msbuild for Visual Studio
premake5 run [DEBUG|STAGING|RELEASE]      # builds, then runs Sandbox; output also written to run.log
premake5 compdb [DEBUG|STAGING|RELEASE]   # compile_commands.json for clangd (gmake files + compiledb)
premake5 clean                            # removes build/, bin/, bin-int/, compile_commands.json
```

- Native generator actions (`premake5 gmake`, `premake5 vs2022`) still work and also fetch DXC; they skip the submodule init.
- `run` writes everything it prints (build output and Sandbox's stdout/stderr) to `run.log` in the repo root as well as the console. The file is overwritten on every `run` and ignored by git (`*.log`). Other commands don't touch it. Debug-layer messages sent to the debugger (`OutputDebugString`) don't appear there.
- `clean` replaces premake's built-in `clean`.
- make doesn't rebuild when only compiler flags change; after editing flags in a premake file, run `premake5 clean` and `premake5 gen` first.
- `build` adds `C:\mingw64\bin` to `PATH` if `g++` isn't found and that folder exists, and runs make with `SHELL=cmd.exe` on Windows.
- `vs2022`/`vs2026` builds need `msbuild` on `PATH` (Developer PowerShell).
- Failures exit non-zero, so CI and hooks can rely on the exit code.

## Configurations

Configurations are `DEBUG`, `STAGING` and `RELEASE`; what each one enables is in `docs/architecture.md`. The defines are `TESSERA_DEBUG`/`TESSERA_STAGING`/`TESSERA_RELEASE`, plus `TESSERA_DEV_UI` in DEBUG and STAGING.

AddressSanitizer and UndefinedBehaviorSanitizer (`-fsanitize=undefined,address`) are on in DEBUG and STAGING with Clang/GCC: the compile flag goes on both projects, and the link flag only on executables. RELEASE has no sanitizers. ASan needs `libclang_rt.asan_dynamic-x86_64.dll` from the llvm-mingw toolchain on `PATH` at runtime.

## Outputs

- `build/`: generated projects (`Tessera.make`, `Sandbox.make`, `Makefile` or `.sln`).
- `bin/<CONFIG>-windows-x86_64/<Project>/` and `bin-int/...`: binaries and intermediates.
- `compile_commands.json` at the repo root, generated from a GNU Make dry run.

None of these are committed.

## Shaders and DXC

Generating and building download the pinned DXC into `.tools/dxc/` when it's missing (`premake/dxc.lua`: version, URL and SHA-256 in one place). The archive's SHA-256 is checked with `certutil` before extracting, since premake has no SHA-256 function, and nothing is left behind on failure. Shader build steps use that copy, so DXC doesn't need to be installed or on `PATH`. HLSL lives in `src/tessera/shaders/` and is compiled to DXIL during the build; it is currently driven by Sandbox's premake file, and moves to the engine in M2.

## CI

`.github/workflows/` builds DEBUG and RELEASE with Clang + Make on pull requests, through `premake5 gen` and `premake5 build`. Both workflows install a pinned llvm-mingw (release `20260505`, LLVM 22.1.5, UCRT; SHA-256 checked, cached) so CI uses the same compiler and standard library as local builds. Don't use the runner's preinstalled LLVM: it targets the MSVC STL, which fails when the image's Visual Studio expects a newer Clang. To upgrade, change the release name and hash in both workflow files.
