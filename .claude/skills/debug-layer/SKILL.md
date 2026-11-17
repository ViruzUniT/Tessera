---
name: debug-layer
description: Diagnose D3D12/DXGI debug-layer messages, live-object reports on exit, device removal (DXGI_ERROR_DEVICE_REMOVED/HUNG), GPU hangs or renderer crashes in Tessera. Use when the user pastes debug-layer or validation output, or the app crashes, hangs or shows nothing in the renderer.
---

Mentor mode applies: explain cause and fix in plain language first; no code edits or snippets unless asked.

## Where output comes from

- `src/tessera/src/core/DXDebugLayer.cpp` enables the D3D12 debug layer and an `ID3D12InfoQueue1` callback (runtime messages go through Tessera's logger), and calls `ReportLiveObjects` at shutdown (captured from debugger output).
- `DEBUG` and `STAGING` have diagnostics; `RELEASE` does not. If the issue only happens in RELEASE, suspect uninitialized state or timing, not validation.

## Procedure

1. Ask for the **first** error, not the last — later messages are usually consequences. Ask for the full message including its ID (e.g. `[ EXECUTION ERROR #...: ... ]`) if it's missing.
2. Classify by the message's category and map it to the object and the code that creates or uses it (`src/tessera/src/rhi_dx12/`).
3. Read only that code, and the frame flow (`BeginFrame`/`EndFrame`/`Present`/`Resize`) if timing is involved.
4. Explain: what the runtime detected → why it happens in this code → what to change → how to confirm it's gone.

## Common causes

| Symptom | Usual cause |
|---|---|
| Live objects at exit (refcount > 0) | Owner not reset, wrong destruction order (members destroy in reverse declaration order), ImGui backend still holding device/heap, or a raw `AddRef` without matching release |
| `Resource state mismatch` / `INVALID_SUBRESOURCE_STATE` | Missing or wrong barrier (e.g. backbuffer not PRESENT→RENDER_TARGET→PRESENT) |
| `COMMAND_ALLOCATOR_SYNC` / reset while in use | Allocator or command list reset before its fence value completed |
| Object deleted while still in use | Releasing a resource the GPU still references; wait on the fence first |
| `ResizeBuffers` fails / `DXGI_ERROR_INVALID_CALL` on resize | Backbuffer references (RTVs, ImGui, `buffers[]`) still held; GPU not flushed first |
| Device removed / hung | Out-of-bounds descriptor or root parameter, invalid PSO/root signature combination, infinite shader loop. Call `GetDeviceRemovedReason`; enable GPU-based validation or DRED for detail |
| Blank window, no errors | Clear/draw recorded but list not executed, wrong RTV handle for `currentBuffer`, viewport/scissor zero-sized, or present skipped |

## Tips to suggest when stuck

- Name objects with `SetName` in DEBUG so reports show which object leaks.
- Turn on GPU-based validation or DRED temporarily for device-removed cases.
- Capture a frame with PIX or RenderDoc to see actual resource states and bindings.
