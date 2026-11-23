# Live completion audit

2026-11-24. Source inventory, not acceptance of new work. The separate original
design is read-only and contains superseded details; do not blindly implement it.

## Confirmed next priorities

1. **Common primitive pixel parity is not actually selected by -Renderer dx12.**
   The existing Vulkan pixel helper calls create_vulkan_renderer directly, and
   tests/renderer depends on renderer-vulkan. Setting CGPUI_RENDERER does not
   change a named factory. Independent DX12 tests cover many analogous cases,
   but are not evidence that the exact common 13 scene/oracle pairs passed.
   Next scope should make the shared pixel tests explicitly backend-selectable,
   check actual backend_info, preserve backend-specific Vulkan tests, and run both.
2. **True Runtime recovery acceptance is missing, not the retry mechanism.**
   runtime_frame_retry.cpp, runtime_rendering.cpp and runtime_window_rendering.cpp
   already consume frame_retry_required and request another render for root and
   child windows. window_runtime_frame_retry_test covers begin/present x
   root/child x retry/fatal using a test Renderer. Need actual Win32/DX12
   RemoveDevice -> typed retry -> scheduled redraw -> recovered capture/generation,
   without factory recreation or faking successful presentation.
3. **Actual DXGI pacing wait is missing.** dx12_swapchain.cpp requests the
   waitable-object flag and maximum latency 1, but no source calls
   GetFrameLatencyWaitableObject. dx12_presentation waits GPU fences only.
   A future pacing owner must not treat a latency signal as GPU completion proof.
4. **DX12 cross-thread/busy cohort recovery remains bounded/refusing.** Same
   adapter+HWND-thread peers can retire. Foreign-thread and busy peers are skipped;
   later explicit owner progress is needed. Existing tests require terminal error
   while the blocker remains, not automatic cross-thread recovery.
5. **Original recovery diagnostic additions are absent.** No DRED setup/capture,
   and no retained_image_byte_count in RendererFrameDiagnosticSnapshot. Define
   memory ownership precisely (cache vs all old-frame snapshot refs) before adding
   a field. Current InfoQueue debug checks and tight upload bytes are not substitutes.
6. **Vulkan device-loss parity is not implemented.** presentation_recovery repairs
   synchronization and returns the error; no VK_ERROR_DEVICE_LOST reconstruction
   branch. Native image objects have no CPU pixels after staging retirement;
   backend generation stays 1. This is separate work, not a DX12 retry-hook tweak.
7. **Blur unsupported behavior differs.** Public RenderFrame promises tint-only
   fallback; Vulkan uses it. DX12 currently errors when actual blur capability is
   false. Determine and consistently implement the shared contract rather than
   describing it as verified parity.
8. **No dedicated reproducible WARP CI entry exists.** Current workflow/CI source
   contains no CGPUI_DX12/WARP selection. Local WARP logs are not hosted-CI proof.

Window compatibility proposals remain under read-only review. Do not rerun the
already negative plain HWND/DWM or client-HMENU caption-band recipes unchanged.
The all-composition-from-start experiment is not yet a default implementation
or proof of menu/chrome/backdrop/rollback/opaque-clear compatibility.
