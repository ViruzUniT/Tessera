# CLAUDE.md

Tessera: C++23 engine. Code-only library first (+ optional ImGui `dev_ui`, Sandbox), editor and Luau scripting later. DX12 and Vulkan behind one backend-neutral API; multi-window by design.

## Docs (read when relevant, not by default)

- `docs/status.md` — current milestone, next steps, decisions, known issues. Read before planning new work.
- `docs/roadmap.md` — milestones M0–M7 in order, with exit criteria.
- `docs/architecture.md` — products, layers, backend policy, multi-window and app model, folder layout.
- `docs/build.md` — toolchain, commands, configs, DXC/shaders, CI.

## Hard rules

- Dependencies flow app/tools → `renderer` → `rhi` → backend (`rhi_dx12`). Never upward.
- Shared code (`core`, `platform`, `renderer`, `rhi`, app) never includes DX12/DXGI/Vulkan headers or uses native objects.
- ImGui code is guarded by `#if TESSERA_DEV_UI`; `RELEASE` contains no ImGui, debug panels or dev menus.
- Never edit `vendor/`, add dependencies, or commit generated output (`build/`, `bin/`, `bin-int/`, `compile_commands.json`) without asking.

## Working mode

- Default is mentor mode: do not edit files or show code snippets unless asked. When troubleshooting, explain cause and guidance in plain language first.
- When asked to implement: for changes spanning modules or RHI contracts, propose a short plan first and wait for approval.
- Small, scoped diffs. No drive-by refactors or reformatting of untouched code.
- When a next step in `docs/status.md` is completed, update that file in the same change.

## Commands (Windows)

```powershell
./build.ps1 build [DEBUG|STAGING|RELEASE]   # default DEBUG
./build.ps1 run   [DEBUG|STAGING|RELEASE]   # builds, then runs Sandbox
```

Everything else (`gen`, `compdb`, `clean`): `docs/build.md`.

## Before saying "done"

No test framework yet; the vertical slice is the test.

1. `./build.ps1 build DEBUG` — no new warnings.
2. `./build.ps1 build RELEASE` when touching `TESSERA_DEV_UI` guards, config logic, or premake files.
3. For runtime changes, tell the user what to check in `run DEBUG` / `run STAGING` (Claude cannot see the window).
4. On failure, paste the relevant compiler / debug-layer output.

## Style

- `.clang-format` is the source of truth for layout and column limit.
- Everything in namespace `Tessera`. Include public headers as `<tessera/<module>/...>`.
- Lowercase folder names with underscores.

## Performance

Back performance claims with a measurement (PIX/RenderDoc capture or timing), not intuition.

## Commits & PRs

Short conventional subjects (`docs: add struct docs`), imperative and scoped. PRs list changes, validation commands, linked issues, and renderer-visible captures.
