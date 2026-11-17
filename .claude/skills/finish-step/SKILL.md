---
name: finish-step
description: Verify and wrap up the current change - builds, layering check, docs/status.md update, commit message draft.
disable-model-invocation: true
argument-hint: "[step being finished]"
---

Wrap up the current working-tree change. Do not commit unless the user asks.

1. `git status` and `git diff` (staged and unstaged). Summarize what changed in 3–6 bullets. Ignore `vendor/` and generated folders.
2. Build:
   - `./build.ps1 build DEBUG` always.
   - `./build.ps1 build RELEASE` if the diff touches premake files, `core/config.h`, ImGui code or `TESSERA_DEV_UI` guards.
   - Report errors and new warnings in changed lines. Do not fix code here unless the user asks.
3. Review the diff against the hard rules in `CLAUDE.md`:
   - Shared code (`core`, `platform`, `renderer`, `rhi`, sandbox) gained no DX12/DXGI/Vulkan includes or native types.
   - New ImGui code is inside `#if TESSERA_DEV_UI`.
   - New DX12 objects follow `.claude/rules/rhi_dx12.md` (ownership, fence waits before release, no per-frame creation).
   List violations with `file:line`; don't silently fix them.
4. Update `docs/status.md`: move the finished step (`$ARGUMENTS`, or the one the diff completes) to "Done" in one line; add any new known issue or decision you found. Show the user the change.
5. Improvements: add an entry to `docs/improvements.md` for anything in the diff that works but isn't production quality yet (don't fix it). Remove entries the diff resolved. List the added/removed entries in one line each.
6. Tell the user what to check in `./build.ps1 run DEBUG` (and STAGING if relevant), including the debug-layer live-object report on exit.
7. Draft a conventional commit subject (and a short body if needed), e.g. `rhi: add per-frame command allocators`.
