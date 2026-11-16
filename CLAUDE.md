# CLAUDE.md

Project rules, architecture, build commands and current status live in AGENTS.md (single source of truth — update it there, not here):

@AGENTS.md

## Working mode

- Default is **mentor mode** (see Collaboration Rules in AGENTS.md): explain causes, trade-offs and next steps in plain language; do not edit code or show snippets unless explicitly asked.
- When asked to implement: for anything touching more than one module or the RHI contracts, propose a short plan first and wait for approval.
- Keep diffs small and scoped to the request. No drive-by refactors, renames or formatting of untouched code.
- Never edit `vendor/`. Never add a dependency without asking.

## Verification (required before saying "done")

1. `./build.ps1 build DEBUG` compiles with no new warnings.
2. `./build.ps1 build RELEASE` when touching `NGIN_DEV_UI` guards, config logic, or premake files.
3. If the change affects runtime behaviour, say what the user should check when running `./build.ps1 run DEBUG` (Claude cannot see the window) — e.g. clear color, resize, shutdown without D3D12 debug-layer leak reports.
4. Report results honestly: paste the relevant compiler/debug-layer output on failure.

## Engine-specific review checklist

When writing or reviewing renderer/RHI code, check explicitly:

- Layering: no `d3d12.h`/`dxgi*.h` or native handles outside `rhi_dx12` (and backend-specific ImGui binding).
- Lifetime: COM objects owned via `ComScope`; GPU resources not released while in flight (fence-protected); clean debug-layer report on exit.
- Frame flow: fence wait → allocator reset → list reset → record → close → execute → present → signal; correct resource barriers/states.
- Per-frame paths: no heap allocations, no per-frame PSO/root-signature creation, no blocking waits beyond frame pacing.
- Performance claims must be backed by a measurement (PIX/RenderDoc capture or timing), not intuition.
