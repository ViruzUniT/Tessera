---
paths:
  - "src/tessera/include/tessera/rhi_dx12/**"
  - "src/tessera/src/rhi_dx12/**"
  - "src/tessera/shaders/**"
---

# DX12 backend rules

- Own COM objects via `ComScope`; never raw `Release()` calls.
- Never release a GPU resource that may still be in flight — wait on its frame fence first.
- Frame order: wait fence → reset allocator → reset list (with PSO) → record → close → execute → present → signal.
- Every resource state transition is an explicit barrier; batch barriers where possible.
- No per-frame creation of PSOs, root signatures, or descriptor heaps; no heap allocations in per-frame paths.
- Exit must produce a clean D3D12/DXGI debug-layer live-object report in `DEBUG`.
- Nothing from this module's headers may leak into `renderer`, `rhi`, or app code.
