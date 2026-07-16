# GPUI-Core API Parity Audit

Step 258 establishes the complete-replication parity ledger for a pinned
upstream GPUI revision:
`5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`.
The detailed source-of-truth ledger is
`docs/gpui-complete-parity-ledger.md`, with machine-readable status in
`docs/gpui-complete-parity-ledger.json`.

The implemented desktop targets are Windows/Win32 + Vulkan, Linux/Wayland +
Vulkan, Linux/X11 + Vulkan, and macOS/Cocoa + Metal. Phase H closes macOS
production parity over the stable public APIs and renderer/platform boundaries;
Phase I closes strict Linux backend parity over the same public surface.

## Desktop Scope

- Windows uses the Win32 platform backend and Vulkan renderer target.
- Linux provides Wayland and X11 platform backends with Vulkan renderer targets.
- macOS uses the Cocoa platform backend and Metal renderer target.
- The public authoring surface is exposed through `cgpui/cgpui.hpp` and the
  core UI headers.
- Phase G records Windows and WSL Arch Linux verification; Phase H records
  native macOS verification on the current Xcode host; Phase I records the
  final Windows, WSLg, and isolated Xvfb X11 matrices.

## Not Full Upstream GPUI Parity

This project is close to a practical GPUI-core-shaped API on Windows and Linux,
but it is not full upstream GPUI parity. The current implementation focuses on
the core authoring model, deterministic element/runtime behavior, reusable
widgets, renderer command metadata, and platform hooks. Production-grade
native adapters, full text rendering, deep theming, full animation, and several
desktop integration surfaces remain separate future work.

## Implemented

- Public prelude and authoring entry: `cgpui/cgpui.hpp`, `run_app`,
  `AppRunnerOptions`, `AppContext`, `WindowOptions`, `View::render(ViewContext&)`,
  `Context<T>`, public element factories, style helpers, and demo usage through
  the public API.
- Model and state APIs: `Entity<T>`, `Model<T>`, weak handles, entity
  read/update/remove helpers, typed globals, model observers, owned
  `Subscription` tokens, update batching, invalidation snapshots, and
  diagnostics snapshots.
- Runtime scheduling APIs: deferred callbacks, deterministic timers, async
  task handles with main-thread completion dispatch, cancellable background
  task execution through a small threaded executor, task cancellation tokens,
  task diagnostics counters, and platform wakeup hooks for timers, tasks, and
  deferred work, plus typed scalar animation transitions with eased from/to
  value snapshots across runtime, runtime-context, and async-context surfaces,
  and runtime-owned keyed element lifecycle animation state with scoped
  mount/update/unmount tracking and shared delayed frame wakeups, including
  infinite repeated stages, indexed one-shot animation chains, pinned tween
  curve variants, and parameterized spring curves shared across runtime,
  element, sequence, and style animation paths, plus distinct cancellation
  state with frozen progress, immediate callback release, idempotent timer
  teardown, handle observability, and aggregate plus last-cancellation diagnostics,
  plus one deadline-driven frame timer shared by ordinary and element
  animations with cadence-preserving late-frame delivery, and
  broad production style interpolation across layout, paint, and typography
  fields. The pinned official animation and opacity examples compile and run
  through the same public authoring surface without direct runtime internals.
- Assets now have root-confined file-backed asset loading through
  `AssetSource` and `FileAssetSource`, with optional missing results, stable
  directory listing, canonical escape protection, and pre-allocation limits.
  Signature-detected PNG and JPEG decoding produces bounded RGBA8 bitmaps
  through a fixed, private stb_image backend with explicit status reporting.
  Animated GIF decoding preserves bounded composited RGBA8 frames, delays, and
  finite/infinite loop metadata after structured pre-decode block scanning.
  Bounded SVG asset decoding validates intrinsic dimensions through LunaSVG
  and bridges safely into the existing viewport-aware recolorable raster path.
  Stable decoded-asset cache keys isolate source, normalized cross-platform
  path, kind, and revision with deterministic FNV-1a hashing.
  Thread-safe asset reload invalidation advances shared source/path revisions
  atomically across decoded variants with saturation and diagnostics.
  Bounded async asset loading owns source lifetimes, validates cache identity,
  reads on priority-aware workers, and completes success, missing, or error
  results on the owning runtime thread with cancellation suppression.
  Frame-local `PaintList` invalidation/upload records reach the renderer before
  image draws and are cleared with reused paint state. The pinned public image
  and GIF viewer examples compile and smoke-run through the prelude; GIF frames
  advance from decoded durations and loop metadata without bitmap copying.
