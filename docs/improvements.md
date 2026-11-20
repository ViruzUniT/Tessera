# Improvements

Things that work but aren't production quality yet, or have a better approach. Nothing here blocks progress. The milestones in `docs/roadmap.md` are the plan; this list is "fix when you're in the area, or when it starts to hurt".

Format: `- [area] what's suboptimal → better approach (why it matters). Noted YYYY-MM-DD.` Remove an entry when it's done or rejected; rejected entries become a line in `docs/status.md` Decisions.

## Open

- [core] `Error` codes such as `InvalidArgumentIGuess` and `CouldBeAProblem` are vague, and `operator bool` logs as a side effect → give each code one clear meaning, and keep logging out of the bool conversion (users of the lib need to know exactly what failed, and a check shouldn't print). Noted 2026-11-17.
- [build] `premake5 build` falls back to a hard-coded `C:\mingw64\bin` when `g++` isn't on `PATH` (carried over from `build.ps1`) → detect the toolchain or document it as a requirement (other machines and CI won't have that path). Noted 2026-11-17.
- [build] Both project premake files repeat the same warning flags (`-Wunused-result` twice) and config blocks → move shared compiler/config settings into one `premake/common.lua` (one place to change flags, and fewer filter mistakes). Noted 2026-11-17.
- [rhi_dx12] `Device` stores its swapchains as `Ref<Swapchain>` (and `Swapchain::Create` hands out a `Ref`) although the device is the only owner → `Scope<Swapchain>` (ownership is explicit; nobody can keep a swapchain alive past its device). Noted 2026-11-20.
- [renderer] The `Renderer` constructor calls `std::exit(1)` when device or swapchain creation fails, and the swapchain message logs the device's error code → return the `Error` through `Renderer::Create` and let the app decide (a library must not end the user's process; and the message should carry the swapchain's own error). Noted 2026-11-20.
- [platform] The app reads `window.hasResized`/`window.dimensions` directly and resets the flag itself → a `ConsumeResize()`-style accessor that returns the new size and clears the flag (one call, can't forget the reset, fields can become private). Noted 2026-11-20.
- [rhi_dx12] The SRV heap uses `ExampleDescriptorHeapAllocator`, copied from the ImGui sample → replace it with an engine descriptor allocator when the binding model is designed in M5 (graphics pipeline) (the sample is fixed-size and ImGui-specific). Noted 2026-11-17.
