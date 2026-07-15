# Phase H macOS Cocoa + Metal Parity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete roadmap Steps 679-758 with production Cocoa and Metal adapters, native macOS verification, unchanged public authoring semantics, and fresh Windows/WSL regressions.

**Architecture:** Keep `PlatformApplication`, `PlatformWindow`, `Renderer`, and `RenderFrame` as the external seams. Turn the existing macOS and Metal files into thin composition roots and place native behavior in focused Objective-C++ modules with private test seams. Reuse existing renderer command and platform event types so examples remain source-identical.

**Tech Stack:** C++23, Objective-C++, AppKit, CoreText, QuartzCore, Metal, Xmake 3.0.9, Xcode 26.6.

**Design:** `docs/superpowers/specs/2026-07-16-phase-h-macos-metal-parity-design.md`

---

## Execution Rules

- For each task: add one focused assertion, run it to the expected failure, add minimal implementation, rerun to green, refactor, then commit.
- Register structure tests outside `is_plat("macosx")`; register native tests inside it.
- Do not place new behavior bodies in public aggregate headers, `macos_application.mm`, or `metal_renderer.mm`.
- Run `git diff --check` and the task's architecture test before every commit.
- Use ARC strong ownership for native members and weak/non-owning delegate
  back-references. Do not call `retain`, `release`, or `autorelease`.
- In this nested worktree, pass `-P "$PWD"` (or the absolute worktree path) to
  every Xmake command; relative `-P .` can resolve the outer checkout.

## Task 1: Steps 679-686 - Cocoa Application and NSWindow Lifecycle

**Files:**
- Create: `src/platform/macos/macos_application_internal.hpp`
- Create: `src/platform/macos/macos_application_delegate.mm`
- Create: `src/platform/macos/macos_event_loop.mm`
- Create: `src/platform/macos/macos_window_internal.hpp`
- Create: `src/platform/macos/macos_window.mm`
- Create: `src/platform/macos/macos_window_factory.mm`
- Create: `src/platform/macos/macos_window_delegate.mm`
- Create: `src/platform/macos/macos_window_lifecycle.mm`
- Create: `src/platform/macos/macos_window_cursor.mm`
- Create: `src/platform/macos/macos_window_wakeup.mm`
- Modify: `src/platform/macos/macos_application.mm`
- Modify: `xmake.lua`
- Test: `tests/architecture/phase_h_cocoa_lifecycle_structure_test.cpp`
- Test: `tests/platform/macos/macos_window_lifecycle_test.mm`

- [ ] Write tests requiring every focused file, a composition root no longer than 120 lines, real `MetalSurfaceHandle`, resize/scale/activation/focus/wakeup events, cancellable close, cursor mapping, and window teardown.
- [ ] Run `xmake -r -P "$PWD" phase_h_cocoa_lifecycle_structure_test macos_window_lifecycle_test && xmake test -P "$PWD" phase_h_cocoa_lifecycle_structure_test/default macos_window_lifecycle_test/default`; expect missing-file and missing-behavior failures.
- [ ] Implement this private adapter shape:

```cpp
class MacOSWindow final : public PlatformWindow {
 public:
  NativeSurfaceHandle native_surface() const override;
  WindowState state() const override;
  PlatformWindowLifecycleState lifecycle_state() const override;
  bool request_display_state(PlatformWindowDisplayState) override;
  PlatformWindowCloseState close_request_state() const override;
  bool resolve_close_request(PlatformWindowCloseResolution) override;
  void request_redraw() override;
  void request_close() override;
  void set_title(std::string_view) override;
  void set_cursor(CursorShape) override;
  void set_ime_text_input_placement(std::optional<ImeTextInputPlacement>) override;
};
```

- [ ] Make `MacOSApplication` own a non-owning window registry, main-run-loop source, delayed timer, and reopen callback. Private Objective-C++ headers use ARC `__strong` native members and weak delegate back-references. The factory allocates all native objects before installing delegates and clears strong ownership in reverse on failure.
- [ ] Rerun the Task 1 command; expect 2/2 pass. Commit with `git commit -m "feat: add cocoa window lifecycle"`.