- Standalone `TestApp` and `TestAppWindow` setup owns a deterministic private
  platform and renderer, creates persistent test windows without a production
  event loop, exposes stable runtime/root-view handles and typed root access,
  and rejects empty root views without changing runtime state.
- Window-scoped `TestAppWindow` input simulation routes keyboard sequences,
  pointer movement/buttons/scrolling, activation, window focus, and element
  focus through the deterministic platform callback while preserving isolated
  per-window input snapshots and rejecting invalid key grammar.
- Deterministic `TestApp` timer control advances runtime time explicitly,
  drains only ready work until parked, combines both operations, cancels timers
  by id, and uses a fixed private platform clock plus runnable hidden parent.
- Deterministic `TestApp` async control manually completes runtime tasks,
  drains completions by priority and FIFO order, processes nested ready tasks,
  rejects invalid/repeated ids, and reuses the production queue and task pool.
- Per-window `TestAppWindow` rendering control simulates resize and redraw,
  exposes fallible and throwing frame draws, runs the production additional-
  window render path, and reports isolated private renderer/frame snapshots.
- Deterministic `TestApp` platform service fakes isolate clipboard and opened
  URL state, consume FIFO file/prompt responses, fail closed when queues are
  empty, and expose menu/dialog/URL/reopen/quit call snapshots.
- A GPUI-style C++ test runner provides ordinary executable entry macros,
  typed multi-TestApp and deterministic-seed injection, explicit/environment
  seed plans, bounded retries, failure callbacks, and reproducible summaries.
- Unit-action macro production behavior provides `CGPUI_ACTIONS(...)` and
  `CGPUI_ACTIONS_IN(namespace_name, ...)` from a focused public leaf, generating
  default-constructible, copyable, equality-comparable `Action` types with
  stable unscoped or namespace-qualified names. Payload actions remain explicit
  C++ types outside the unit-action macro contract.
- Element tree behavior: `AnyElement`, keyed elements, keyed reconciliation,
  lifecycle hooks, per-element state storage, event routes, bubbling, disabled
  handling, focus traversal, focus handles, pointer capture, scroll routing,
  hit testing, layout, z order, and deterministic paint traversal.
- Reusable widgets: `button`, `label`, `text_input`, and `scrollable_list`
  built from public element, focus, style, action, text model, clipboard, and
  scroll APIs.
- Style and layout primitives: logical pixel helpers, colors, spacing, border,
  radius, flex alignment, grow/shrink, absolute positioning, layer/elevation,
  style state overlays, style classes, theme tokens, and deterministic cascade
  resolution.
- Text and renderer command depth: font descriptors, font database skeleton,
  deterministic fallback shaping and fallback glyph raster data, glyph metadata,
  glyph cache/atlas allocation records, text paint commands, textured glyph
  quad records, caret/selection metadata, opacity and transform metadata,
  HiDPI scale propagation, renderer command batching diagnostics, submission
  plan records, frame statistics, unsupported-command diagnostics, and
  frame-level Vulkan text/render reports that distinguish glyph-backed work,
  planned submissions, and remaining renderer gaps from metadata-only
  placeholders.
- Platform hooks on active targets: Win32 cursor application, Win32 clipboard,
  Wayland clipboard support state plus text selection payload extraction,
  focused text IME geometry, Win32 IME placement, Wayland text-input v3
  enter/leave plus preedit/commit event dispatch, Win32 drag/drop event
  skeleton, Wayland data-device drag/drop text and URI-list payload extraction,
  lifecycle events, multi-window runtime registry, native additional-window
  creation scaffolding, platform wakeups, and accessibility tree snapshots.
- Windows/Linux demo smoke coverage exercises window creation, first frame,
  resize, close, text input, clipboard flow, redraw, and bounded shutdown.

## Partial

- Renderer text is CPU fallback-raster/atlas-record-backed. Vulkan consumes
  text draw commands into glyph cache lookups, fallback rasterized glyphs,
  upload records, textured glyph quad records, and deterministic text render
  report counters, but real Vulkan atlas texture objects, GPU uploads, shader
  sampling, subpixel positioning, and font fallback shaping are not complete.
- Accessibility has a platform-neutral tree snapshot and platform update
  payload. Windows UIA and Linux AT-SPI adapter skeletons now consume snapshot
  node summaries and track root, node, focused-node, and text-input counts, but
  production UI Automation and AT-SPI providers are not implemented.
