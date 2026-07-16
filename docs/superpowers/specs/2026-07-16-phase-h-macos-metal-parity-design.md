# Phase H macOS Cocoa + Metal Parity Design

**Status:** Approved by the continued Phase H goal after the recommended
module design was presented without requested changes.

**Scope:** Roadmap Steps 679-758 in
`docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md`.

## Objective

Bring macOS to the stable Windows/Wayland public behavior through Cocoa,
CoreText, AppKit accessibility, and Metal adapters. Public authoring semantics
remain platform-neutral. Phase H is complete only after native macOS behavior,
renderer pixels, examples, smoke tests, and the complete macOS Debug suite all
pass and the parity ledger records the evidence. Phase G's completed
Windows/WSL verification remains authoritative and is not rerun by Phase H.

## Constraints

- Preserve the existing `PlatformApplication`, `PlatformWindow`, `Renderer`,
  and `RenderFrame` seams unless a required platform-neutral capability is
  demonstrably missing.
- Keep public aggregate headers thin. Platform-specific Objective-C types stay
  in private Objective-C++ headers and implementation files.
- Split implementation by native responsibility from the first change. The
  existing `macos_application.mm` and `metal_renderer.mm` placeholders become
  composition roots, not implementation containers.
- Keep renderer command semantics and ordering identical across Vulkan and
  Metal. Backend-specific encoding may differ, observable output may not.
- Use the ARC mode selected by Xmake/Xcode for Objective-C++ targets. Native
  implementation headers may use `__strong` and `__weak` ownership qualifiers
  but remain private to `.mm` translation units.
- Add structure tests with each new module band. Native behavior tests run only
  on macOS; portable policy and structure tests remain runnable everywhere.

## Chosen Architecture

The public platform and renderer interfaces are the external seams. macOS and
Metal are adapters at those seams. Each adapter contains private internal
seams for deterministic policy tests, but callers never learn AppKit, CoreText,
QuartzCore, or Metal types.

The two existing placeholder files retain only factory/composition behavior:

- `src/platform/macos/macos_application.mm` creates and owns the Cocoa
  application adapter.
- `src/renderer/metal/metal_renderer.mm` validates the surface and creates the
  Metal renderer adapter.

Focused `.mm`, `.cpp`, and `_internal.hpp` files own lifecycle, event
translation, rendering, platform services, and accessibility. This makes the
modules deep: callers use the already-small cross-platform interfaces while
native complexity remains local.

## Module Layout

### Cocoa application and windows, Steps 679-686

- `macos_application_internal.hpp`: private application state and window
  registry interface.
- `macos_application_delegate.mm`: `NSApplicationDelegate` bridge, activation,
  reopen, and termination callbacks.
- `macos_event_loop.mm`: run-loop entry, cross-thread wakeups, delayed wakeups,
  cancellation, and monotonic time.
- `macos_window_internal.hpp`: private retained Cocoa handles and window state.
- `macos_window.mm`: `PlatformWindow` adapter and public virtual overrides.
- `macos_window_factory.mm`: `NSWindow`, content view, and `CAMetalLayer`
  construction with rollback on failure.
- `macos_window_delegate.mm`: close, focus, move, resize, backing scale, and
  display-state notifications.
- `macos_window_lifecycle.mm`: close-resolution and visible/minimized/maximized
  transitions.
- `macos_window_cursor.mm`: `CursorShape` to `NSCursor` mapping.
- `macos_window_wakeup.mm`: redraw invalidation and main-run-loop signaling.

### Metal bootstrap and renderer, Steps 687-702

- `metal_renderer_internal.hpp`: retained device, queue, layer, frame counters,
  diagnostics, and resource caches.
- `metal_device.mm`: device and command-queue creation with stable errors.
- `metal_surface.mm`: `CAMetalLayer` configuration, drawable size, pixel
  format, and drawable acquisition.
- `metal_frame.mm`: frame command collection, present lifecycle, completion
  handling, and dropped-frame reporting.
- `metal_frame_pacing.mm`: in-flight frame limit and drawable timeout policy.
- `metal_pipeline_library.mm`: shader library, pipeline, sampler, and blend
  state creation.
- `metal_geometry.mm`: solid and rounded rectangle lowering.
- `metal_text.mm`: glyph cache consumption, atlas uploads, selection, and caret
  geometry.