## Task 2: Steps 687-694 - CAMetalLayer and Renderer Bootstrap

**Files:**
- Create: `src/renderer/metal/metal_renderer_internal.hpp`
- Create: `src/renderer/metal/metal_device.mm`
- Create: `src/renderer/metal/metal_surface.mm`
- Create: `src/renderer/metal/metal_frame.mm`
- Create: `src/renderer/metal/metal_frame_pacing.mm`
- Create: `src/renderer/metal/metal_diagnostics.mm`
- Modify: `src/renderer/metal/metal_renderer.mm`
- Modify: `xmake.lua`
- Test: `tests/architecture/phase_h_metal_bootstrap_structure_test.cpp`
- Test: `tests/renderer/metal/metal_bootstrap_test.mm`

- [ ] Test invalid/empty surfaces, BGRA8 layer setup, device/queue creation, drawable acquisition, resize, committed present, three-frame in-flight limit, and diagnostic snapshot.
- [ ] Run `xmake -r -P "$PWD" phase_h_metal_bootstrap_structure_test metal_bootstrap_test && xmake test -P "$PWD" phase_h_metal_bootstrap_structure_test/default metal_bootstrap_test/default`; expect the placeholder present path to fail.
- [ ] Implement private state:

```cpp
struct MetalRendererState {
  __strong CAMetalLayer* layer = nil;
  __strong id<MTLDevice> device = nil;
  __strong id<MTLCommandQueue> command_queue = nil;
  Size framebuffer_size{};
  DpiScale scale{};
  std::uint64_t next_frame_id = 1;
  std::size_t in_flight_frames = 0;
  RendererFrameDiagnosticSnapshot diagnostics{};
};
```

- [ ] Acquire one drawable per frame, encode a clear pass, commit before presentation, and decrement in-flight state in the completion handler.
- [ ] Rerun Task 2 tests and `git diff --check`; commit with `git commit -m "feat: bootstrap metal rendering"`.

## Task 3: Steps 695-702 - Metal Renderer Primitives

**Files:**
- Create: `src/renderer/metal/metal_pipeline_library.mm`
- Create: `src/renderer/metal/metal_geometry.mm`
- Create: `src/renderer/metal/metal_text.mm`
- Create: `src/renderer/metal/metal_image.mm`
- Create: `src/renderer/metal/metal_clip_transform.mm`
- Create: `src/renderer/metal/metal_command_batches.mm`
- Create: `src/renderer/metal/shaders.metal`
- Modify: `src/renderer/metal/metal_frame.mm`
- Modify: `src/renderer/metal/metal_diagnostics.mm`
- Modify: `xmake.lua`
- Test: `tests/architecture/phase_h_metal_primitives_structure_test.cpp`
- Test: `tests/renderer/metal/metal_primitive_pixel_test.mm`
- Test: `tests/renderer/metal/metal_text_image_pixel_test.mm`
- Test: `tests/renderer/metal/metal_clip_transform_pixel_test.mm`

- [ ] Add deterministic 64x64 pixel tests for solid/rounded rects, text atlas, selections/carets, images/SVG, clips, opacity, transforms, mixed draw order, batching, frame statistics, and `width*height*4` capture bytes.
- [ ] Run all four new targets; expect non-solid primitives and capture to fail.
- [ ] Record every draw with:

```cpp
struct MetalFrameDrawOrderEntry {
  RendererPrimitiveKind primitive_kind{};
  std::size_t command_index = 0;
};
```

- [ ] Compile `shaders.metal`; create solid, rounded, glyph, and image pipelines; upload dirty resources; lower commands in stable order with clip/scissor, transform, and opacity state. Do not add public Metal helpers.
- [ ] Run `xmake test -P "$PWD" phase_h_metal_primitives_structure_test/default metal_primitive_pixel_test/default metal_text_image_pixel_test/default metal_clip_transform_pixel_test/default`; expect 4/4 pass. Commit with `git commit -m "feat: render cgpui primitives with metal"`.

## Task 4: Steps 703-710 - macOS Input