- Wayland clipboard is partially protocol-backed. It can connect to a Wayland
  display, bind data-device state, track selection MIME offers, prefer
  `text/plain;charset=utf-8` over `text/plain`, and read text payload bytes
  through `wl_data_offer_receive`; default platform-factory integration,
  clipboard ownership/write offers, non-text formats, and production desktop
  edge cases remain incomplete.
- Wayland drag/drop is partially protocol-backed. It tracks drag data-offer
  MIME types, reads `text/plain` and `text/plain;charset=utf-8` payloads,
  parses `text/uri-list` local file URIs into file payloads, and keeps no-data
  offers graceful. Drag actions, accept/finish negotiation, non-local URI
  handling, richer MIME formats, and production desktop integration remain
  incomplete.
- Wayland IME is partially protocol-backed. It binds a minimal
  `zwp_text_input_v3` manager when available, reports text-input support as
  available, commits surrounding-text cursor and default content-type records
  from focused text placement, and routes deterministic enter/leave plus
  preedit/commit protocol events into public `ImeComposition` events. Full
  desktop input-method integration, delete-surrounding editing, richer content
  hints/purposes, serial synchronization policy, and production compositor
  edge cases remain incomplete.
- Win32 drag/drop uses deterministic event hooks and public payload shapes, not
  full OLE `IDropTarget` shell integration or drag-effect negotiation.
- Multi-window support records per-window runtime ownership, independent child
  renderers, event routing, and deterministic close cleanup. App-opened child
  windows defer native activation until the root exists, then use a real root
  owner HWND on Win32 and `xdg_toplevel.set_parent` on Wayland. Broader
  simultaneous multi-window event-loop fairness and stress remain incomplete.
- Frame timing diagnostics expose stable counters, and production Vulkan frame
  pacing now covers bounded CPU submission, MAILBOX/FIFO selection, swapchain
  recovery, and coalesced next-frame redraws; real profiler timing remains
  incomplete.
- Font discovery has deterministic abstractions and platform override slots,
  but real DirectWrite/fontconfig discovery is not complete.
- Threaded async now has a bounded reusable pool, low/normal/high scheduling,
  main-runtime completion dispatch, and move-only structured task groups with
  recursive parent/child observability plus descendant bulk/destructor
  cancellation. One-shot async I/O hooks now deliver payloads or failures back
  to the runtime thread with priority, cancellation, and teardown safety.
  Runtime timers use platform monotonic clocks and nearest-deadline delayed
  wakeups on Win32 and Wayland. Explicit `CrossThreadEntity<T>` handles queue
  callback-scoped reads and updates back to the owning runtime thread with
  context isolation, missing-entity results, and shutdown detachment.

## Missing

- Rich theme inheritance, dynamic runtime theme switching, and full design
  system integration.
- Final image/GIF public examples.
- Rich text editing, selection handles, undo/redo stacks, complex shaping,
  bidi text, emoji/color glyphs, and platform input-method depth beyond the
  current skeletons.
- Native menus, command palettes, accelerators, window chrome customization,
  dialogs, file pickers, and deeper shell integration.
- Production accessibility bridges for UIA and AT-SPI.
- Full Vulkan renderer pipeline for rounded rectangles, text selection/caret
  drawing, glyph atlas textures, clipping stacks, transforms, opacity, and GPU
  batching.
- Full layout virtualization and large-list recycling beyond the current
  `scrollable_list` container.

## Mac/Metal Complete

Phase H implements production Cocoa application/window lifecycle, Metal
surface and primitive rendering, input, CoreText/font and text-input services,
clipboard/drag, native menus and dialogs, URL/reopen/chrome services,
NSAccessibility providers, and source-identical public examples. Native types
remain private to focused Objective-C++ modules behind the existing
platform-neutral interfaces.

Phase H macOS full debug passes 380/380 on macOS 26.5.2 (25F84), Xcode 26.6 (17F113), and Xmake 3.0.9+HEAD.2b184e178, including native Cocoa, Metal primitive/clip/text-image pixel capture, accessibility, and public-example smoke coverage.

Phase H required macOS gaps: 0. Phase I Step 759 X11/XCB platform boundary.

Phase I Step 798 completes strict Linux backend parity with Windows full Debug
at 478/478, WSLg full Debug at 470/470, an isolated Xvfb X11 matrix at 11/11,
cross-platform/macOS source guards, and preserved Phase H native macOS evidence
at 380/380. Phase I required X11 gaps: 0. Phase J Step 799 re-run upstream extractor against the pinned
revision.

## Next Milestone

Phase J Step 799 re-runs the upstream extractor against the pinned revision and
fails if any required API row lacks a C++ equivalent or accepted adaptation.