- `metal_image.mm`: image/SVG texture upload, invalidation, source rectangles,
  tint, and sampling.
- `metal_clip_transform.mm`: clip stack, opacity, transform, and scissor
  lowering.
- `metal_command_batches.mm`: stable ordering and compatible command batches.
- `metal_diagnostics.mm`: frame timing, resource, draw, upload, pixel capture,
  and frame-statistics snapshots.
- `src/renderer/metal/shaders.metal`: vertex and fragment entry points for
  solid, rounded, glyph, and image pipelines.

Metal testable policy records remain private. Phase H does not add a second
set of backend-branded helper functions to the public `renderer.hpp` aggregate.

### Input and text, Steps 703-718

- `macos_content_view.mm`: `NSView`/`NSTextInputClient` bridge and flipped
  coordinate ownership.
- `macos_pointer.mm`: mouse buttons, movement, drag gestures, enter/exit, and
  pointer capture.
- `macos_scroll.mm`: precise trackpad deltas, phase/momentum metadata, and
  scale conversion.
- `macos_keyboard.mm`: physical/logical key mapping, repeat, modifiers, and
  shortcut dispatch.
- `macos_text_input.mm`: committed text, marked text, replacement ranges,
  unmarking, and delete-surrounding equivalent events.
- `macos_ime_placement.mm`: framebuffer-to-screen candidate rectangle
  conversion.
- `macos_font_discovery.mm`: CoreText family/face enumeration and conversion to
  `FontFaceDescriptor`.
- `macos_text_services_diagnostics.mm`: stable counters and last-failure state
  surfaced through existing diagnostics facilities.

### Clipboard, drag/drop, and platform services, Steps 719-734

- `macos_clipboard.mm`: `NSPasteboard` text, files, and image payloads with
  declared ownership.
- `macos_drag_source.mm`, `macos_drag_target.mm`, and
  `macos_drag_payload.mm`: AppKit drag session lifecycle, operation
  negotiation, payload conversion, completion, and cancellation.
- `macos_native_menu.mm`: recursive `NativeMenuModel` conversion, command
  identity, enabled/checked state, and accelerators.
- `macos_file_dialog.mm`: `NSOpenPanel` and `NSSavePanel` option/result mapping.
- `macos_message_dialog.mm`: `NSAlert` mapping.
- `macos_open_url.mm`: validated `NSWorkspace` URL opening.
- `macos_window_chrome.mm`: titlebar, transparency, decorations, and window
  controls.
- `macos_platform_services.mm`: application-facing composition of these
  focused adapters.

### Accessibility, examples, and closeout, Steps 735-758

- `macos_accessibility_internal.hpp`: native element registry and generation
  ownership.
- `macos_accessibility_element.mm`: `NSAccessibilityElement` provider object.
- `macos_accessibility_tree.mm`: tree update, navigation, roles, state,
  bounds, values, and text ranges.
- `macos_accessibility_actions.mm`: press, increment/decrement, focus, and text
  actions routed through existing accessibility callbacks.
- `macos_accessibility_events.mm`: focus/value/text/live notifications.
- `macos_accessibility_lifetime.mm`: replacement, stale-provider rejection,
  window teardown, and generation checks.
- Existing public examples remain source-identical. Xmake gains macOS smoke,
  renderer pixel, input, service, and accessibility test targets rather than
  macOS-specific example source forks.
- Phase closeout adds a macOS full-debug verifier, preserves the completed
  Phase G Windows/WSL evidence as history, and updates the Markdown/JSON ledger
  plus roadmap only after a fresh successful macOS run.

## Data Flow

### Window and input

1. `PlatformApplication::create_window` enters `macos_window_factory.mm`.
2. The factory creates an `NSWindow`, `CGPUIMacOSContentView`, and
   `CAMetalLayer`, then installs delegates only after all resources succeed.
3. Cocoa callbacks enter focused translation modules.
4. Translators normalize Cocoa coordinates, scale, modifiers, key identity,
   and text ranges into existing `PlatformEvent` variants.
5. `PlatformEventCallback` delivers events to `WindowRuntime` without
   platform-specific branches in public UI code.

### Rendering