**Files:**
- Create: `src/platform/macos/macos_content_view.mm`
- Create: `src/platform/macos/macos_input_internal.hpp`
- Create: `src/platform/macos/macos_pointer.mm`
- Create: `src/platform/macos/macos_scroll.mm`
- Create: `src/platform/macos/macos_keyboard.mm`
- Modify: `src/platform/macos/macos_window.mm`
- Modify: `src/platform/macos/macos_window_cursor.mm`
- Modify: `src/platform/macos/macos_window_factory.mm`
- Modify: `xmake.lua`
- Test: `tests/architecture/phase_h_macos_input_structure_test.cpp`
- Test: `tests/platform/macos/macos_input_event_test.mm`

- [ ] Test flipped framebuffer coordinates, scale, buttons, precise/line scroll, phase/momentum, modifiers, repeat, key identity, focus, cursors, pointer capture, and drag motion; run targets and expect no-event failures.
- [ ] Implement these private translators:

```cpp
Point macos_event_position_in_framebuffer(double x, double y, double view_height, float scale);
KeyboardKey macos_translate_key_event(std::uint16_t key_code, std::string text, std::uint64_t flags, bool pressed, bool repeat);
PointerScrolled macos_translate_scroll_event(double dx, double dy, bool precise, std::uint64_t phase, std::uint64_t momentum, float scale);
```

- [ ] Put native overrides only in `macos_content_view.mm`; pointer capture installs/removes a local monitor and emits one transition event.
- [ ] Run `xmake test -P "$PWD" phase_h_macos_input_structure_test/default macos_input_event_test/default window_runtime_input_test/default`; expect 3/3 pass. Commit with `git commit -m "feat: add macos input translation"`.

## Task 5: Steps 711-718 - CoreText and Text Services

**Files:**
- Create: `src/platform/macos/macos_font_discovery.mm`
- Create: `src/platform/macos/macos_text_input.mm`
- Create: `src/platform/macos/macos_ime_placement.mm`
- Create: `src/platform/macos/macos_text_services_diagnostics.mm`
- Modify: `src/platform/macos/macos_content_view.mm`
- Modify: `src/platform/macos/macos_application.mm`
- Modify: `src/platform/macos/macos_window.mm`
- Modify: `xmake.lua`
- Test: `tests/architecture/phase_h_macos_text_structure_test.cpp`
- Test: `tests/platform/macos/macos_font_discovery_test.mm`
- Test: `tests/platform/macos/macos_text_input_test.mm`

- [ ] Test stable CoreText face records, committed/marked/unmarked text, UTF-16 to UTF-8 ranges, replacement/delete-surrounding behavior, candidate placement, and diagnostics; run targets and expect generic-fallback failures.
- [ ] Implement:

```cpp
PlatformFontDiscoveryResult macos_discover_fonts();
std::pair<std::size_t, std::size_t> macos_nsrange_to_utf8_bytes(std::string_view, std::size_t, std::size_t);
Rect macos_candidate_rect_to_screen(Rect, Size framebuffer_size, DpiScale, Point window_origin);
```

- [ ] Implement required `NSTextInputClient` selectors on the content view and expose discovery through `MacOSApplication`.
- [ ] Run `xmake test -P "$PWD" phase_h_macos_text_structure_test/default macos_font_discovery_test/default macos_text_input_test/default window_runtime_text_test/default`; expect 4/4 pass. Commit with `git commit -m "feat: add macos text services"`.

## Task 6: Steps 719-726 - Clipboard and Drag/Drop

**Files:**
- Create: `src/platform/macos/macos_clipboard.mm`
- Create: `src/platform/macos/macos_drag_internal.hpp`
- Create: `src/platform/macos/macos_drag_payload.mm`
- Create: `src/platform/macos/macos_drag_source.mm`
- Create: `src/platform/macos/macos_drag_target.mm`
- Modify: `src/platform/clipboard.cpp`
- Modify: `src/platform/macos/macos_content_view.mm`
- Modify: `src/platform/macos/macos_window.mm`
- Modify: `xmake.lua`
- Test: `tests/architecture/phase_h_macos_clipboard_drag_structure_test.cpp`
- Test: `tests/platform/macos/macos_clipboard_test.mm`
- Test: `tests/platform/macos/macos_drag_drop_test.mm`

