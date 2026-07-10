# GPUI-Core API Parity Audit

Step 258 establishes the complete-replication parity ledger for a pinned
upstream GPUI revision:
`5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`.
The detailed source-of-truth ledger is
`docs/gpui-complete-parity-ledger.md`, with machine-readable status in
`docs/gpui-complete-parity-ledger.json`.

The active implementation target remains Windows/Win32 + Vulkan and
Linux/Wayland + Vulkan. macOS Cocoa + Metal remains a later parity track after
Windows/Linux public APIs and renderer/platform boundaries stabilize. X11 is
tracked as deferred, not active, unless the user explicitly chooses strict
upstream Linux backend parity.

## Windows/Linux Scope

- Windows uses the Win32 platform backend and Vulkan renderer target.
- Linux uses the Wayland platform backend and Vulkan renderer target.
- The public authoring surface is exposed through `cgpui/cgpui.hpp` and the
  core UI headers.
- Verification for this track is Windows plus WSL Arch Linux; macOS is not run
  from this machine.

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
  deferred work.
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
- Multi-window support records per-window runtime ownership and app-opened
  roots, and app-opened child windows now attempt native `PlatformWindow`
  creation with graceful per-record `native_window_error` storage. Independent
  child renderers, child render loops, full event routing, and production
  lifecycle ownership remain incomplete.
- Frame timing diagnostics expose stable counters, and production Vulkan frame
  pacing now covers bounded CPU submission, MAILBOX/FIFO selection, swapchain
  recovery, and coalesced next-frame redraws; real profiler timing remains
  incomplete.
- Font discovery has deterministic abstractions and platform override slots,
  but real DirectWrite/fontconfig discovery is not complete.
- Threaded async now has cancellable background work and main-runtime
  completion dispatch, but it is still a small executor skeleton rather than a
  full GPUI async runtime with priorities, structured task groups, pooled
  scheduling policy, async I/O integration, or cross-thread entity access.

## Missing

- Full upstream GPUI animation and easing system.
- Rich theme inheritance, dynamic runtime theme switching, and full design
  system integration.
- Asset, image, SVG, and texture upload pipelines.
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
- Cross-thread entity access guarantees, async I/O integration, task priority
  scheduling, and a production task-pool runtime beyond the current small
  threaded executor.

## Mac/Metal Deferred

macOS parity is intentionally deferred. Step 88 already reserves the target
mapping: macOS/Cocoa uses Metal, while Windows and Linux use Vulkan. The
current macOS slots provide source-level readiness for Cocoa windows, a Metal
surface handle, and a Metal renderer placeholder, but they do not implement the
Windows/Linux behavior listed above.

The Mac handoff requires:

- Cocoa app lifecycle, window lifecycle, cursor, keyboard, pointer, clipboard,
  drag/drop, IME, accessibility, menu, and focus adapters.
- Metal renderer parity for solid rectangles, rounded rectangles, text glyphs,
  clipping, transforms, opacity, HiDPI scale, command diagnostics, and frame
  statistics.
- macOS build and runtime verification on a Mac host.
- Alignment with the existing platform-neutral APIs instead of forking public
  authoring semantics.

## Next Milestone

The next Windows/Linux milestone should be a depth pass, not another surface
area expansion. Recommended order:

1. Make Vulkan text rendering real: promote the existing fallback rasterized
   glyphs, atlas allocation records, upload records, textured glyph quads, and
   text render counters into real Vulkan atlas textures, GPU uploads, shader
   sampling, and draw calls.
2. Continue promoting Wayland clipboard, Wayland drag/drop, and Wayland
   text-input from deterministic protocol slices to production desktop
   handling, including clipboard write/ownership, drag action negotiation, and
   richer text-input editing semantics.
3. Deepen Windows UIA and Linux AT-SPI skeletons into production accessibility
   bridges using the existing accessibility snapshot as the source of truth.
4. Promote the native additional-window scaffold into fully owned child-window
   renderer, event-loop, activation/focus, and lifecycle behavior.
5. Start a separate macOS/Cocoa + Metal parity track only after the
   Windows/Linux depth pass has stable renderer and native adapter behavior.