1. `WindowRuntime` emits the same `RenderFrame` calls used by Vulkan.
2. `MetalFrame` records compact command records in call order.
3. Present lowers records into geometry, texture uploads, pipeline batches,
   clip/scissor state, and Metal encoders.
4. One command buffer targets one acquired drawable. Completion updates
   diagnostics and releases in-flight resources.
5. Pixel capture, when requested, copies the drawable texture to a managed
   readback buffer before present completion and exposes it through
   `last_frame_pixels()`.

### Accessibility and services

Application service calls synchronously translate cross-platform option types
to AppKit and return the existing structured result types. Accessibility tree
updates replace a window-owned snapshot; provider objects retain only window
identity, element identity, and generation so stale objects fail closed rather
than keeping old trees alive.

## Ownership and Threading

- AppKit objects and all `NSApplication`, `NSWindow`, view, menu, dialog,
  pasteboard, drag, and accessibility operations execute on the main thread.
- Cross-thread wakeup requests signal `CFRunLoopSource`; they never call AppKit
  directly from worker threads.
- Cocoa delegates hold weak/non-owning references and are detached before C++
  adapter destruction. C++ adapters use ARC-strong native members for objects
  they expose or use; explicit `retain`, `release`, and `autorelease` calls are
  forbidden.
- Metal device and queue are renderer-owned. A frame owns its drawable and
  command buffer until completion. Cached buffers and textures use explicit
  frame-generation retirement so no in-flight resource is destroyed early.
- Window registries use stable numeric identity and erase entries before
  native window release to prevent callback-after-destruction.

## Error Policy

- Factory failures return existing `Result<T>` errors with the narrowest
  applicable `ErrorCode` and stable backend-prefixed messages.
- Invalid descriptors, null layers, empty framebuffer sizes, unavailable
  devices, queue/library/pipeline failures, and drawable acquisition failures
  are explicit; no successful placeholder frame is returned.
- Recoverable drawable absence or resize races produce dropped-frame
  diagnostics and allow the next frame. Device or pipeline initialization
  failures fail renderer creation.
- Unsupported clipboard/drag payload formats are ignored while supported
  formats continue. Total conversion failure is recorded in diagnostics.
- Close requests remain cancellable through `resolve_close_request`; native
  destruction happens only after an accept resolution.
- Stale accessibility providers return unavailable/empty values and never
  dereference replaced tree storage.

## Testing Strategy

Every behavior change follows red-green-refactor.

1. **Portable structure and policy tests:** require each focused source file,
   cap composition-root line counts, reject Objective-C implementation bodies
   in public headers, validate Xmake registration, and exercise pure mapping
   helpers where AppKit is not required.
2. **macOS native tests:** create Cocoa windows and Metal devices on a Mac host;
   verify lifecycle, scale, input translation, CoreText discovery, clipboard,
   drag negotiation, menus/dialog policies, accessibility providers, drawable
   recovery, diagnostics, and resource lifetime.
3. **macOS pixel and smoke tests:** compare solid, rounded, text, image, clip,
   opacity, transform, ordering, resize, and capture output; run every public
   example through first frame, resize, interaction, and close.
4. **Regression tests:** run the complete macOS Debug suite on the current
   Xcode host. Phase G already owns Windows/WSL full-debug verification, so
   Phase H does not duplicate those platform runs.
5. **Closeout audit:** a final parity test checks evidence for all Steps
   679-758, zero required macOS ledger gaps, current handoff to Phase I, and no
   regression of the Phase G Windows/Linux status.

## Completion Criteria

Phase H is complete only when all of the following are true:

- Roadmap Steps 679-758 and all ten Phase H bands are checked with concrete
  test evidence.
- `macos_application.mm` and `metal_renderer.mm` are thin composition roots;
  all new native behavior lives in focused modules.
- macOS implements window lifecycle, Metal rendering primitives, input, text,
  clipboard/drag, platform services, accessibility, and public-example smoke
  behavior through existing public authoring interfaces.
- macOS full debug passes on the current Xcode/command-line-tools host.
- Phase G's successful Windows and WSL full-debug evidence remains preserved;
  no new Windows/WSL run is a Phase H completion requirement.
- The Markdown and JSON parity ledgers agree, report no required macOS gaps,
  retain optional X11 as Phase I, and contain exact verification counts and
  environment evidence.
