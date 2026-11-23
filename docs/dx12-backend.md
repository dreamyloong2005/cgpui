# Experimental Direct3D 12 backend

DX12 is implemented as an **opt-in opaque-window bootstrap**, not a complete
replacement for Vulkan. Vulkan remains the Windows default; Linux stays Vulkan
and macOS stays Metal. The previous Popup policy is unchanged for Vulkan.

## Implemented slice

- Native D3D12 hardware device (feature level 11.0 and shader model 6.0), direct
  command queue, root signature/PSO, two-buffer HWND swapchain and fence-based
  resource retirement. The first suitable hardware adapter is selected; software
  fallback is not automatic.
- Clear, solid and rounded rectangles, selection/caret rectangles, clipping,
  composed transforms/opacity, DPI scaling and original draw order.
- Text uses the shared shaping/rasterization path and persistent R8 glyph atlas
  pages, including native DirectWrite glyphs. Images support RGBA8 uploads,
  nearest/linear clamp sampling, source crops, tint and cached reuse. Both consume
  the neutral CPU geometry and preserve order when interleaved with rectangles.
- Outer box shadows reuse the shared analytic shader and CPU geometry: CSS
  blur radius/sigma and spread, rounded corners, original-box clip-out, affine
  transforms, composed opacity, clips and DPI. Shadow commands retain their
  position in the same rectangle/text/image draw stream.
- Backdrop blur snapshots the scene immediately before each effect, reuses the
  shared downsample/separable Gaussian/rounded-mask shader, mixes tint and
  composed opacity, then resumes the original draw stream. `blur_radius` is
  Gaussian sigma (unlike outer-shadow CSS radius/2). The actual RGBA8
  render/load/filter-sample capability is queried from the device.
- Pixel readback with row-pitch handling, resize, and explicit rejection of
  overlapping frames or resize while a frame is outstanding. Destroying a
  renderer before its outstanding frame is safe; that frame cannot present.
- Shared CPU geometry in `renderer/planning` and shared Slang primitive shaders;
  no DX12-to-Vulkan package dependency or runtime shader compiler.
- Vulkan and DX12 populate `Renderer::backend_info()` and
  `Window::backend_info()` with the actual backend/adapter, vendor/LUID,
  software flag and selection. Other renderers, including the unchanged Metal
  backend, use the base `unknown` report until they opt in.

This is a synchronous baseline: GPU completion is awaited for each submitted
frame. Upload allocation pooling and deeper frame overlap remain optimization
work; the bootstrap is not a performance-parity claim.

Image uploads and invalidations update the CPU cache while commands are recorded;
abandoning or rejecting a frame does not roll back those cache operations. Each
recorded draw retains an immutable texture snapshot, so replacing/invalidation
later in the same frame does not change earlier draws. Uploads reach the GPU
lazily when first drawn; images are evicted after more than 120 successfully
presented idle frames. Glyph pages are cached for the renderer lifetime and
republished as full-page snapshots when dirty. This is correctness-first, not a
partial-upload, pooled-descriptor or bounded-glyph-cache performance guarantee.

Backdrop PSOs are lazy and cached. Each visible effect owns a full-frame scene
snapshot and two region-sized downsample targets with immutable descriptors.
They are retained until the next successful frame-start idle wait or renderer
destruction, including across resize; frames without effects allocate none.
This avoids reusing resource states from unsubmitted command lists, but is not
target pooling or a bounded peak-memory/performance guarantee. Full renderer
diagnostic snapshots/timings remain unimplemented; effect capability reporting
does not imply diagnostic parity.

Resource-lifetime validation covers normal submission, resize and orphan frames,
not arbitrary GPU failures. Repeated queue Signal, event-registration or wait
failures have not been fault-injected; safe retirement during destruction after
such failures remains a production-readiness gate.

## Selection and testing

On Windows, `create_renderer` reads `CGPUI_RENDERER`:

| Value | Behavior |
| --- | --- |
| Unset, empty, `auto` | Vulkan only until DX12 reaches parity |
| `vulkan` | Vulkan only; return its error on failure |
| `dx12` | Experimental DX12 only; return its error on failure |
| Anything else | `invalid_argument`; create neither backend |

Named factories in `cgpui/renderer/renderer_backend.hpp` bypass that selection:
`create_vulkan_renderer(descriptor)` and, on Windows,
`create_dx12_renderer(descriptor)`. Application code can supply either through
its existing renderer factory; `RenderSurfaceDescriptor` has no new backend or
native/composition mode field.

Windows applications using `create_renderer` link `cgpui.renderer-windows`,
which owns both backend dependencies. Backend-only consumers may link the
named backend package. SDK link closure includes the dispatcher and both
backends, but this does not change the default or certify a DX12 release.
The dispatcher still carries the normal Vulkan loader dependency: selecting
`dx12` does not provide loader-free startup. Vulkan delay-loading is not
implemented. Linking only the named DX12 package avoids its Vulkan package
dependency.

Run the native acceptance suite from PowerShell:

```powershell
$env:MCPP_JOBS = '1'
scripts/dev/test.ps1 -p tests/renderer-dx12
# Deterministic software rasterizer and strict native validation:
$env:CGPUI_DX12_ADAPTER = 'warp'
$env:CGPUI_DX12_DEBUG = '1'
scripts/dev/test.ps1 -Renderer dx12 -p tests/renderer-dx12
```

`CGPUI_DX12_ADAPTER` also accepts an explicit DXGI adapter index, with no
fallback. `CGPUI_DX12_GPU_VALIDATION=1` requests GPU-based validation. Debug
requests require the installed Windows graphics debugging layer; missing
support returns an error instead of silently claiming validation. Debug
configuration is fixed before the process's first DX12 device and cannot be
changed by later renderer creations. These environment variables are diagnostic
controls, not cross-platform window options. Use a separate process per debug
configuration. The hello-window and backdrop-blur examples can use this opaque
subset. Use the dedicated native tests for pixel acceptance; one first-frame
smoke does not establish full workbench interaction or desktop appearance.

## Not implemented yet

Transparent ordinary windows,
DirectComposition Popup presentation and device-loss recovery are future work.
Unsupported capabilities reject their commands rather than silently dropping
content. Transparent/composition descriptors fail during creation;
`popup_windows_supported()` is false. This genuine missing capability permits
the existing overlay fallback; Vulkan is not preemptively disabled.

The original DX12 plan's shared extraction and shader phases are only partially
complete: rectangle/text/image/shadow/backdrop algorithms and DXIL are implemented,
but full parity, transparent-window/native-menu compatibility, diagnostics, recovery,
performance gates and default-switch/release verification must precede automatic
DX12 selection. No release was published by this milestone.

See `.planning/2026-11-23-dx12-bootstrap/`,
`.planning/2026-11-23-dx12-textures/`,
`.planning/2026-11-23-dx12-shadows/` and
`.planning/2026-11-23-dx12-backdrop/` for dated validation evidence.
