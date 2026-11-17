---
name: dx12-object
description: Checklist for adding or changing a native DX12 object in Tessera's rhi_dx12 backend (device children, heaps, resources, command allocators/lists, fences, PSOs, root signatures, swapchain buffers). Use when implementing, planning or reviewing such a change.
---

Apply in addition to `.claude/rules/rhi_dx12.md`. When guiding (mentor mode), walk the user through these points instead of writing the code.

## Placement

- Declaration and native types stay in `src/tessera/include/tessera/rhi_dx12/` and `src/tessera/src/rhi_dx12/`. If `renderer` or the app needs it, expose a backend-neutral handle or description through `rhi/`, never the native pointer.
- Decide the lifetime class up front: **device-lifetime** (PSO, root signature, heaps), **swapchain-lifetime** (backbuffers, RTVs — recreated on resize), or **per-frame** (allocators, upload space — one per frame in flight, indexed by frame).

## Ownership and order

- Own it with `ComScope<T>` (or `Scope<T>` for Tessera wrappers). No raw `Release()`.
- Members are destroyed in reverse declaration order: declare an object after everything it depends on (e.g. after the device). Check `RHI::~RHI()` for explicit resets that must stay ordered (fence first, then buffers).
- GPU work referencing the object must complete before it is released or reset: flush/wait on the fence.

## Creation

- Follow the existing pattern: a private `Create...()` returning `Error`, called from the constructor, which returns early on failure.
- `Error::message` is a `std::string_view`: pass string literals or storage that outlives the `Error`. A `std::format(...)` result is a temporary and will dangle.
- Check every `HRESULT`; include the value in the message.
- In DEBUG, `SetName` the object so debug-layer reports identify it.

## Frame and resize

- Per-frame objects: wait on that frame's fence value before reset/reuse; never create in the frame loop.
- Swapchain-lifetime objects: on `Resize`, flush the GPU, release every reference (including ImGui and RTV handles), `ResizeBuffers`, then recreate.
- State transitions are explicit barriers, batched when possible.

## Done when

- `premake5 build DEBUG` is clean; `run DEBUG` shows no debug-layer errors, survives a window resize, and the live-object report on exit is clean.
- `docs/status.md` updated if this completes a step.