- [ ] Test named-pasteboard Unicode/file/image payloads, copy/move/link/none negotiation, enter/update/drop/exit ordering, unsupported formats, cancellation, and teardown; run targets and expect adapter-selection failures.
- [ ] Implement `MacOSClipboard : Clipboard`, use `NSPasteboardTypeString`, file URLs, PNG/TIFF, and register destination types on the content view.
- [ ] Keep operation conversion and payload parsing in focused drag modules; always close runtime drag state on drop, exit, cancellation, or window teardown.
- [ ] Run `xmake test -P "$PWD" phase_h_macos_clipboard_drag_structure_test/default macos_clipboard_test/default macos_drag_drop_test/default clipboard_test/default window_runtime_drag_drop_cancellation_diagnostics_test/default`; expect 5/5 pass. Commit with `git commit -m "feat: add macos clipboard and drag drop"`.

## Task 7: Steps 727-734 - macOS Platform Services

**Files:**
- Create: `src/platform/macos/macos_native_menu.mm`
- Create: `src/platform/macos/macos_file_dialog.mm`
- Create: `src/platform/macos/macos_message_dialog.mm`
- Create: `src/platform/macos/macos_open_url.mm`
- Create: `src/platform/macos/macos_window_chrome.mm`
- Create: `src/platform/macos/macos_platform_services.mm`
- Modify: `src/platform/macos/macos_application.mm`
- Modify: `src/platform/macos/macos_application_delegate.mm`
- Modify: `src/platform/macos/macos_window.mm`
- Modify: `xmake.lua`
- Test: `tests/architecture/phase_h_macos_services_structure_test.cpp`
- Test: `tests/platform/macos/macos_platform_services_test.mm`

- [ ] Test recursive menus, enabled/checked state, Command accelerators, open/save panels, alerts, URL validation, reopen, quit policy, and all `WindowChromeOptions`; run targets and expect inherited-unsupported failures.
- [ ] Implement focused functions returning existing result types and dispatch `NativeMenuCommand` through the existing callback. All AppKit calls execute on the main thread.
- [ ] Route `applicationShouldHandleReopen` through `dispatch_reopen("macos")`; preserve content size when changing style masks.
- [ ] Run `xmake test -P "$PWD" phase_h_macos_services_structure_test/default macos_platform_services_test/default native_menu_action_dispatch_test/default`; expect 3/3 pass. Commit with `git commit -m "feat: add macos platform services"`.

## Task 8: Steps 735-742 - NSAccessibility

**Files:**
- Create: `src/platform/macos/macos_accessibility_internal.hpp`
- Create: `src/platform/macos/macos_accessibility_element.mm`
- Create: `src/platform/macos/macos_accessibility_tree.mm`
- Create: `src/platform/macos/macos_accessibility_actions.mm`
- Create: `src/platform/macos/macos_accessibility_events.mm`
- Create: `src/platform/macos/macos_accessibility_lifetime.mm`
- Modify: `src/platform/macos/macos_content_view.mm`
- Modify: `src/platform/macos/macos_window.mm`
- Modify: `xmake.lua`
- Test: `tests/architecture/phase_h_macos_accessibility_structure_test.cpp`
- Test: `tests/platform/macos/macos_accessibility_test.mm`

- [ ] Test navigation, role/subrole, names, values, bounds, focus, text ranges, actions, live notifications, provider reuse, stale rejection, multi-window isolation, and teardown; run targets and expect base no-op failures.
- [ ] Implement generation-keyed providers:

```cpp
struct MacOSAccessibilityProviderKey {
  std::uint64_t window_id = 0;
  std::uint64_t element_id = 0;
  std::uint64_t generation = 0;
};
```

- [ ] Providers resolve every selector against the current window snapshot. Post matching AppKit notifications only after replacement.
- [ ] Run `xmake test -P "$PWD" phase_h_macos_accessibility_structure_test/default macos_accessibility_test/default accessibility_pattern_runtime_test/default window_runtime_multi_window_accessibility_test/default`; expect 4/4 pass. Commit with `git commit -m "feat: add macos accessibility"`.

## Task 9: Steps 743-750 - Examples and Smoke Matrix

