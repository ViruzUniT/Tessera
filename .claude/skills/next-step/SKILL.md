---
name: next-step
description: Plan the next step from docs/status.md (or the step given as argument) before any code is written.
disable-model-invocation: true
argument-hint: "[step number or description]"
---

Plan one step of Tessera work. Do not edit files in this skill.

1. Read `docs/status.md`. Pick the step in `$ARGUMENTS`; if empty, the first item under "Next steps". Say which step you picked.
2. Read only the docs and code the step touches (`docs/architecture.md` only if it crosses module boundaries or RHI contracts; `docs/roadmap.md` for the milestone's exit criteria).
3. Check "Known issues" in `docs/status.md` for anything that blocks or overlaps the step, and `docs/improvements.md` for open entries in the same area. Mention at most 3 relevant ones, one line each, as optional ("while you're there, you could…").
4. Output, briefly:
   - **Goal** — one sentence; what is observably different when done.
   - **Changes** — per file: what changes and why. Mark new files.
   - **Contracts** — any change to `rhi`/`renderer` public headers, and how it stays backend-neutral.
   - **Risks** — lifetime/synchronization hazards (GPU in-flight resources, resize, shutdown order), layering, RELEASE impact.
   - **Verify** — build configs to run and what the user should look for in `run DEBUG` (window output, debug-layer messages, clean live-object report on exit).
   - **Split** — if larger than one reviewable diff, propose smaller sub-steps.
5. End by asking: does the user want to implement it themselves (you then guide), or have you implement it?