**Files:**
- Create: `scripts/ci/macos-example-smoke.sh`
- Create: `scripts/ci/macos-example-targets.txt`
- Test: `tests/architecture/phase_h_macos_example_smoke_structure_test.cpp`
- Test: `tests/platform/macos/macos_example_smoke_test.mm`
- Modify: `xmake.lua`
- Modify: `.github/workflows/ci.yml`

- [ ] Test that every public example appears exactly once, has macOS native dependencies, and covers first frame, resize, interaction, timer/async update, capture where applicable, and close; run targets and expect missing-manifest failures.
- [ ] Add a strict script:

```bash
#!/usr/bin/env bash
set -euo pipefail
while IFS= read -r target; do xmake -r -P "$PWD" "$target"; done < scripts/ci/macos-example-targets.txt
xmake test -P "$PWD" macos_example_smoke_test/default
```

- [ ] Add missing target branches and a macOS CI job; do not add macOS-only example sources.
- [ ] Run both tests plus `bash scripts/ci/macos-example-smoke.sh`; expect all targets pass. Commit with `git commit -m "test: add macos example smoke matrix"`.

## Task 10: Steps 751-756 - Full Cross-Host Verification

**Files:**
- Create: `scripts/ci/macos-debug.sh`
- Test: `tests/architecture/phase_h_macos_full_debug_verification_structure_test.cpp`
- Test: `tests/architecture/phase_h_regression_verification_structure_test.cpp`
- Modify: `xmake.lua`
- Modify: `.github/workflows/ci.yml`
- Modify: `task_plan.md`
- Modify: `progress.md`
- Modify: `findings.md`

- [ ] Add guards requiring a workspace-confined macOS debug script, locked Xmake, complete tests, exact environment/pass evidence, and fresh Windows/WSL evidence; run them and expect absent-evidence failures.
- [ ] Run:

```bash
bash scripts/ci/macos-debug.sh
powershell -File scripts/ci/windows-debug.ps1
wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'
```

- [ ] Record exact Xmake/Xcode/OS versions, totals, failures, and native Metal pixel/smoke coverage. Never record a skipped platform as passed.
- [ ] Rerun both guards; expect 2/2 pass. Commit with `git commit -m "test: verify phase h across hosts"`.

## Task 11: Steps 757-758 - Ledger and Final Closeout

**Files:**
- Test: `tests/api_parity/phase_h_final_closeout_test.cpp`
- Test: `tests/architecture/phase_h_final_closeout_structure_test.cpp`
- Modify: `docs/gpui-complete-parity-ledger.md`
- Modify: `docs/gpui-complete-parity-ledger.json`
- Modify: `docs/gpui-core-api-parity.md`
- Modify: `docs/platform-mac-readiness.md`
- Modify: `docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md`
- Modify: `task_plan.md`
- Modify: `progress.md`
- Modify: `findings.md`
- Modify: `xmake.lua`

- [ ] Add final tests requiring Steps 679-758 checked, ten bands named, exact fresh three-host evidence, zero macOS gaps, JSON/Markdown agreement, Phase G preserved, optional X11 still Phase I, and handoff to Step 759; run them and expect deferred-ledger failures.
- [ ] Update authoritative records with:

```json
{
  "phase_h_status": "complete",
  "phase_h_required_macos_gaps": 0,
  "phase_h_current_handoff": "Phase I Step 759 X11/XCB platform boundary"
}
```

- [ ] Run the complete audit:

```bash
xmake f -y -c -m debug -P "$PWD"
xmake test -y -P "$PWD"
bash scripts/ci/macos-example-smoke.sh
bash scripts/ci/macos-debug.sh
git diff --check
git status --short
```

- [ ] Confirm Windows/WSL evidence is later than the last product change, then commit with `git commit -m "test: close phase h"`.

## Self-Review

- Spec coverage: Tasks 1-11 cover every roadmap band from 679 through 758.
- Module layout: each native responsibility has a focused owner; structure tests cap both composition roots.
- Type consistency: adapters satisfy existing interfaces and keep native types private.
- Verification: macOS native/pixel/examples/full debug plus Windows/WSL and ledger agreement are explicit gates.
- Placeholder scan: no deferred implementation or ambiguous completion step remains.
