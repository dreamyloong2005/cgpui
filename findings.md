# CGPUI GPUI-Core Findings

## 2026-07-04 Complete GPUI Replication Planning

- Official GPUI sources checked for the roadmap were the Zed GPUI README,
  gpui.rs, the GPUI crate root, context docs, and key dispatch docs. The useful
  planning conclusion is that full replication must be pinned to a specific
  upstream revision because upstream GPUI is pre-1.0 and can break between
  versions.
- The official scope is broader than the current CGPUI Step 218 core: it
  covers state/entities, views/rendering, low-level elements, contexts,
  actions/key dispatch, platform services, async executor integration, test
  context support, and the public example set.
- The roadmap should not start macOS or X11 immediately. The durable order is
  upstream parity ledger first, Windows/Linux production parity second,
  macOS/Cocoa + Metal third, and optional X11 only if the user later wants
  strict upstream Linux backend matrix parity.
- A complete roadmap was written to
  `docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md`.
  It estimates roughly 460 more slices for strong Windows/Linux production
  parity, roughly 540 for Windows/Linux plus macOS, and roughly 580-620 for a
  strict all-upstream-platform interpretation, with the exact count corrected
  after Phase A generates the parity ledger.

## 2026-07-03 Aggressive Structural Optimization Plan

- The user clarified that the optimization target should be thorough even when
  the risk is higher than the earlier conservative structure pass. The next
  plan should therefore optimize for long-term file boundaries over short-term
  source compatibility.
- A fresh scan of `.worktrees/structural-optimization` shows implementation
  files are no longer the main problem: `src/platform/linux` has a 177-line
  maximum with four files over 120 lines, `src/renderer/vulkan` has a 151-line
  maximum with four files over 120 lines, `src/ui` has a 128-line maximum, and
  `src/platform/win32` has a 149-line maximum.
- The dominant remaining issue is public header size. `include/cgpui/ui` has
  29 headers, 6252 total lines, 13 headers over 200 lines, and 11 headers over
  300 lines. The biggest are `text_model.hpp` 688 lines,
  `runtime_types.hpp` 567, `text_layout.hpp` 481, `window_runtime.hpp` 464,
  and `element_tree.hpp` 461.
- The preferred aggressive sequence is private declaration surgery first,
  remaining long bridge implementation files second, public API header surgery
  third, and test-suite structure fourth. This avoids starting with public API
  churn while private implementation boundaries are still broad.
- The public header pass should move non-template inline bodies out of headers,
  split template implementation headers by domain, keep compatibility
  aggregate headers under 40 lines, and reduce public leaf headers toward a
  220-line ceiling. For especially large public classes, PIMPL/private state or
  focused facade domains are acceptable because compatibility is not the top
  constraint for this pass.
- A standalone design was written to
  `docs/superpowers/specs/2026-07-03-structural-optimization-design.md`, and
  an executable plan was written to
  `docs/superpowers/plans/2026-07-03-structural-optimization-execution-plan.md`.

## 2026-07-03 Structural Optimization Audit

- With the latest aggressive split, `win32_application.cpp` is no longer a
  Win32 window monolith. It is now the Win32 application/factory layer; window
  lifecycle/chrome/DPI, input/test drag hooks, IME placement, and OLE drag-drop
  ownership have separate files guarded by `win32_window_source_test`.
- `runtime_context.cpp` was a pure forwarding pile. It now only constructs
  `WindowRuntimeContext`; input/focus/tree forwarding, action/command
  forwarding, text/clipboard forwarding, scheduling/diagnostics forwarding,
  and native platform/theme forwarding are separate files guarded by
  `ui_source_structure_test`.
- This continuation also split `WaylandApplication` construction from window
  creation, event-loop forwarding, platform service facades, and the platform
  factory; split renderer report key comparison helpers out of command and
  submission planning; split Wayland pointer axis/scroll handling from
  enter/move/button handling; split `PaintList` text/image command emission
  from core paint stack commands; split `WindowRuntime` run/shutdown from the
  constructor; and split Win32 drag/drop payload extraction by source.
- The later aggressive pass then split Wayland text-input protocol bindings,
  Wayland DnD drag actions/events, runtime animation start/state/tick,
  Vulkan presentation recovery, Vulkan swapchain creation, Win32 window
  chrome/size handling, Wayland window input/drag/text event forwarding,
  AppContext/run_app responsibilities, and Win32 clipboard text/read/write
  responsibilities. The old aggregate files for those areas are now 1-line or
  very small placeholders guarded by architecture tests.
- After the latest scan, the implementation-oriented files still above about
  150 LF are `wayland_application_internal.hpp` (186),
  `vulkan_internal.hpp` (170), `vulkan_swapchain_create.cpp` (161),
  `win32_application.cpp` (159), `wayland_window.cpp` (156),
  `wayland_application_windows.cpp` (155), `wayland_window_internal.hpp` (154),
  `win32_window_proc.cpp` (154), and `vulkan_report_submission.cpp` (153).
  These are now mostly private declaration headers, single long lifecycle
  flows, or platform message bridges rather than old mixed-responsibility
  monoliths.
- The next implementation-file optimization pass should focus on private
  declaration surgery (`wayland_application_internal.hpp`,
  `vulkan_internal.hpp`, `wayland_window_internal.hpp`) and then decide whether
  single-flow files like `vulkan_swapchain_create.cpp` should be split by
  helper extraction or left intact for readability. The remaining large public
  headers are a separate public API surgery pass with higher include-order and
  source-compatibility blast radius.

## 2026-07-03 UI Runtime Structure Split

- `include/cgpui/ui/ui.hpp` is now a compatibility aggregate over
  `paint.hpp`, `view.hpp`, and `runtime.hpp` instead of owning the full public
  UI declaration surface.
- Paint declarations now live with paint command types and `PaintList`; view
  declarations carry `ViewContext` / `Context<T>` and the `View` base class;
  runtime declarations keep `WindowRuntime`, `WindowRuntimeContext`,
  `AppContext`, templates, and runner APIs.
- The former `src/ui/ui.cpp` monolith is split into focused implementation
  files. `ui.cpp` is now only the small RAII/handle implementation layer, while
  paint, render-view, app context, runtime core, events, scheduling,
  diagnostics, and context forwarding have separate compilation units.
- `src/ui/ui_internal.hpp` is deliberately private to the UI implementation and
  keeps shared helper code out of the public headers. This avoids inventing a
  public helper API just to share render/routing/accessibility internals.
- `ui_source_structure_test` now guards the split so future work does not
  silently grow `ui.hpp` or `ui.cpp` back into monoliths.
- The largest remaining implementation unit is `runtime_events.cpp`; a later
  structure-only cleanup can split event routing, input/text, clipboard, and
  action/command-palette logic without changing the public API.

## 2026-07-03 Threaded Async Executor And Cancellation

- Step 218 keeps the existing manual `spawn_task(...)` / `complete_task(...)`
  queue and adds `spawn_background_task(...)` for cancellable background work
  that completes back on the main runtime queue.
- `TaskCancellationToken` is a small cooperative token over shared atomic
  state. `TaskHandle` now reports cancellation and can request cancellation
  through `cancel()`.
- Background tasks are backed by `std::jthread`; the runtime protects task
  records and the completion queue with a task-scoped mutex. Completion
  callbacks are still copied out and executed without holding that mutex, so
  callbacks can safely call normal runtime/context APIs.
- `WindowRuntime::~WindowRuntime()` requests cancellation and joins workers
  before member teardown, avoiding background threads reaching back into a
  partially destroyed runtime.
- Runtime diagnostics now expose task, active, queued, completed, cancelled,
  and background task counters. This makes the executor visible in the same
  diagnostics snapshot used by existing frame/platform state.
- This is a small threaded executor, not a full GPUI async runtime: task pools,
  priorities, async I/O integration, structured task groups, and cross-thread
  entity access remain future work.

## 2026-07-03 Asset And Image Pipeline Skeleton

- Step 217 adds deterministic in-memory RGBA8 bitmap assets, image asset ids,
  descriptors, and soft validation metadata without adding external PNG, JPEG,
  or SVG decoders.
- Image paint commands now flow through `PaintList::draw_image(...)`,
  `render_view(...)`, and `RenderFrame::draw_image(...)` with clip stack,
  composition stack, bounds, sampling, tint, and opacity metadata preserved for
  renderer-facing records.
- Frame statistics now count image commands, and the Vulkan diagnostic path
  treats image commands as a supported primitive for reporting, batching, image
  render summaries, submission plans, and upload byte/region counters.
- `vulkan_plan_image_uploads(...)` produces deterministic upload batches and
  regions from image assets. It is still an upload plan, not real Vulkan image
  allocation, GPU upload, shader sampling, texture cache lifetime management,
  or SVG/image decoder integration.
- Step 218 moves to threaded async executor and cancellation so background work
  can leave the current main-thread/manual-completion skeleton.

## 2026-07-03 Animation Clock And Tween Primitives

- Step 216 adds deterministic animation primitives without introducing a full
  GPUI animation subsystem yet: `AnimationEasing`, clamped progress, easing,
  float/color/transform/style tween helpers, and `StyleTween`.
- Runtime animation state is owned by `WindowRuntime` and driven through the
  existing deterministic timer path. `AnimationHandle` exposes id, active,
  completion, snapshot, and cancellation queries; `WindowRuntimeContext`
  forwards start/snapshot/cancel APIs.
- Animation snapshots report elapsed duration, linear progress, eased progress,
  easing mode, and completion. Repeating timer catch-up can invoke the same
  animation timer more than once for one coarse `advance_time(...)`; the
  runtime deduplicates callbacks per current runtime timestamp so tests see
  one animation tick per time advance.
- Animation callbacks run with the normal root `WindowRuntimeContext`, so
  callback-driven `request_render()` uses the existing invalidation and redraw
  path. The current test harness observes one redraw/paint per animation tick.
- Style opacity tweening normalizes the final opacity to a small decimal grid
  to keep deterministic exact-float tests stable, while color interpolation
  keeps the raw float path so color channel fractions remain exact for existing
  RGB/RGBA expectations.
- This slice is deterministic clock/tween infrastructure. It does not include
  transitions attached to element lifecycle, spring animation, compositor
  frame pacing, cancellation propagation across async work, or a full upstream
  GPUI animation graph. Step 217 moves to asset and image pipeline skeletons.

## 2026-07-03 Runtime Theme Inheritance And Switching

- Step 215 adds runtime-owned theme hierarchy on top of the existing inert
  `Theme` token map: an app theme provides defaults and per-window themes
  override individual color/spacing tokens by `WindowRuntimeId`.
- `WindowRuntimeContext` now carries its `window_runtime_id`, so root and
  additional-window contexts can resolve theme tokens through the same
  window-then-app fallback path.
- `AppContext`, `WindowRuntime`, and `WindowRuntimeContext` all expose theme
  setters, clearers, and token lookups. Missing tokens continue to soft-fail
  with `std::nullopt`.
- Theme changes request full render/layout/paint invalidation and redraw.
  `clear_invalidation()` now also clears pending redraw bookkeeping, making it
  a complete observable reset point for tests and diagnostics between theme
  changes.
- This is runtime theme storage and lookup, not a full design-system cascade
  installation into every element style. Step 216 moves to deterministic
  animation clock and tween primitives.

## 2026-07-03 Additional Window Lifecycle Cleanup

- Step 214 makes app-opened child-window close deterministic instead of only
  flipping the child record inactive.
- `WindowRuntime::handle_native_additional_window_event(...)` now records the
  close lifecycle event before cleanup, so diagnostics still see the child
  window and renderer context that existed at close time.
- `cleanup_closed_additional_window(...)` removes the matching owned native
  child window from `native_additional_windows_`, removes and destroys an owned
  child root view, erases subscriptions for that root view, and clears the
  child record's window, renderer, active, and ownership flags while preserving
  the record as historical runtime metadata.
- The Step 213 child routing test needed its child-view counter snapshot moved
  before emitting close, because Step 214 intentionally destroys the owned
  child root during close. This avoided a WSL-only dangling-read failure while
  preserving the routing assertions.
- Renderer objects created by `run_app` still live in the app renderer owner
  vector until `run_app` exits; Step 214 releases the runtime record's renderer
  ownership pointer/flag rather than introducing a renderer-owner removal API.
  Full child render loops and production OS window-loop integration remain
  future multi-window depth.

## 2026-07-03 Additional Window Event Routing

- Step 213 adds record-specific child-window event routing for app-opened
  native windows keyed by `WindowRuntimeId`.
- `WindowRuntime` can now build a `WindowRuntimeContext` from a child
  `WindowRuntimeRecord`, route child focus/pointer/keyboard events to the
  child root view, and emit `EventDispatchRecord`s whose context and route use
  the child root `ViewId` instead of the root window view.
- Child redraw requests now render through the child renderer and child root
  view, and child resize events forward to the child renderer while updating
  the child descriptor's logical size. Child close events mark only the child
  record inactive and emit child-routed lifecycle diagnostics without quitting
  the app.
- This is routing, not full child-window lifetime ownership. Closed child
  roots, subscriptions, native windows, and renderers are still retained until
  the broader runtime cleanup path runs. Step 214 moves to deterministic
  child-window lifecycle teardown.

## 2026-07-03 Additional Window Renderer Ownership

- Step 212 moves app-opened child windows from native-window-only records to
  records with independent child renderers created from each child
  `PlatformWindow` native surface, framebuffer size, and scale.
- `run_app` now retains all renderers produced by the app renderer factory in a
  vector, so root and app-opened child renderers share the same ownership
  boundary without changing the public `RendererFactory` pointer-returning
  contract.
- Child renderer creation remains graceful: if the factory fails or returns an
  empty renderer, the child record stores `native_window_error`, clears the
  transient window/renderer pointers, and avoids adding the native child window
  to the active native window list.
- This still does not implement child-window event routing, render loops, or
  lifecycle teardown. Step 213 moves to routing pointer, keyboard, focus,
  redraw, resize, and close events by `WindowRuntimeId`.

## 2026-07-03 Accessibility Value And Live Update Events

- Step 211 adds `PlatformAccessibilityLiveUpdateKind` and
  `PlatformAccessibilityLiveUpdate` as platform-neutral event records for
  value, text, and focus changes at the platform accessibility boundary.
- `WindowRuntime` now keeps the previous platform accessibility update and
  emits a `live_updates` batch by comparing same-element node values, text, and
  focused state across redraw-driven accessibility snapshots.
- Win32 UIA and Wayland AT-SPI adapters retain the latest live update batch
  alongside their provider/object facade node records, so later production
  event bridges can raise UIA/AT-SPI notifications from the same source of
  truth.
- This remains deterministic event metadata, not production accessibility
  event emission. No COM UIA provider events or D-Bus AT-SPI signals are
  emitted yet. Step 212 moves to additional-window renderer ownership.

## 2026-07-03 AT-SPI Object Model Facade

- Step 210 mirrors the Step 209 Win32 UIA provider-node facade on
  Linux/Wayland. `WaylandAtspiAccessibilityAdapter` now builds
  `WaylandAtspiObjectNode` records from the latest
  `PlatformAccessibilityTreeUpdate` instead of only retaining summary counts.
- AT-SPI object paths are deterministic and element-id based:
  `/org/a11y/atspi/accessible/<element-id>`. Child records also retain optional
  parent object paths, so later D-Bus provider work can navigate the object
  model without inventing a separate public accessibility identity.
- The facade preserves role, name, text, value, enabled/focusable/focused
  state, bounds, and child counts while keeping the existing root/node/focus
  summary counters intact.
- This remains an internal object model, not production AT-SPI. It does not
  register on the accessibility bus, expose D-Bus object paths, implement
  AT-SPI interfaces, or emit accessibility events. Step 211 moves to
  accessibility value and live update event records shared by the platform
  adapters.

## 2026-07-03 UIA Provider Tree Facade

- Step 209 adds `PlatformAccessibilityNodeUpdate::value` so text-input
  accessibility nodes expose their editable value separately from name/text at
  the platform boundary.
- `platform_accessibility_update_from(...)` now maps text-input snapshot text
  into the platform `value` field while leaving non-text-input values empty.
- The Win32 UIA adapter now owns internal `Win32UiaProviderNode` records built
  from the latest `PlatformAccessibilityTreeUpdate`. The facade retains stable
  element ids, parent ids, role, name, text, value, enabled/focusable/focused
  state, bounds, and child counts while preserving the existing root/node/focus
  counters.
- This remains a facade, not production UI Automation. It does not expose
  `IRawElementProvider*`, raise UIA events, implement navigation methods, or
  register COM provider objects. Step 210 mirrors the object-model facade on
  Linux/Wayland for AT-SPI.

## 2026-07-03 Platform Diagnostics Event Stream

- Step 208 adds `PlatformDiagnosticKind` and `PlatformDiagnosticEvent` as
  platform-neutral event metadata for platform-facing runtime hooks.
- `EventKind` moved from `ui.hpp` to `core/events.hpp`, so platform diagnostic
  metadata can live at the platform boundary without depending on UI-only
  declarations.
- `WindowRuntime` now keeps a bounded 32-event platform diagnostics stream,
  exposes it through `platform_diagnostics()`, and copies it into
  `RuntimeDiagnosticsSnapshot`; `WindowRuntimeContext` forwards the same
  read-only stream.
- The runtime records deterministic diagnostics for clipboard copy/cut/paste
  outcomes, drag/drop events and payload counts, IME placement updates,
  accessibility tree updates, window lifecycle dispatch, native-menu
  installation, and native file-dialog requests.
- This is an observability layer, not full native telemetry. Backend names are
  currently runtime/platform-result summaries, and deeper Win32 UIA, Wayland
  AT-SPI, menu, dialog, chrome, and renderer-backed diagnostics remain future
  production-depth work. Step 209 moves to the Win32 UIA provider facade.

## 2026-07-03 App Command Palette Registry

- Step 207 adds `CommandPaletteEntry` as platform-neutral command metadata:
  action name, title, group, intended action scope, enabled state, and optional
  view/element ids.
- `WindowRuntime`, `WindowRuntimeContext`, and `AppContext` can register
  command palette entries, enumerate the stable registry, filter by group, and
  dispatch a registered command through the existing scoped action registry.
- Disabled entries remain searchable but dispatch as unhandled without calling
  their action handler, and missing palette action names update the last action
  dispatch with an unhandled result.
- This is a registry and dispatch layer, not a full command palette UI. Fuzzy
  search, keyboard palette presentation, menu/palette unification,
  accelerator display, platform diagnostics, and native command routing remain
  future work. Step 208 moves to a bounded platform diagnostics event stream.

## 2026-07-03 Window Chrome Customization Skeleton

- Step 206 adds platform-neutral window chrome metadata:
  `WindowChromeOptions` now lives on `WindowDescriptor` with titlebar
  visibility, decoration, resizable, and transparent-background flags.
- `WindowOptions` exposes fluent `.titlebar_visible(...)`, `.decorations(...)`,
  `.resizable(...)`, and `.transparent(...)` helpers, and app-opened child
  windows preserve those descriptors through the runtime registry and native
  `PlatformApplication::create_window(...)` path.
- `PlatformWindowChromeState` records requested/applied chrome state, backend
  name, support state, and unsupported reason. The base `PlatformWindow`
  returns an explicit unsupported result so fallback backends stay graceful.
- Win32 computes window `style` / `extended_style` from the chrome request,
  uses those styles during `CreateWindowExW`, and can reapply them through
  `apply_window_chrome(...)`. Wayland records the request and applied default
  chrome while reporting unsupported xdg-decoration behavior until decoration
  negotiation is implemented.
- This is still a skeleton. Full frameless hit testing, resize grips,
  drag-to-move regions, Wayland xdg-decoration/client-side decoration policy,
  transparent swapchain/compositor behavior, and native runtime diagnostics
  remain future platform-depth work. Step 207 moves to command palette metadata
  over the existing action registry.

## 2026-07-03 Native File Dialog API Skeleton

- Step 205 adds platform-neutral native file dialog descriptors:
  `NativeFileDialogKind`, `NativeFileDialogFilter`,
  `NativeFileDialogOptions`, and `NativeFileDialogResult`.
- The public surface can express open-file, open-files, and save-file requests
  with title, default directory, suggested name, and extension filters. Results
  carry support/acceptance state, backend name, requested kind, selected paths,
  optional error text, and filter count diagnostics.
- `AppContext`, `WindowRuntimeContext`, and `WindowRuntime` now forward
  `show_native_file_dialog(...)` through `PlatformApplication` and retain the
  last dialog result for frame/runtime inspection.
- Win32 and Wayland own inert native file-dialog state objects that report
  `backend = "win32"` or `backend = "wayland"` plus requested kind and filter
  count while returning `supported = false`. Real `IFileOpenDialog`/
  `IFileSaveDialog`, xdg-desktop-portal, non-blocking dialog lifetimes, and
  sandbox/desktop integration remain future platform-depth work.

## 2026-07-03 Native Menu And Accelerator API Skeleton

- Step 204 adds the first platform-neutral native menu surface:
  `NativeMenuModel`, `NativeMenuItem`, `NativeMenuItemKind`,
  `NativeMenuAccelerator`, `PlatformMenuInstallationResult`, and
  `NativeMenuInstallation`.
- `AppContext`, `WindowRuntimeContext`, and `WindowRuntime` can now install a
  native menu model and retain the last installation record. The retained model
  keeps submenu hierarchy, command action names, enabled/checked state, and
  accelerator metadata visible to later command-palette and native-menu depth
  work.
- Win32 and Wayland now both own inert native menu state objects that report
  backend names, root menu count, recursive item count, and accelerator count
  while returning `supported = false`. This preserves graceful behavior until
  real Win32 `HMENU`/accelerator tables and Wayland/desktop-shell menu
  integration are designed.
- Recursive `native_menu_item_count(...)` and
  `native_menu_accelerator_count(...)` live at the platform-neutral boundary,
  so Step 205 can add file dialog request/result APIs without reopening menu
  model counting or runtime forwarding.

## 2026-07-03 Win32 OLE Drop Target Skeleton

- Step 203 adds an internal `Win32OleDropTarget` implementing `IDropTarget`,
  with `QueryInterface`/`AddRef`/`Release` and `DragEnter`/`DragOver`/
  `DragLeave`/`Drop` methods that route into the existing platform drag events.
- `Win32Application` now initializes OLE, `Win32Window` registers and revokes
  the drop target with per-window `Win32OleDropTargetRegistrationState`
  diagnostics, and the Win32 platform target links `ole32` alongside the
  existing Win32 system libraries.
- `drag_payload_from_ole_data_object(...)` creates the conversion boundary for
  `CF_UNICODETEXT` and `CF_HDROP`, while `drag_action_from_drop_effect(...)`
  maps `DROPEFFECT_COPY` and `DROPEFFECT_MOVE` into the public
  `DragDropAction` metadata added in Step 200.
- The deterministic Win32 drag/drop test hook now carries a drop effect, so
  text enter/update and file drop coverage verifies public copy/move action
  metadata without relying on a real shell drag gesture.
- This remains a skeleton, not full production OLE drag/drop. Richer effect
  negotiation, non-text/non-file formats, async shell edge cases, and deeper
  user-facing drag policy remain future Windows platform-depth work. Step 204
  moves to native menu and accelerator API scaffolding.

## 2026-07-03 Wayland XDG Configure Lifecycle State

- Step 202 adds deterministic Wayland XDG configure lifecycle records:
  `WaylandXdgConfigureState` tracks pending size, pending serial, last acked
  serial, and current/pending toplevel state; `WaylandXdgToplevelState` tracks
  activated, maximized, and fullscreen flags parsed from the compositor state
  array.
- `handle_surface_configure(...)` now records the acked serial before updating
  configured state, and lifecycle dispatch uses existing public events:
  activation changes emit `WindowActivated`, while leaving maximized/fullscreen
  state emits `WindowRestored`. Resize delivery still waits for the matching
  surface configure path.
- The Wayland test compositor can now send stateful resize configures and
  report the last configure state, including the serial observed by
  `ack_configure`. The resize test asserts size, activation, max/fullscreen
  flags, and exact ack serial instead of only checking a boolean ack.
- This is still a deterministic lifecycle slice, not full shell-policy
  integration. It does not yet add native maximize/fullscreen APIs, window
  state queries in the public `WindowState`, tiled states, compositor serial
  policy, or production configure-loop edge handling. Step 203 pivots to the
  Win32 OLE drop target skeleton.

## 2026-07-03 Wayland Cursor Theme Image State

- Step 201 adds deterministic internal Wayland cursor theme/image state records
  before real cursor-theme loading exists. `WaylandCursorThemeState` records the
  current theme status, requested shape, mapped cursor name, serial, apply
  count, hotspot, and graceful unavailable reason.
- The cursor-shape mapping now has explicit Wayland names for default arrow,
  pointing hand, text, crosshair, horizontal/vertical resize, and not-allowed
  cursors. The runtime still calls the existing null `wl_pointer_set_cursor`
  path, so this slice improves observability and state ownership without
  claiming loaded cursor images.
- The Wayland pointer test now requests a pointing-hand cursor during drag
  enter and expects an additional compositor-visible cursor set request,
  proving cursor application records are updated outside the initial pointer
  enter path.
- Real `libwayland-cursor`/theme loading, cursor surfaces, shared-memory cursor
  buffers, animated cursors, scale-aware images, and compositor edge cases
  remain future Wayland platform-depth work. Step 202 moves to XDG configure
  lifecycle state.

## 2026-07-03 Wayland Drag Action Negotiation

- Step 200 adds public `DragDropAction::{none, copy, move}` metadata to
  `DragEntered`, `DragUpdated`, `DragDropped`, and `DragExited`, keeping the
  default action as `none` for existing call sites.
- `WaylandDataDevice` now records `wl_data_offer.source_actions` and
  `wl_data_offer.action`, accepts the preferred supported MIME type, advertises
  destination copy/move support through `wl_data_offer_set_actions`, and calls
  `wl_data_offer_finish` after drop payload extraction.
- The Wayland test compositor now sends source/selected DnD action events and
  records client-side `accept`, `set_actions`, and `finish` requests. The
  platform test covers text move and URI-list copy flows, while the runtime
  test verifies action metadata survives normal event routing.
- The slice intentionally maps only Wayland copy/move/none into the public API.
  Wayland ask actions, richer drag-effect policy, non-local URI handling, and
  production shell edge cases remain future platform-depth work.

## 2026-07-03 Wayland Clipboard Ownership And Send Offers

- Step 199 promotes standalone `WaylandClipboard::Connection::write_text(...)`
  from memory fallback to a real Wayland selection owner when connected to an
  available data device. It creates a `wl_data_source`, offers
  `text/plain;charset=utf-8` and `text/plain`, and installs it with
  `wl_data_device_set_selection`.
- The clipboard connection keeps a small dispatch loop alive while it owns a
  source so compositor `wl_data_source.send` events can be received after
  `write_text(...)` returns. The send callback copies the current owned UTF-8
  payload under a mutex and writes it to the compositor-provided fd.
- `MemoryClipboard` remains the fallback/read-back path, so unsupported,
  no-seat, or failed protocol write cases keep the existing local clipboard
  contract while connected Wayland displays now have a protocol-backed write
  path.
- The Wayland test compositor now models both directions of clipboard data:
  compositor-owned `wl_data_offer` payloads for reads, and client-owned
  `wl_data_source` selection with offered MIME types and deterministic payload
  requests for writes.
- This slice still does not implement non-text formats, serial policy beyond
  the deterministic test path, clipboard manager persistence after process
  exit, or richer desktop edge cases. Step 200 moves to Wayland drag action
  negotiation rather than expanding clipboard formats.

## 2026-07-03 Text Soft Wrap Layout Records

- Step 198 adds `TextWrapLine`, `TextWrapLayout`, and
  `wrap_text_measurement(...)` in `include/cgpui/ui/text.hpp`. The current
  algorithm is deterministic greedy glyph-level wrapping over already-shaped
  fallback glyph advances; it does not split glyphs or introduce word/bidi
  paragraph layout.
- `PaintList::fill_text(...)` now derives wrap records from the measured text
  and authored paint width, expands text paint height when wrapped content
  needs more lines, and emits wrap-aware glyph origins. Existing single-line
  text remains one line with unchanged glyph positions when it fits the paint
  width.
- `TextPaint` and `TextDraw` carry the same line records, giving renderer
  tests and future Vulkan text work a stable view of wrapped byte ranges,
  glyph ranges, relative line origins, and line sizes.
- `TextElement` and `LabelElement` layout now size text through
  `wrap_text_measurement(...)` using the current max-width constraint. This is
  still a fallback-metric soft-wrap skeleton, not rich paragraph layout,
  platform shaping, bidi visual lines, selection geometry over wraps, or
  word-aware wrapping.

## 2026-07-03 Text Pointer Selection Geometry

- Step 197 adds deterministic single-line pointer hit geometry in
  `include/cgpui/ui/text.hpp`: `TextHitTestResult`,
  `hit_test_text_position(...)`, and `text_selection_range_from_points(...)`
  map measured glyph advances to byte offsets and normalized selection ranges.
- `WindowRuntime` now keeps a small text pointer drag state for text-input
  elements. Left pointer down focuses the input through the existing focus path
  and collapses the model selection to the hit offset; pointer move/up continue
  selecting against the original text-input element even when the pointer moves
  outside its bounds.
- The slice intentionally remains single-line and fallback-metric based. It
  does not add soft wrapping, bidi visual order, grapheme column accounting,
  paragraph layout, platform shaping, or selection handles. Step 198 now builds
  wrap records on top of the reusable measurement primitives while wrapped
  selection geometry remains future work.

## 2026-07-02 Linux AT-SPI Accessibility Adapter Skeleton

- Step 177 mirrors the Step 176 Win32 adapter boundary on Linux/Wayland. The
  new `WaylandAtspiAccessibilityAdapter` consumes
  `PlatformAccessibilityTreeUpdate` and tracks root element id, total nodes,
  focused nodes, and text-input nodes.
- The adapter is stored inside `WaylandWindow`, and
  `RegisteredWaylandWindow::update_accessibility_tree(...)` forwards platform
  updates into the underlying Wayland window. This matters because the runtime
  owns the registered wrapper, not the raw `WaylandWindow`.
- The slice intentionally avoids D-Bus, AT-SPI object paths, role mapping to
  AT-SPI enums, event emission, or desktop accessibility bus registration.
  Windows and Linux now have matching adapter skeletons, while production
  accessibility bridges remain future work.
- Step 178 can move to native additional-window creation without revisiting the
  public accessibility snapshot surface.

## 2026-07-02 Windows UIA Accessibility Adapter Skeleton

- Step 176 expands the platform accessibility update from a summary-only root
  id/node count into node records derived from `AccessibilityTreeSnapshot`.
  Each platform node carries element id, optional parent id, role, name, text,
  enabled/focusable/focused state, optional layout bounds, and child count.
- `WindowRuntime::handle_redraw()` now forwards the current accessibility
  snapshot to `PlatformWindow::update_accessibility_tree(...)` after layout,
  keeping platform adapters fed by the same tree used by public
  `accessibility_snapshot()` queries.
- The Win32 backend now has a `Win32UiaAccessibilityAdapter` skeleton that
  consumes the platform update and tracks root, total-node, focused-node, and
  text-input-node counts. This intentionally does not create COM objects,
  expose `IRawElementProvider*`, or claim production UI Automation parity yet.
- Step 177 should mirror this boundary on Linux with an AT-SPI adapter
  skeleton consuming the same platform update rather than adding
  Linux-specific public accessibility semantics.

## 2026-07-02 Glyph Bitmap And Fallback Rasterizer

- Step 169 adds CPU-side fallback raster data in `include/cgpui/ui/text.hpp`,
  not platform font rasterization or Vulkan texture upload. The new data model
  is `GlyphBitmap`, `GlyphRasterizerOptions`, and `RasterizedGlyph`.
- `rasterize_fallback_glyph(...)` consumes the existing `TextGlyphPaint`
  metadata from Step 151/152. It preserves the `GlyphAtlasKey`, uses
  `device_advance` for bitmap width, uses `key.device_font_size` for bitmap
  height, records the same advance/device font size, and derives a stable
  fallback baseline at 80% of device font size.
- The fallback bitmap is an alpha-only buffer with deterministic padding and
  foreground/background alpha options. This is enough for Step 170 to pack and
  upload bytes into atlas records without depending on DirectWrite,
  fontconfig, HarfBuzz, or real Vulkan images yet.
- Header cleanliness coverage now proves the glyph raster data model is usable
  through both UI text headers and renderer-facing includes. The renderer still
  stores placeholder atlas bounds until Step 170 teaches the glyph cache to
  allocate atlas slots from `RasterizedGlyph` bitmap data.

## 2026-07-02 Steps 169-178 Depth Pass Planning

- The next ten-step track should continue Windows/Linux depth work instead of
  pivoting to macOS. The current parity audit identifies Vulkan text rendering,
  Wayland payload/protocol handling, native accessibility adapters, and native
  multi-window creation as higher-leverage blockers for the existing target
  pair.
- Step 169 should start with CPU-side glyph bitmap and deterministic fallback
  rasterization in `include/cgpui/ui/text.hpp`. Existing Step 151/152 work
  already provides glyph keys, glyph paint metadata, and renderer cache
  lookup/store, but atlas entries still use placeholder bounds derived from
  glyph positions instead of actual raster data.
- Step 170 can then allocate atlas slots and upload records from rasterized
  glyphs without needing real Vulkan image upload yet. This keeps renderer
  state testable before GPU texture objects become necessary.
- Steps 173-175 should deepen Wayland using the existing test compositor. The
  current data-device and text-input code already routes skeleton events, so
  the next useful work is MIME offer tracking, text/URI payload extraction,
  and protocol-independent text-input state records.
- Steps 176-177 should consume the platform-neutral accessibility snapshot for
  UIA/AT-SPI adapter skeletons. They should not add separate public
  accessibility semantics until the adapter boundary proves a missing concept.

## 2026-07-02 GPUI-Core API Parity Audit

- Step 168 adds `docs/gpui-core-api-parity.md` as the Windows/Linux truth
  source for current GPUI-core-like parity. It deliberately separates
  Implemented, Partial, Missing, and Mac/Metal Deferred areas instead of
  claiming full upstream GPUI parity.
- Implemented areas now include the public prelude, `Context<T>`, `Entity<T>`,
  `WindowOptions`, entity/global/action/subscription/defer/timer/async
  runtime APIs, keyed element behavior, lifecycle/state storage, reusable
  widgets (`button`, `label`, `text_input`, `scrollable_list`), style/layout
  primitives, text/glyph metadata, renderer diagnostics, Win32/Wayland platform
  hooks, and Windows/Linux demo smoke coverage.
- Partial areas are explicit: Vulkan text still needs real glyph
  raster/upload/draw, accessibility is snapshot-level without UIA/AT-SPI,
  Wayland clipboard/drag/drop/IME are skeletons, Win32 drag/drop is not full
  OLE shell integration, multi-window is registry-level, frame timings are not
  a real profiler, and platform font discovery is still skeletal.
- Missing areas are now named for the next depth pass: full animation,
  theming, asset/image pipelines, rich text editing, native menus/dialogs,
  production accessibility bridges, full Vulkan drawing for remaining
  primitives, virtualization, and threaded async/cancellation.
- Mac remains a separate parity handoff. The audit keeps macOS/Cocoa + Metal
  outside the Windows/Linux completion claim and requires a Mac host plus Cocoa
  platform adapters and Metal renderer parity before any Mac parity claim.

## 2026-07-02 Accessibility Tree Skeleton

- Step 166 adds a platform-neutral accessibility snapshot, not Windows UIA or
  Linux AT-SPI adapters. `AccessibilityTreeSnapshot` is built from
  `ElementTree` preorder traversal and carries root id, per-node parent/child
  ids, role, name, text, enabled/focusable/focused flags, and optional layout
  bounds.
- `Element` now exposes default accessibility role/name/text hooks. Labels and
  text elements report their text as name/text, text inputs report the text
  role as `text_input`, and buttons report role `button` with accessible name
  derived from their child label/text before falling back to action name.
- `WindowRuntime::accessibility_snapshot()` and
  `WindowRuntimeContext::accessibility_snapshot()` reuse the installed
  runtime-owned element tree and mark the current keyboard-focus element owner.
  No installed tree returns an empty snapshot.
- The platform hook remains intentionally low-coupling:
  `PlatformAccessibilityTreeUpdate` carries only root element id and node
  count, while `PlatformWindow::update_accessibility_tree(...)` defaults to
  no-op. Real UIA/AT-SPI tree serialization remains future platform work.
- Step 167 can now focus on demo smoke coverage without needing accessibility
  OS adapter work; Step 168 should count accessibility as snapshot-level
  partial parity, not full native accessibility parity.

## 2026-07-02 Multi-Window Runtime Registry

- Step 159 introduces a platform-neutral ownership registry, not a full native
  multi-window event loop. `WindowRuntimeRecord` tracks runtime id, descriptor,
  root view id, live platform window/renderer pointers when active, and
  explicit `owns_window`, `owns_renderer`, and `owns_root_view` flags.
- The root window keeps the existing single-window run path. Its record is
  active only while `WindowRuntime::run(...)` owns a live platform window and
  borrowed renderer pointer, then clears the live pointers when the run returns.
  This preserves current callback/context behavior.
- App-opened windows now receive stable `WindowRuntimeId` values in
  `AppOpenedWindow::runtime_id`. Owned root views are still stored in the
  existing view registry, while the new runtime record declares future window
  and renderer ownership separately from actual platform creation.
- The new query surface is read-only:
  `root_window_runtime_id()`, `window_runtime_records()`, and
  `window_runtime_record(...)`. This gives Step 160 lifecycle events a concrete
  per-window record to update without forcing Step 159 to rewrite Win32 or
  Wayland event-loop behavior.

## 2026-07-02 Renderer Unsupported-Command Diagnostics

- Step 158 adds a renderer-facing diagnostic report rather than changing UI
  paint routing or pretending unsupported primitives are rendered. Supported
  Vulkan primitives remain `solid_rect` and `text`; rounded rects,
  text-selection metadata, and text-caret metadata are now nameable
  `RendererPrimitiveKind` values that can be reported explicitly when a
  renderer does not handle them yet.
- `RendererCommandReport` keeps supported command batching and unsupported
  diagnostics together. This gives later frame diagnostics and renderer-depth
  work one surface to inspect without breaking the existing
  `vulkan_build_renderer_command_batches(...)` helper used by prior tests.
- Unsupported command diagnostics carry the primitive kind, original command
  index, `unsupported_primitive` reason, and a human-readable message including
  the primitive name. The report preserves supported command batches even when
  unsupported commands are interleaved in the same stream.
- The post-merge Windows full debug run initially saw a transient
  `win32_text_input_test/default` failure. A targeted rerun passed 1/1 and the
  full Windows debug rerun passed 29/29, matching the feature-worktree Windows
  full debug result. WSL Arch Linux full debug passed 26/26.
- Step 159 should now move into Band H with multi-window runtime ownership.
  The renderer diagnostic surface is intentionally backend-facing and should
  not force platform window registry design to depend on Vulkan internals.

## 2026-07-01 Paint Command Snapshots

- Step 157 keeps paint snapshot serialization test-only in
  `tests/ui/paint_snapshot.hpp`; it does not add public API surface or change
  renderer/runtime ownership.
- The snapshot format captures command index, primitive kind, logical rects,
  colors, text content/font/device font size/glyph count, clip metadata,
  opacity, and affine transform. This gives later renderer changes a stable
  evidence trail for command order and metadata propagation.
- Widget coverage currently fixes a combined button/label/text-input paint
  stream, including rounded background, border order, text selection, text
  command, and caret command. `render_view_test` separately snapshots the
  renderer-submitted text command after `render_view(...)` translates
  `PaintList` output into `TextDraw`.
- The hello demo smoke marker is source-level and inert by default:
  `CGPUI_DEMO_PAINT_SNAPSHOT_SMOKE` requests a redraw when present, giving
  Step 167 a stable hook for future Windows/Linux demo smoke expansion without
  changing normal demo behavior.

## 2026-07-01 HiDPI Scale Propagation

- Step 156 makes the runtime store both framebuffer size and logical viewport
  size. Platform resize events still deliver framebuffer dimensions plus
  `DpiScale`; `WindowRuntime` derives `viewport_size = framebuffer / scale`
  for view context, layout, and paint.
- Authored sizes remain logical pixels. `LayoutInput::scale` and
  `to_logical_pixels(...)` / `to_device_pixels(...)` expose the conversion
  boundary without requiring existing element builders or style APIs to switch
  units.
- Text shaping now preserves logical `font_size`, `total_advance`, glyph
  origins, and glyph advances while also exposing device font size, device
  total advance, device origins, and device advances. Vulkan glyph cache
  entries consume the device-space glyph metadata so renderer resources can be
  scale-aware without changing text authoring APIs.
- Step 157 can use the new logical/device paint metadata to serialize stable
  paint command snapshots for widgets and the demo.

## 2026-07-01 Frame Statistics Diagnostics

- Step 155 keeps frame timing deterministic for tests: `FrameStatistics`
  exposes timing fields, but the current runtime-populated timings remain zero
  until a later real profiler/timer integration is added.
- The first useful statistics layer is structural rather than temporal:
  layout, paint, and render pass counts; emitted, submitted, and skipped
  command counts; primitive counts; and begin/clear/present counters are
  derived from the actual render record and command stream.
- `render_view(...)` accepts an optional statistics output pointer, preserving
  old call sites while letting `WindowRuntime::handle_redraw()` store the same
  frame record in `last_render_record_` and diagnostics. This keeps
  diagnostics observable without turning every render helper into a profiling
  API.
- Step 156 should thread HiDPI scale through layout, text metrics, and
  renderer resize metadata while keeping authored dimensions in logical pixels.

## 2026-07-01 Renderer Command Batching

- Step 154 keeps batching diagnostic-only. `RendererCommandBatchKey` is the
  shared public key shape over `RendererPrimitiveKind`, optional clip rect, and
  `PaintMetadata`; it does not describe a GPU pipeline state object yet.
- `vulkan_build_renderer_command_batches(...)` groups commands in stable
  submitted order, first solid rectangles and then text draws, and only merges
  adjacent commands with identical primitive kind, clip rect, opacity, and
  transform metadata. It deliberately does not reorder commands to chase larger
  batches because paint order is still authoritative.
- `VulkanRendererState::present_frame(...)` records the latest command batches
  after consuming text glyph cache metadata and before command-buffer recording.
  The current Vulkan path still only clears solid rectangles; text and batching
  remain diagnostic surfaces for later frame statistics and renderer-depth
  steps.
- Step 155 should extend diagnostics with frame timing and layout/paint/render
  counters using these command-batch records as renderer-facing evidence, while
  keeping real timing optional/deterministic for tests.

## 2026-07-01 Font Database Skeleton

- Step 149 introduces the font API as platform-neutral descriptors and an
  in-memory `FontDatabase`, not as real system font enumeration yet.
  `FontFaceDescriptor` currently carries `FontDescriptor`, PostScript name,
  source, and optional path.
- `FontDatabase::add_face(...)` ignores empty families and suppresses exact
  duplicate face descriptors. `resolve(...)` matches by family, while an empty
  request family soft-falls back to the first available face.
- `discover_test_fonts(...)` is the deterministic fake discovery route for
  tests and later shaping work. It preserves fixture order through the same
  database insertion rules used by platform discoveries.
- `PlatformApplication::discover_fonts()` is declared with only a forward
  declaration of `FontDatabase` in `platform.hpp`; the default empty
  implementation lives out-of-line in `src/platform/empty.cpp`. This avoids
  making every platform header consumer include UI text definitions.
- Win32 and Wayland now override `discover_fonts()` but intentionally return
  empty databases. Real DirectWrite/fontconfig-style discovery remains a later
  depth step; Step 150 can consume these descriptors for deterministic fallback
  shaping without depending on real platform font enumeration.

## 2026-07-01 Scrollable List Container

- Step 148 keeps the scrollable list as a public reusable widget built on
  existing primitives: `ScrollState`, `VerticalStackElement`, keyed elements,
  layout bounds, and paint clip metadata. It does not introduce a separate
  runtime scroll owner or virtualization system yet.
- `scrollable_list(state)` installs each `.item(key, child)` by setting the
  child element key and storing it in an internal vertical stack. This makes
  item identity visible to keyed reconciliation without inventing a second
  list-specific key map.
- Layout computes content size through the internal stack, writes viewport and
  content dimensions back to `ScrollState`, and applies the current scroll
  offset to child layout bounds so tests and paint commands observe scrolled
  geometry.
- Paint pushes the list viewport as a clip around direct item painting. The
  internal `VerticalStackElement` remains a layout container rather than a
  paint dispatcher, so the list paints its items explicitly.
- Step 149 should switch focus to text/font infrastructure: platform-neutral
  font descriptors and deterministic Win32/Linux discovery skeletons, while
  keeping renderer text work command-driven until later glyph cache slices.

## 2026-07-01 Text Input Widget Primitive

- Step 147 adds `TextInputElement` as a focusable widget backed by an existing
  `TextModel`; it reuses `TextElement` layout and caret/selection/text paint
  metadata instead of adding a second editable text rendering path.
- `text_input(model)` returns a fluent `TextInputBuilder` with text style,
  key, enabled, and disabled support. The widget is intentionally model-backed
  by reference, so ownership remains with the caller/runtime authoring code.
- `WindowRuntime::focused_text_model()` now preserves explicit
  `bind_text_model(...)` behavior first, then falls back to the focused
  installed `TextInputElement` model. This lets text input widgets receive text
  input, text edit bindings, clipboard copy/cut/paste, and IME geometry without
  requiring authors to manually bind the model to the element id.
- `TextInputElement::handle_event(...)` consumes text input, IME composition,
  and left pointer press events after runtime routing has applied the shared
  edit behavior. It does not introduce a private widget-specific editor loop.
- Step 148 should build the scrollable list container on top of the existing
  `ScrollState`, `ScrollElement`, keyed reconciliation, and viewport clip
  metadata rather than expanding text-input behavior further.

## 2026-07-01 Label Widget Primitive

- Step 146 keeps `LabelElement` as a read-only text widget over owned
  `std::string` content, not as a bound `TextModel` or editable text control.
- `label(...)` emits a single `PaintCommandKind::text` command when it has
  non-empty text and layout bounds. It deliberately omits
  `text_selection` and `text_caret` commands so label paint output cannot be
  confused with editable text input metadata.
- Label text styling uses the same `Style` fields as other text paths:
  foreground color, `FontDescriptor`, and font size. Missing foreground falls
  back to the current default label text color rather than adding theme or
  cascade lookup in this slice.
- The builder supports `.key(...)`, `.enabled(...)`, and `.disabled()` so label
  widgets fit keyed reconciliation and disabled subtree conventions, even
  though labels do not handle input directly.
- Step 147 should introduce an editable text input widget by composing the
  existing focus, text model, selection, clipboard, key-edit, and IME geometry
  surfaces. It should not retrofit editability into `LabelElement`.

## 2026-07-01 FocusHandle Primitive

- Step 144 keeps `FocusHandle` deliberately thin: it stores only an
  `ElementId` and forwards request/release to the existing
  `WindowRuntime::request_keyboard_focus(ElementId)` and
  `release_keyboard_focus(ElementId)` owner semantics.
- `FocusHandle::contains(...)` and `focused(...)` query the current
  `ViewInputState::keyboard_focus_element_owner`; for this primitive slice the
  two spellings are equivalent. The duplicate naming matches the GPUI-like API
  shape while leaving room for richer containment/focus scopes later.
- `WindowRuntime::input_state()` is now a public snapshot helper so handles and
  later widgets can query focus without requiring an event-time
  `WindowRuntimeContext`.
- Step 144 intentionally does not add focus rings, focus scopes, tab-order
  changes, platform focus adapters, or accessibility focus integration. Those
  remain later widget/platform/accessibility slices.

## 2026-07-01 Async Task Completion Skeleton Merged

- Step 136 is merged on `master` at
  `e957c6e feat: add async task completion skeleton` and post-merge verified
  on Windows and WSL Arch Linux.
- Step 137 should add runtime update batching on top of the existing
  model/global update and redraw scheduling paths without changing the Step
  136 deterministic completion queue or adding platform wakeups early.

## 2026-07-01 Async Task Completion Skeleton

- Step 136 keeps async work deterministic and runtime-local: `TaskId`,
  `TaskHandle`, `TaskCompletionCallback`, `spawn_task(...)`,
  `complete_task(...)`, and `drain_task_completions()` define the public
  skeleton without adding a thread pool, coroutine runtime, or platform wakeup
  integration yet.
- Completion injection is explicit in tests: `complete_task(id)` marks a task
  queued and inactive, but does not run the callback until
  `drain_task_completions()` executes on the runtime thread while the window
  and renderer are live.
- Completion callbacks drain FIFO, see a normal `WindowRuntimeContext`, and
  share the same redraw deferral discipline as deferred callbacks and timers.
  Duplicate completion of an already completed task soft-fails.
- Step 136 intentionally does not implement cancellation, background
  scheduling, thread safety, or platform event-loop wakeups. Those remain out
  of scope until later runtime/platform slices.

## 2026-07-01 Timer API

- Step 135 keeps timers deterministic and runtime-local: `TimerId`,
  `TimerCallback`, `schedule_timer(...)`, `schedule_repeating_timer(...)`,
  `cancel_timer(...)`, and `advance_time(...)` live on the shared runtime/
  context surface without adding real OS timer backends yet.
- Timer callbacks run while `WindowRuntime` still has a live window and
  renderer. Tests therefore drive `advance_time(...)` from the fake
  application's `run()` callback rather than after `WindowRuntime::run(...)`
  returns.
- One-shot timers are removed before their callback runs, repeating timers
  reschedule by their interval, and `request_render`/`request_layout`/
  `request_paint` inside timer callbacks defer a redraw until timer firing
  finishes. This mirrors Step 134's event/deferred-callback redraw discipline.
- Step 135 intentionally does not add platform event-loop wakeups, async task
  handles, batching, or diagnostics. Those remain Steps 136-138 and 165.

## 2026-07-01 Deferred Callback Queue Merged

- Step 134 is merged on `master` at
  `64f1614 feat: add deferred callback queue` and post-merge verified on
  Windows and WSL Arch Linux.
- Step 135 should build timer ids and deterministic ticking on top of the
  runtime loop without changing the Step 134 event-turn defer ordering or
  introducing platform wakeups early; platform wakeups remain Step 165.

## 2026-07-01 Deferred Callback Queue

- Step 134 keeps deferred work runtime-owned and deliberately small:
  `DeferredCallback` is a public `std::function<void(const
  WindowRuntimeContext&)>` spelling, `WindowRuntimeContext::defer(...)`
  forwards to `WindowRuntime`, and the runtime drains callbacks after
  `after_event_callback_` but before the deferred redraw request is flushed.
- Deferred callbacks are drained FIFO in batches. Callbacks queued by a
  deferred callback run in a later drain iteration before the runtime leaves
  the event turn, which keeps nested `cx.defer(...)` behavior deterministic
  without introducing timers or async task handles early.
- Redraw scheduling now treats `draining_deferred_callbacks_` like event
  dispatch: `request_render`, `request_layout`, and `request_paint` made from a
  deferred callback set the deferred redraw flag and request one redraw after
  the queue is drained.
- Step 134 intentionally does not add timer ids, async task handles, platform
  wakeups, batching, or diagnostics. Those remain Steps 135-138 and 165.

## 2026-07-01 Subscription Ownership Token Merged

- Step 133 is merged on `master` at
  `77293cb feat: add subscription ownership token` and post-merge verified on
  Windows and WSL Arch Linux.
- Step 134 should build deferred callbacks on top of the runtime/context
  capability surface without changing the new subscription token lifetime
  contract or legacy permanent observer behavior.

## 2026-07-01 Subscription Ownership Token

- Step 133 adds an explicit owned-observer path instead of changing the legacy
  `observe_model(...) -> bool` API. The legacy API remains a permanent
  observer registration so existing Step 102/132 behavior and demo authoring
  code do not silently disconnect at the end of an expression.
- `Subscription` is a move-only RAII token over `SubscriptionId`; dropping the
  token calls `release()`, and explicit `WindowRuntime::remove_subscription`
  clears the observer callback. Releasing/removing an already disconnected or
  unknown subscription soft-fails with `false`.
- Owned observer removal tombstones the callback rather than erasing the vector
  entry. This keeps notification iteration stable if a subscription is removed
  while callbacks are being processed.
- `subscriptions_for_view(...)` is intentionally unchanged. Step 133 owns
  observer lifetime tokens, not automatic view dependency tracking, reactive
  dependency inference, deferred callbacks, timers, or async completions.

## 2026-07-01 Scoped Action Registry Merged

- Step 132 is merged on `master` at
  `7de89c7 feat: add scoped action registry` and post-merge verified on
  Windows and WSL Arch Linux.
- Step 133 should add subscription ownership/disconnect semantics on top of the
  existing observer list without changing Step 132 action scope lookup, legacy
  action compatibility, or `ActionDispatchResult` metadata.

## 2026-07-01 Scoped Action Registry

- Step 132 keeps the old `register_action(name, handler)` spelling as the
  app/global action scope, so existing key bindings and demo authoring code stay
  source-compatible.
- Scoped dispatch now has explicit metadata through `ActionScope` and
  `ActionDispatchResult::scope`, `view_id`, and `element_id`. Missing actions
  still return `handled == false` with no scope or owner metadata.
- Lookup order is focused element, current/target view, window, then app. The
  view lookup uses the current event route when dispatch happens inside event
  handling and falls back to the root view outside an event route.
- `ViewContext` exposes explicit `register_app_action`,
  `register_window_action`, `register_view_action`, and
  `register_focused_element_action` helpers while preserving the older
  `register_action` helper as app/global scope forwarding.
- This step deliberately does not add subscription lifetime tokens, deferred
  callbacks, timers, async completions, or redraw batching; those remain Steps
  133-137.

## 2026-07-01 Global App State Registry Merged

- Step 131 is merged on `master` at
  `54bcec4 feat: add global app state registry` and post-merge verified on
  Windows and WSL Arch Linux.
- Step 132 should build scoped actions on top of the existing action dispatch
  path without changing the new global registry semantics; globals remain
  passive typed state until Step 137 adds update batching/redraw behavior.

## 2026-07-01 Global App State Registry

- Step 131 keeps global state runtime-owned and typed by `std::type_index`,
  mirroring the existing entity-store pattern without introducing a separate
  app object or ownership layer.
- `AppContext` and `ViewContext` expose the same `set_global`, `global`, and
  `update_global` spelling, so setup-time globals are visible to frame/event
  context code through one shared runtime store.
- Missing global reads return `nullptr`, and missing global updates return
  `false`. Global updates do not schedule redraw yet; Step 137 owns update
  batching and redraw coalescing for model/global changes.

## 2026-07-01 Entity Handle Convenience API Merged

- Step 130 is merged on `master` at
  `57e103a feat: add entity handle convenience` and post-merge verified on
  Windows and WSL Arch Linux.
- The next Step 131 global registry should reuse the existing context/runtime
  capability style: typed helpers over one runtime-owned store, soft-fail
  missing lookups, and no new app ownership layer.

## 2026-07-01 Entity Handle Convenience API

- Step 130 keeps `EntityHandle<T>` as a lightweight typed id wrapper, not a new
  ownership model. The runtime remains the source of truth for entity storage,
  liveness, notification, and invalidation.
- `EntityHandle<T>::read(cx)` delegates to the existing context entity read
  helper, while `update(cx, fn)` delegates to the existing notifying update
  path. This intentionally gives entity handles the same subscribed-view
  invalidation behavior already used by model updates.
- `downgrade()` returns the existing `WeakEntity<T>` shape, preserving the
  soft-fail upgrade semantics from Step 101 instead of introducing a second
  weak handle type.

## 2026-07-01 Context Authoring Alias Merged

- Step 129 is merged on `master` at
  `d1576fe feat: add context authoring alias` and post-merge verified on
  Windows and WSL Arch Linux.
- The public context authoring surface now has the GPUI-like spelling
  `Context<T>` while preserving the existing `ViewContext`/`WindowRuntimeContext`
  runtime behavior.
- Step 130 should add entity handle convenience methods on top of this alias
  and the existing entity/model helpers, without turning `Context<T>` into an
  owning context wrapper.

## 2026-07-01 Context Authoring Alias

- Step 129 keeps `Context<T>` intentionally as a public authoring alias over
  `ViewContext`; it does not add new context storage, model ownership, global
  state, async behavior, or runtime lifetime semantics.
- The alias is type-compatible with the existing `ViewContext` helper surface,
  so `Context<MyView>` can call model helpers, input snapshots, invalidation
  helpers, and other context APIs without changing `WindowRuntimeContext`.
- The template parameter is reserved for authoring readability and future
  typed-view ergonomics. Step 130 should build entity handle convenience on top
  of the existing context/model APIs rather than changing the `Context<T>`
  alias into an owning wrapper.

## 2026-07-01 Public Prelude Demo Rewrite Merged

- Step 128 is merged on `master` at
  `4026899 feat: rewrite demo with public prelude` and post-merge verified on
  Windows and WSL Arch Linux.
- The 128-step gate is complete: targeted architecture/prelude tests passed
  2/2, Windows full debug passed 29/29, and WSL Arch Linux full debug passed
  26/26 after the merge.
- Step 129 can now start from a fresh `context-authoring-alias` worktree. It
  should add the public `Context<T>` authoring alias over `ViewContext` without
  changing runtime ownership or starting the later entity handle/global-state
  steps early.

## 2026-07-01 Public Prelude Demo Rewrite

- Step 128 rewrites `examples/hello_window` around the public prelude and
  authoring APIs: `cgpui/cgpui.hpp`, `run_app`, `AppRunnerOptions`,
  `AppContext`, `View::render(ViewContext&)`, free factories, fluent builder
  shortcuts, and `ViewContext` model/text helpers.
- The demo no longer owns a manual `ElementTree` or constructs
  `WindowRuntime` directly. App-level lifecycle callbacks are still installed
  during `setup_context`, while author-facing text binding, focus, cursor,
  action, key binding, subscription, and model update calls go through
  `ViewContext`.
- Current rendered elements only give the runtime a stable root element id
  during `ElementTree::set_root(...)`. The demo therefore binds the text model
  to the rendered root id for this step and keeps true nested/keyed element
  identity deferred to the planned keyed reconciliation/widget work.
- Feature-worktree verification passed: targeted architecture/prelude tests
  2/2, Windows hello-window smoke tests 3/3, Windows full debug 29/29, and WSL
  Arch Linux full debug 26/26.

## 2026-07-01 IME Candidate Rectangle Data Merged

- Step 127 is merged on `master` at
  `80aadae feat: add focused text ime rect` and post-merge verified on Windows
  and WSL Arch Linux.
- The remaining pre-back-40 gate is Step 128 plus the post-Step-128 targeted,
  Windows full debug, and WSL Arch Linux full debug verification.
- Step 128 should rewrite the demo around the public prelude and new authoring
  APIs; it should not expand into Win32 IME placement or Wayland text-input
  protocol work now that focused-text IME geometry is available.

## 2026-07-01 IME Candidate Rectangle Data

- Step 127 exposes IME composition/candidate geometry as shared runtime data,
  not platform placement: `ImeCandidateRect`,
  `WindowRuntime::focused_text_ime_rect()`, and
  `WindowRuntimeContext::focused_text_ime_rect()` give later Win32 and Wayland
  IME code one geometry source to consume.
- The candidate rectangle intentionally mirrors Step 123 caret geometry:
  origin x is the focused `TextElement` layout x plus cursor byte offset times
  deterministic glyph width (`font_size * 0.5F`), origin y is the text element
  layout y, width is `1.0F`, and height is the text element font size.
- The runtime lookup soft-fails with `std::nullopt` when there is no focused
  text element, no bound `TextModel`, no routed `TextElement`, or no layout
  bounds. This keeps platform IME placement code from guessing stale geometry.
- Step 127 deliberately does not call Win32 IME APIs or Wayland text-input
  protocols. Those are later platform-depth slices; this step only pins the
  shared focused-text geometry contract.

## 2026-07-01 Wayland Clipboard Skeleton Merged

- Step 126 is merged on `master` at
  `ab464d5 feat: add wayland clipboard skeleton` and post-merge verified on
  Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 127-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch Linux full debug verification.
- Step 127 should expose focused text IME composition/candidate rectangle data
  without wiring platform IME placement yet.

## 2026-07-01 Wayland Clipboard Skeleton

- Step 126 keeps Wayland clipboard work deliberately skeletal: Linux now has a
  public `WaylandClipboard` implementation with observable `unsupported`,
  `no_seat`, and `available` support states, but it does not implement full
  Wayland data-device transfer yet.
- `WaylandClipboard` uses a `MemoryClipboard` fallback for all current support
  states, preserving runtime copy/cut/paste behavior when the compositor lacks
  data-device support or no seat is available.
- Linux `create_platform_clipboard()` now returns `WaylandClipboard` instead
  of `MemoryClipboard`, giving future data-device work a platform-specific
  backend boundary without changing the shared `Clipboard` interface.
- Step 126 intentionally does not wire `wl_data_device_manager` discovery into
  `WaylandApplication`; that belongs in later platform-depth work once the
  skeleton contract is pinned.

## 2026-07-01 Win32 System Clipboard Backend Merged

- Step 125 is merged on `master` at
  `389b9fb feat: add win32 system clipboard` and post-merge verified on
  Windows and WSL Arch Linux.
- The remaining pre-back-40 gate after Step 125 was Steps 126-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch Linux full debug verification.
- From the Step 125 merge snapshot, Step 126 should start from
  `.worktrees/wayland-clipboard-skeleton` on
  `codex/wayland-clipboard-skeleton` and add a Wayland clipboard backend
  skeleton with graceful unsupported/no-seat behavior while preserving the
  shared `Clipboard` interface.

## 2026-07-01 Win32 System Clipboard Backend

- Step 125 keeps `MemoryClipboard` as the deterministic test/runtime fixture
  while making the Windows platform clipboard real through
  `CF_UNICODETEXT`.
- `create_platform_clipboard()` now remains platform-neutral at the API
  boundary: Windows returns a UTF-8 wrapper over the system clipboard, while
  non-Windows keeps the existing memory fallback until the Wayland skeleton in
  Step 126.
- The RED test needs a forced target rebuild after editing
  `tests/platform/clipboard_test.cpp`; otherwise xmake may reuse the previous
  binary and falsely report the old clipboard contract as passing.
- The WSL distro name on this machine is `archlinux`, not `Arch`; use
  `wsl -d archlinux ...` for Linux verification.

## 2026-07-01 Platform Cursor Application Merged

- Step 124 is merged on `master` at
  `74ad787 feat: apply platform cursors` and post-merge verified on Windows and
  WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 125-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 125 should add a Win32 system clipboard backend for UTF-8 text while
  keeping the existing memory clipboard path deterministic for runtime tests.

## 2026-07-01 Text Caret And Selection Paint Metadata Merged

- Step 123 is merged on `master` at
  `b0b9e00 feat: add text caret selection paint` and post-merge verified on
  Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 124-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 124 should connect runtime cursor state to Win32 and Wayland platform
  hooks while keeping clipboard, IME geometry, and cursor theme depth out of
  scope.

## 2026-07-01 Text Caret And Selection Paint Metadata

- Step 123 is implemented in `.worktrees/text-caret-selection-paint` on
  `codex/text-caret-selection-paint` and feature-worktree verified on Windows
  and WSL Arch Linux.
- Caret and selection are paint-list metadata only at this stage:
  `PaintCommandKind::text_selection`, `PaintCommandKind::text_caret`,
  `TextSelectionPaint`, `TextCaretPaint`, and matching `PaintList` fill helpers
  expose intent without adding Vulkan text drawing.
- `TextElement` uses the existing Step 122 deterministic fallback metrics:
  glyph width is `font_size * 0.5F`, selection geometry is derived from sorted
  byte offsets, and caret geometry is one pixel wide at the model cursor.
- A bound empty `TextModel` still emits caret metadata. Text content commands
  are emitted only when text is non-empty.
- `render_view(...)` skips text, text selection, and text caret commands until
  later text/glyph renderer work consumes them directly.
- Step 123 intentionally does not add shaping, glyph cache ownership, platform
  IME placement, platform cursor/clipboard behavior, or Vulkan text drawing.

## 2026-06-30 Font Descriptor And Font Size Style Merged

- Step 122 is merged on `master` at
  `58561b1 feat: add font size style` and post-merge verified on Windows and
  WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 123-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 123 should use the deterministic Step 122 `font_size` metrics and
  `TextPaint` font metadata to emit caret and selection paint metadata. Keep
  shaping, glyph cache ownership, platform IME placement, and Vulkan text
  drawing out of Step 123.

## 2026-06-30 Font Descriptor And Font Size Style

- Step 122 keeps font work as conservative public metadata: `FontDescriptor`
  currently carries a family string, `Style` and `StyleOverlay` carry font and
  `font_size`, and the element builder exposes `.font(...)` and
  `.font_size(...)` without adding platform font discovery yet.
- The default text metric contract is preserved at 16px height and 8px glyph
  width because `TextElement` now derives deterministic fallback metrics as
  glyph width `font_size * 0.5F` and height `font_size`.
- `TextPaint` now carries copied font metadata and font size alongside bounds,
  color, content, byte length, and clip metadata. This gives Step 123 a stable
  metric source for caret/selection paint metadata.
- Step 122 intentionally does not add shaping, glyph caches, font discovery,
  Vulkan text drawing, caret/selection commands, cursor, clipboard, or IME
  behavior. Those remain later planned slices.

## 2026-06-30 Text Paint Command Merged

- Step 121 is now merged at `cf180f4 feat: add text paint command` and
  post-merge verified on Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 122-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 122 should now add font descriptor and basic font-size style primitives
  on top of explicit text command metadata. Keep shaping, glyph caches, and
  Vulkan text drawing out of Step 122.

## 2026-06-30 Text Paint Command

- Step 121 separates UI text intent from rectangle painting by adding
  `PaintCommandKind::text`, `TextPaint`, and `PaintList::fill_text(...)`.
  Text payloads preserve bounds, color, copied UTF-8 content, byte length, and
  active clip metadata.
- `TextElement::paint(...)` now emits text commands instead of a placeholder
  solid rectangle. This keeps later font, caret, selection, glyph cache, and
  Vulkan text work attached to explicit text metadata instead of trying to
  infer text from rectangle fallbacks.
- `render_view(...)` intentionally skips text commands for now. That is better
  than converting text back to a solid rectangle because Step 152 will teach
  Vulkan to consume text/glyph metadata directly; until then text commands are
  observable in paint-list tests but not drawn by the simple rect renderer.
- Step 121 intentionally does not add font descriptors, font size style,
  caret/selection paint metadata, glyph caches, platform clipboard behavior, or
  real text shaping. Those remain Steps 122, 123, 125/126, and 149-152 work.
- Step 122 should build on this by adding font descriptor and font-size style
  metadata before caret/selection geometry tries to use text metrics.

## 2026-06-30 Back-40 Planning With Step 121 Active

- The后 40 步 remain Steps 129-168 and are still gated behind Steps 121-128
  plus the post-Step-128 Windows/WSL verification on `master`.
- The current planning anchor is the docs closeout commit
  `45a8dad docs: mark step 120 merged`; use
  `9aba0e6 feat: honor vulkan solid rect clips` as the Step 120 behavior
  commit, not the current `master` HEAD.
- Step 121 is already active in `.worktrees/text-paint-command` on
  `codex/text-paint-command`; future execution should finish and merge that
  branch before opening Step 122. Do not create a Step 129 worktree until the
  Step 128 exit contract passes.
- A useful后 40 步 plan needs more than branch names and target tests: each
  step now has an exit artifact and an explicit keep-out-of-scope note. This
  should reduce step creep in high-risk areas such as async, widgets, text
  shaping, Vulkan text drawing, multi-window lifecycle, accessibility, and the
  final parity audit.
- The distance estimate remains 8 implementation steps to Step 129 while Step
  121 is unmerged. After Step 121 merges and is post-merge verified, the
  distance becomes 7 implementation steps, Steps 122-128, plus the
  post-Step-128 verification gate.

## 2026-06-30 Back-40 Planning After Step 120 Merge

- The后 40 步 remain Steps 129-168, and they are now gated behind Steps
  121-128 plus the post-Step-128 Windows/WSL verification on `master`.
- Step 120 is no longer part of the active gate: it is merged at
  `9aba0e6 feat: honor vulkan solid rect clips` and post-merge verified on
  Windows and WSL Arch Linux.
- The next implementation slice is Step 121, text paint command metadata. It
  should replace text placeholder rectangles with command metadata without
  pulling in font descriptors, caret/selection rendering, or platform clipboard
  work early.
- The Step 168 outcome remains a practical Windows/Linux GPUI-core foundation
  plus a parity audit document. It is not full upstream GPUI parity, and
  macOS/Cocoa + Metal remains explicitly deferred.

## 2026-06-30 Vulkan Clip Rect Metadata

- Step 120 extends `SolidRect` with optional renderer-facing clip metadata so
  clipped paint commands can reach the backend without introducing a separate
  paint-command renderer API yet.
- `render_view(...)` must copy `PaintCommand::clip_rect` onto the `SolidRect`
  sent to `RenderFrame::draw_rect(...)`; otherwise the UI paint-list metadata
  added in Steps 53 and 119 stops before the renderer boundary.
- The current Vulkan solid-rect implementation uses `vkCmdClearAttachments`
  with a `VkClearRect`. Honoring clip metadata means computing the framebuffer-
  clamped intersection of `SolidRect::rect` and `SolidRect::clip_rect` before
  issuing the clear. This is equivalent to a scissor for the current clear-rect
  path and does not require a graphics pipeline yet.
- Win32 pixel sampling from an sRGB swapchain reports the clear color after
  conversion, so the outside-clip test should assert a dark clear-color range,
  not the raw linear `0.08/0.09/0.10` channel bytes.
- Step 121 can now focus on text paint commands. It should not revisit Vulkan
  clip handling or rounded-rect rasterization unless new text command tests
  force a shared command-vocabulary change.

## 2026-06-30 Back-40 Planning After Step 119 Merge

- The后 40 步 are still Steps 129-168, but the live entry gate has moved
  forward: Step 119 is merged and post-merge verified, so the remaining
  pre-back-40 work is Steps 120-128 plus post-Step-128 Windows/WSL
  verification.
- Step 120 should now be the next worktree. Its scope is Vulkan consumption of
  `PaintCommand::clip_rect` for solid rectangles; it should not expand into
  rounded-rect rasterization or text drawing.
- The Step 168 outcome remains a practical Windows/Linux GPUI-core foundation
  plus a parity audit document. It is not full upstream GPUI parity, and
  macOS/Cocoa + Metal remains explicitly deferred.

## 2026-06-30 Rounded-Rect Paint Command

- Step 119 keeps rounded rectangles as paint-list command metadata first. The
  Vulkan backend still receives the `solid_rect` fallback carried by each
  command; actual rounded drawing is intentionally left for later renderer
  work.
- Nonzero `Style::border_radius` on a styled background emits
  `PaintCommandKind::rounded_rect` with a `RoundedRect` payload preserving all
  four corner radii. Zero radius preserves the existing
  `PaintCommandKind::solid_rect` background behavior.
- `PaintList::fill_rounded_rect(...)` attaches the same active clip metadata as
  `fill_rect(...)`, so Step 120 can focus on Vulkan clip consumption without
  revisiting command storage.
- The next renderer slice should teach Vulkan to honor `PaintCommand::clip_rect`
  for solid rectangles. It should not implement real rounded-rect rasterization
  yet unless the plan is explicitly changed.

## 2026-06-30 Back-40 Planning After Step 118 Docs Closeout

- The后 40 步 are Steps 129-168 and remain gated behind completion of Steps
  119-128 plus post-Step-128 Windows and WSL verification.
- Current planning should anchor on the docs closeout commit
  `c0d38c7 docs: mark step 118 merged`, not only the Step 118 feature commit
  `9dfc2e7 feat: add layer elevation z order`.
- The intended Step 168 outcome is a practical Windows/Linux GPUI-core
  foundation plus `docs/gpui-core-api-parity.md`; it is not a claim of full
  upstream GPUI parity and it keeps macOS/Cocoa + Metal deferred.
- The immediate implementation action is still Step 119 in
  `.worktrees/rounded-rect-paint-command`, starting with RED tests for
  border-radius metadata in rounded-rect paint commands.

## 2026-06-30 Layer/Elevation Z Order Merged

- After Step 118 merged, the remaining pre-back-40 gate is Steps 119-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 119 should build on the current paint command stream by preserving
  border-radius metadata in a rounded-rect command shape first. Avoid changing
  Vulkan rounded drawing behavior until the command metadata is pinned by RED
  tests.

## 2026-06-30 Layer/Elevation Z Order

- Step 118 maps layer/elevation onto deterministic paint order by adding
  `Style::layer`, `StyleOverlay::layer`, builder `.layer(...)`, and
  `Element::layer()`/`z_order()`.
- Explicit nonzero `z_index` remains the compatibility override. The effective
  paint key is `z_index` when it is nonzero; otherwise it is `layer`.
- Layer and z-index metadata need to live on the final `Element`, not only on
  `Style`, because parent paint ordering sees the outer wrapper after click,
  pointer, key, and focusable wrappers are applied.
- Stable sort remains important: siblings with the same effective `z_order()`
  continue painting in authored insertion order, preserving the existing
  deterministic z-index contract.

## 2026-06-30 Back-40 Planning After Step 117 Docs Closeout

- The后 40 步 remain Steps 129-168 and should not preempt the active Step
  118-128 queue.
- The current planning anchor is `c3b6ecb docs: mark step 117 merged`; use
  `210c85d feat: add absolute positioning insets` only as the Step 117 feature
  commit reference.
- The remaining route to Step 129 is 11 implementation steps, Steps 118-128,
  plus post-Step-128 targeted verification, Windows full debug, WSL Arch full
  debug, and a clean tracked/staged worktree.
- The back-40 plan now explicitly records Steps 115 and 116 as gate-complete
  alongside Step 117. This matters because Steps 139-148 widget work and
  Steps 149-158 rendering work depend on the full flex/alignment/grow/shrink/
  absolute/layer layout substrate being complete before Step 129 starts.

## 2026-06-30 Absolute Positioning And Insets Merged

- After Step 117 merged, the remaining pre-back-40 gate is Steps 118-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 118 should build on Step 117's layout substrate by adding
  layer/elevation ordering as deterministic metadata first. Avoid changing
  absolute child sizing or adding right/bottom anchoring unless Step 118 tests
  explicitly require it.

## 2026-06-30 Absolute Positioning And Insets

- Position and inset need to live on the final `Element`, not only on `Style`,
  because parent stack/flex layout reads each built child after behavior
  wrappers like click, pointer, key, and focusability have been applied.
- Default `Position::relative` preserves all previous stack/flex layout.
  Absolute children are measured for their own size but excluded from normal
  flow size, gap spacing, flex grow/shrink allocation, justification, and
  alignment.
- Step 117 intentionally gives `left` and `top` placement semantics first.
  `right` and `bottom` are stored in `EdgeSizes` for API continuity and later
  anchoring work, but they should not drive layout until a later step adds
  explicit right/bottom behavior and tests.
- Stack and flex layout should place absolute children after the parent output
  size is known. This keeps normal-flow children deterministic and gives Step
  118 layer/elevation work a stable layout substrate.

## 2026-06-30 Back-40 Planning After Step 116 Docs Closeout

- The后 40 步 are still Steps 129-168, and the current planning anchor should
  be the docs closeout commit `1e542bd docs: mark step 116 merged`, not only
  the Step 116 feature commit `2806a4a feat: add flex grow shrink layout`.
- Step 117 already has an active worktree at
  `.worktrees/absolute-position-insets` on `codex/absolute-position-insets`.
  Future execution should continue that branch through verification and merge
  instead of creating a new Step 117 branch or starting Step 129.
- The useful post-Step-128 delivery target is four staged outcomes:
  context/entity/async ergonomics, keyed reusable widgets and style cascade,
  backend-ready text/render metadata, and Win32/Wayland platform closure plus a
  parity audit. Step 168 should remain the audit/handoff milestone, not a claim
  of full upstream GPUI parity.
- From current `master`, the back-40 entry is still gated by 12 implementation
  steps, Steps 117-128, plus post-Step-128 targeted, Windows full debug, and
  WSL Arch full debug verification.

## 2026-06-30 Flex Grow And Shrink Merged

- After Step 116 merged, the remaining pre-back-40 gate is Steps 117-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 117 should start from a fresh `.worktrees/absolute-position-insets`
  worktree and keep absolute positioning/insets as layout metadata first,
  before Step 118 layers/elevation and Step 119+ renderer command hardening.

## 2026-06-30 Flex Grow And Shrink

- Flex grow and shrink factors need to live on `Element`, not only on
  `Style`, because the parent `FlexElement` reads each final child element
  during layout.
- Builder-created behavior wrappers must copy the flex factors onto the
  outermost wrapper (`ClickElement`, `PointerElement`, `KeyElement`, and
  `FocusableElement`). Otherwise the parent flex container would see zero
  factors when a child has handlers or focusability.
- Default grow/shrink factors remain `0.0F`, preserving previous flex layout
  for existing children unless authors explicitly opt in.
- Shrink tests that constrain only height must still provide a nonzero max
  width when using aggregate `Size` initialization, because omitted width
  fields default to zero and can collapse output width before shrink behavior
  is evaluated.

## 2026-06-30 Back-40 Planning Hardening

- The后 40 步 should be treated as an execution-ready continuation of the
  current Step 128 gate, not as a separate roadmap. The useful completion
  target is a practical Windows/Linux GPUI-core foundation with public
  context/entity/global APIs, reusable keyed widgets, deterministic
  text/render metadata, platform hooks, and an explicit parity audit.
- Step 168 remains an audit milestone rather than a full GPUI parity claim.
  The audit should be the authoritative place for implemented, partial,
  missing, and Mac/Metal-deferred areas.
- The back-40 non-goals are important safety rails: do not begin macOS/Metal
  parity, do not replace Vulkan/xmake, do not require a full shaping stack
  before deterministic text contracts exist, do not create a separate widget
  framework, and do not combine small steps to reduce commit count.
- Step 129 should start from a verified `master` with a fresh
  `.worktrees/context-authoring-alias` worktree. Its first RED should be a
  public `Context<T>` authoring alias compile failure plus a runtime test that
  proves the alias preserves existing `ViewContext` helper behavior.

## 2026-06-30 Flex Alignment And Justification

- Flex alignment and justification can stay entirely in shared style/element
  code for Step 115. No runtime or platform-specific behavior is needed.
- `JustifyContent::start` preserves the old flex layout behavior. `center` and
  `end` use only positive constrained free space as initial main-axis offset,
  and `space_between` keeps the authored `gap` as the base gap while
  distributing extra free space between children.
- `AlignItems` uses the final constrained cross-axis size, so rows align child
  y origins and columns align child x origins. Cross-axis free space is clamped
  at zero, preserving previous overflow behavior for children larger than the
  constrained container.
- Measuring children before assigning final bounds is necessary because the
  final constrained flex size controls both main-axis justification and
  cross-axis alignment.

## 2026-06-30 Back-40 Planning After Step 114 Closeout

- The后 40 步 remain Steps 129-168 and should still be treated as a
  post-Step-128 execution queue, not the active branch queue while Steps
  115-128 are incomplete.
- Current `master` is anchored at `ac745f8 docs: mark step 114 merged`; the
  Step 114 behavior commit is `d78a017 feat: clip hidden overflow hit testing`.
- Step 115 already has a worktree at `.worktrees/flex-alignment-justification`
  on `codex/flex-alignment-justification`, and its baseline targeted tests have
  passed. The next practical action is to add Step 115 RED tests there, not to
  recreate the worktree or start Step 129.
- The route to Step 129 is 14 incomplete implementation steps, Steps 115-128,
  plus post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. The route through Step 168 is 54 implementation steps plus
  checkpoint reviews after Steps 138, 148, 158, and 168.
- The back-40 order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus parity audit last.

## 2026-06-30 Hidden Overflow Hit Testing

- `StyledElement` needed its own `hit_test(...)` override because the inherited
  `Element::hit_test(...)` only checked the styled wrapper's layout bounds; it
  neither forwarded to the owned child nor applied `Overflow::hidden` clip
  semantics.
- Hidden overflow hit testing should mirror the existing paint clip contract:
  if `Style::clip_rect` is present, use it as the hit-test clip; otherwise use
  the styled element's layout bounds. Points outside that clip return no hit
  before checking child or self hits.
- Visible overflow must keep checking the child before falling back to the
  styled element itself. This preserves current authoring behavior where a
  child can be hit outside the wrapper's own bounds when overflow remains
  visible.
- Runtime coverage for an internally owned child is limited by
  `ElementTree::get(...)`: a child owned inside `StyledElement` is not a tree
  node, so runtime route assertions should verify root/no-target clipping
  behavior unless a later step promotes internal children into the tree.
- Keep `ScrollElement::hit_test(...)` wrapper-targeting unchanged. Step 113
  depends on scroll routing finding the scroll viewport wrapper so the runtime
  can locate the bound `ScrollState`.

## 2026-06-30 Back-40 Planning At Step 113 Docs Closeout

- The后 40 步 are still Steps 129-168, and the current anchor should be the
  docs closeout commit `2251d44 docs: mark step 113 merged`, not only the Step
  113 behavior commit `7a2ef39 feat: route scroll events to scroll state`.
- The gate into Step 129 is now exactly Steps 114-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- The execution order should stay sequential and banded: Steps 129-138 for
  context/entity/global/async, Steps 139-148 for keyed reconciliation/widgets,
  Steps 149-158 for text/font/renderer diagnostics, and Steps 159-168 for
  Windows/Wayland platform closure plus the parity audit.
- The next practical implementation action remains Step 114 in a fresh
  `.worktrees/hidden-overflow-hit-testing` worktree, not Step 129.

## 2026-06-30 Scroll Routing

- `PointerScrolled` routing belongs after route target selection and ancestry
  refresh but before generic element dispatch/view fallback. That lets scroll
  state consume wheel/trackpad deltas without invoking the view when a bound
  `ScrollElement` owns the viewport under the pointer.
- `ScrollElement` should hit-test as the scroll viewport wrapper for now. Its
  child is owned internally rather than indexed in `ElementTree`, so returning
  the child id can leave `WindowRuntime::routed_element(...)` unable to find
  the element that actually owns the `ScrollState`.
- The scroll delta should be applied through `ScrollState::scroll_by(...)`
  instead of setting offsets directly so existing viewport/content-size clamp
  semantics remain the single source of truth.
- A no-scroll route remains a soft-fail path: if no routed `ScrollElement` is
  found, the runtime continues to normal element dispatch and view fallback.


## 2026-06-30 Back-40 Planning After Step 112 Closeout

- The后 40 步 are Steps 129-168 and remain a post-Step-128 follow-on queue, not
  the active branch queue while Step 113-128 are still incomplete.
- Current `master` is anchored at `ed948a7 docs: mark step 112 merged`; Step
  113 already has a worktree at `.worktrees/scroll-routing` on
  `codex/scroll-routing`, so the correct next action is to continue that
  worktree's RED/GREEN cycle.
- Step 129 is 16 implementation steps away: Steps 113-128 plus post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification. Step 168
  is 56 implementation steps away from current `master`, plus the four band
  checkpoint reviews.
- The back-40 band order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus the parity audit last.


## 2026-06-30 Focus Traversal

- Step 111 can reuse `ElementTree::enabled_preorder_ids()` as the traversal
  source. That keeps disabled descendants out of traversal before checking
  `Element::focusable()`, so disabled focusable-looking elements do not receive
  Tab focus.
- Focus traversal belongs in the keyboard-key pre-dispatch path: after a
  successful traversal, the current event route should be retargeted to
  `keyboard_focus_element_owner_` and ancestry refreshed so key bindings,
  routed element handlers, view fallback, and after-event callbacks all see the
  updated focused element.
- Shift+Tab can share the same traversal helper as Tab by passing a reverse
  flag. With no current focus, forward traversal chooses the first enabled
  focusable element and reverse traversal chooses the last one; with current
  focus, both directions wrap.
- The helper should ignore Tab variants with control, alt, or super modifiers
  so future app/window shortcuts can still bind those chords. Shift remains the
  reverse traversal selector.

## 2026-06-30 Back-40 Planning With Step 111 Active

- The后 40 步 still mean Steps 129-168, and they remain a post-Step-128
  follow-on queue. They should not preempt the active Step 111-128 gate.
- Current `master` is anchored at `d52ce80 docs: mark step 110 merged`; the
  Step 110 behavior commit is `10f2dd3 feat: add event propagation phases`.
- Step 111 has landed on `master`; planning should point future workers at
  Step 112, not at recreating `.worktrees/focus-traversal` or starting Step 129
  early.
- From current `master`, Step 129 is 17 implementation steps away: Steps
  112-128 plus post-Step-128 targeted, Windows full debug, and WSL Arch full
  debug verification.
- Step 168 is still 57 implementation steps away from current `master`: Steps
  112-168 plus the four band checkpoint reviews after Steps 138, 148, 158, and
  168.
- The back-40 band order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus parity audit last.

## 2026-06-30 Back-40 Planning After Step 110 Merge

- The后 40 步 remain Steps 129-168. They should still be treated as the
  post-Step-128 follow-on queue, not as the active implementation queue.
- Current `master` is anchored at `10f2dd3 feat: add event propagation phases`;
  Steps 89-110 are merged and post-merge verified on Windows and WSL Arch
  Linux.
- From this state, Step 129 is 18 implementation steps away: Steps 111-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. Step 168 is 58 implementation steps away: Steps 111-168 plus
  the four band checkpoint reviews.
- The next active implementation should remain Step 111, focus traversal over
  enabled focusable elements with Tab and Shift+Tab actions. Starting Step 129
  before focus, scroll, layout depth, renderer/text metadata, cursor/clipboard,
  IME geometry, and the public-prelude demo rewrite land would destabilize the
  follow-on context/widget/platform plan.
- The back-40 band order should stay: context/entity/global/async, keyed
  reconciliation/widgets/style cascade, text/font/renderer diagnostics, then
  Windows/Wayland platform closure plus the parity audit.

## 2026-06-30 Event Propagation Phases

- Step 110 uses `EventRoute::element_ancestry` as the dispatch route. The order
  remains target-to-root: the target element handles first, then each ancestor
  can handle the same event until one returns consumed or cancelled.
- `ElementEventContext::target_element_id` intentionally remains the original
  route target for every bubbling phase. This lets ancestor handlers know which
  descendant was hit/focused without changing the public context shape yet.
- Disabled elements remain part of route ancestry for diagnostics, but their
  handlers are skipped during bubbling. Dispatch continues to later enabled
  ancestors and only falls back to the root view when every routed element
  returns unhandled or is skipped.
- A route without `element_ancestry` still falls back to the route target id,
  preserving compatibility for legacy routes while Step 109's ancestry refresh
  is now the normal path.

## 2026-06-30 Back-40 Planning After Step 109 Merge

- The post-Step-128 "back 40" remains Steps 129-168. It is an execution-ready
  follow-on queue, not the active branch queue while Step 110-128 are still
  incomplete.
- Current `master` is anchored at `08d0fef docs: mark step 109 merged`; the
  Step 109 feature commit is `74df1df feat: add event route ancestry`.
- From this state, Step 129 is 19 implementation steps away: Steps 110-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. Step 168 is 59 implementation steps away: Steps 110-168 plus
  the four band checkpoint reviews.
- The back-40 order should stay banded: context/entity/global/async first,
  keyed reconciliation/widgets/style cascade second, text/font/renderer
  diagnostics third, and Windows/Wayland platform closure plus parity audit
  last. This order avoids asking widgets or platform hooks to invent missing
  shared runtime contracts.

## 2026-06-30 Event Route Ancestry Metadata

- Step 109 keeps ancestry as a route snapshot, not a propagation behavior
  change. `EventRoute::element_ancestry` is ordered target-to-root so Step 110
  can handle the target first and then bubble through ancestors without
  recomputing parent links.
- `EventRoute::view_ancestry` is also ordered target-to-root. A normal root
  event records only `ViewId{1}`; a hit on a registered `ChildViewElement`
  records the child `ViewId` followed by the root view. The current event still
  dispatches through the existing root view fallback until Step 110 adds real
  propagation phases.
- Runtime route refresh must happen after the final target is selected,
  because keyboard focus, pointer capture, and hit testing can each choose a
  different element or view target. View-target pointer capture clears the
  element target before ancestry is recalculated so stale element metadata does
  not leak into view-routed events.
- Child-view route targeting only upgrades to the child view when the placeholder
  references a registered live view. Removed or missing child views still leave
  the event on the root route, matching the view registry soft-fail contract.

## 2026-06-30 Child-View Placeholder

- Step 108 is intentionally metadata-only. `ChildViewElement` stores a
  `ViewId`, lays out to a constrained placeholder size, participates in
  hit-testing as a normal element, and paints nothing for now.
- Real nested view rendering, parent/child view ancestry, and event propagation
  across view boundaries remain Step 109/110 work. Keeping Step 108 small gives
  those steps a concrete element marker without forcing their routing semantics
  early.
- `ViewId` belongs in `element.hpp` now because child-view placeholders are
  authorable through element builders and public prelude helpers. Keeping a
  duplicate `ViewId` in `ui.hpp` would either make `element.hpp` depend on the
  runtime header or split the public id type.
- The runtime integration test should only prove that a registered child
  `ViewId` can be referenced by an installed placeholder element while
  `WindowRuntime::find_view(...)` still resolves the view. It should not call
  the child view's `render(...)` yet.
- After the Step 108 merge, Step 109 is the right next slice because the
  placeholder now gives event routing a concrete element marker that can carry
  both `ElementId` ancestry and child `ViewId` metadata without needing nested
  rendering first.

## 2026-06-30 Back-40 Planning While Step 108 Active

- The後 40 步 are still Steps 129-168, but the active implementation path has
  not crossed the Step 128 gate yet. Step 108 already has a worktree, so the
  next engineering action should continue `.worktrees/child-view-placeholder`
  instead of creating a Step 129 branch.
- The post-Step-128 plan should be treated as an execution-ready queue with
  branch slugs, RED test intents, targeted commands, Windows full debug, and
  WSL Arch full debug for every step. It is not just a roadmap, but it is still
  blocked by Steps 108-128.
- The current pre-back-40 gate count is 21 implementation steps: Step 108
  through Step 128. The high-risk gate items are event ancestry/propagation,
  text paint metadata, Vulkan clip/text command handling, cursor/clipboard
  backends, IME geometry, and the public-prelude demo rewrite.
- Step 168 should close with a parity audit, not a claim of full upstream GPUI
  equivalence. The useful audit split is implemented, partial, missing, and
  Mac/Metal-deferred areas for the Windows/Linux track.

## 2026-06-30 View Registry Skeleton

- Step 107 is intentionally a registry/lifetime skeleton, not child-view
  rendering. It gives later child-view placeholders, ancestry routing, and
  multi-window work a single view lookup path before those behaviors exist.
- The root view is registered as borrowed and non-removable at `ViewId{1}`.
  Additional borrowed views can be registered by reference, and owned views can
  be registered through `std::unique_ptr<View>`.
- App-opened root views should use the general view registry instead of a
  separate root-view ownership map. This keeps `AppContext::open_window(...)`,
  `app_opened_window_root_view(...)`, `WeakView` upgrade, and future child-view
  lookup on the same storage contract.
- Removing a registered non-root view should make `find_view(...)`,
  `is_view_id_allocated(...)`, and `WeakView` upgrade soft-fail for that id.
  Plain ids from `allocate_view_id()` remain compatible with the older
  monotonic allocation contract until a later view lifecycle step replaces it
  with full generation/removal semantics.

## 2026-06-30 Back-40 Planning After Step 107

- The "后40步" plan is now best understood as a post-Step-128 execution plan,
  not an immediate branch queue. With Step 107 feature-worktree verified, the
  remaining gate into Step 129 is Steps 108-128 plus final Windows and WSL Arch
  Linux verification on `master`.
- Step 129 should start only after the child-view, ancestry, propagation,
  focus, scroll, layout-depth, render-command, cursor, clipboard, IME, and
  public-demo slices have landed. Starting it earlier would make the context
  and widget APIs depend on unstable or placeholder runtime surfaces.
- The four 10-step bands should remain ordered: context/entity/async first,
  keyed widgets and style cascade second, text/font/renderer diagnostics third,
  and Windows/Wayland platform closure plus parity audit last. This order gives
  each band a usable substrate instead of asking later platform work to invent
  missing shared contracts.
- Because Steps 129-168 all touch public headers, shared runtime behavior,
  renderer contracts, or platform-neutral event data, every step should keep
  WSL Arch full debug in its normal definition of done even when the visible
  feature name is Win32-specific.

## 2026-06-30 App-Opened Root View Lifecycle

- Step 106 should remain a storage/lifecycle slice, not real multi-window
  platform creation. `AppContext::open_window(WindowOptions,
  std::unique_ptr<View>)` records the window descriptor, allocates a distinct
  `ViewId`, and transfers root view ownership into `WindowRuntime`; `run_app`
  still creates only the one platform/native window from the primary
  descriptor.
- Keeping `open_window(WindowOptions)` source-compatible with an empty
  `root_view_id` preserves the Step 105 skeleton path while allowing callers
  that supply root view ownership to receive an allocated id and query the
  stored view during setup and frame callbacks.
- The runtime-owned root view container naturally destroys app-opened root
  views when `run_app` returns and the `WindowRuntime` is destroyed. No
  explicit cleanup at `WindowRuntime::run()` start or end is needed for this
  slice, and clearing there would risk erasing setup-time app-opened windows.
- The initial Step 106 GREEN failure was test-side: the after-frame callback
  captured a setup-local pointer variable by reference. Capturing the pointer
  value keeps the test checking runtime lifecycle behavior instead of a
  dangling test reference.

## 2026-06-30 Back-40 Planning After Step 105 Merge

- The "后40步" plan is now an execution-ready follow-on queue for Steps
  129-168, but the active implementation path is still Steps 106-128. Starting
  Step 129 early would skip root-view lifecycle storage, the view registry,
  child-view placeholders, route ancestry, propagation, focus/scroll/layout
  depth, render command hardening, cursor/clipboard/IME surfaces, and the
  public-prelude demo rewrite.
- The clean handoff into Step 129 requires 23 remaining implementation steps:
  Steps 106-128, then post-Step-128 targeted verification, Windows full debug,
  WSL Arch full debug, and a clean `master` except the known untracked
  `.vscode/`.
- The back-40 should be reviewed in four checkpoint bands: Step 138 for
  context/entity/global/async APIs, Step 148 for keyed widgets and style
  cascade, Step 158 for text/render diagnostics, and Step 168 for
  Windows/Wayland platform closure plus the GPUI-core parity audit.

## 2026-06-30 Render Invalidation Observability

- Render invalidation should be a strict superset of layout and paint
  invalidation: `request_render()` sets render/layout/paint and schedules a
  redraw, while existing `request_layout()` and `request_paint()` keep the new
  render bit false.
- The first after-render observation point belongs immediately after
  `View::render(...)` returns and any non-null root element is installed, before
  layout, paint, invalidation clearing, and frame index increment. This lets
  callers inspect the render sequence and installed root id while the frame is
  still in progress.
- `RenderRecord::root_element_id` should report the root installed by the
  current render pass. If a view returns an empty `AnyElement`, the record can
  still report sequence, view id, and viewport size with no installed root id.
- Resetting render sequence and last render record at `WindowRuntime::run()`
  keeps each run deterministic and matches the existing event/frame sequence
  reset behavior.

## 2026-06-30 Steps 129-168 Execution Matrix

- The follow-on 40 steps should start only after Step 128 has a clean
  post-merge Windows and WSL baseline; otherwise context/widget/platform work
  will sit on top of unstable render, model, routing, text, or demo surfaces.
- The useful next level of planning detail is operational rather than more
  conceptual: each step now has a branch slug, first RED test intent, and
  targeted command so the existing RED/GREEN/merge cadence can continue
  without re-deciding the shape of the step every turn.
- All Steps 129-168 should keep WSL Arch full debug in the per-step definition
  of done, including Win32-looking steps, because these slices almost always
  introduce shared public headers, runtime contracts, or platform-neutral event
  data consumed by the Linux/Wayland path.
- The dependency order should stay banded: context/entity/global/async before
  widgets, keyed/lifecycle/style/focus/widgets before renderer text depth,
  renderer/text/diagnostics before platform completion and the final parity
  audit.

## 2026-06-30 Runtime Render Pass

- The narrow Step 97 integration point is `WindowRuntime::handle_redraw()`:
  rendering before the existing owned-tree layout pass lets the runtime reuse
  current `ElementTree::layout_root`, hit testing, routed-element lookup, and
  invalidation cleanup behavior.
- `View::render(ViewContext&)` takes a non-const lvalue context, so the runtime
  must materialize `ViewContext render_context = context();` before calling the
  hook instead of passing the temporary result of `context()` directly.
- A non-null render result should replace the runtime owned tree through the
  same `set_element_tree(...)` path used by `ViewContext`; this keeps
  `element_root()` and pointer routing consistent with manually installed
  trees.
- Existing `paint(...)` is still invoked by `render_view(...)` after the render
  tree is installed, preserving the compatibility contract until later steps
  route paint command generation through the rendered element tree.

## 2026-06-30 View Render Hook Skeleton

- `ViewContext` must be available before `View` is declared because the new
  virtual render hook uses the GPUI-like context spelling directly in the base
  class.
- The first render hook should be additive: the default `View::render` returns
  an empty `AnyElement`, while the existing `paint(...)` method remains pure
  virtual and source-compatible for old runtime and demo paths.
- Step 96 deliberately does not install the rendered tree into the runtime.
  That behavior remains Step 97 so the API hook can be verified separately
  from redraw, layout, hit testing, and owned element tree replacement.
- Calling `request_paint()` or `request_layout()` from render already works
  through `ViewContext` because the alias still forwards to
  `WindowRuntimeContext`; Step 98 can add render-specific invalidation and
  observability without changing the Step 96 hook shape.

## 2026-06-30 Steps 129-168 Planning Refresh

- The forward queue should be treated as a post-Step-128 plan, not as the next
  active implementation queue: Step 96 still gates the current track because
  `View::render`, runtime render-tree installation, model/app lifecycle,
  bubbling/focus/scroll/layout depth, render commands, cursor/clipboard/IME
  surfaces, and the public-prelude demo all feed the APIs used by Steps
  129-168.
- The most useful planning boundary is an execution gate rather than a second
  competing active plan. This keeps the follow-on route visible while preserving
  the current Step 96-128 RED/GREEN workflow.
- After Step 128, the first 10 follow-on steps should bias toward context,
  entity, global, scoped action, subscription, deferred, timer, async, batching,
  and diagnostics APIs before widget or renderer expansion.

## 2026-06-30 Style-State Overlays

- A separate `StyleOverlay` is necessary because `Style` has non-optional
  scalar/POD fields where zero is a valid authored value; optional overlay
  fields preserve "unset" versus "override to zero" semantics.
- Step 95 should stay authoring/data-only: `resolved_style(...)` is
  deterministic and testable, but runtime hover/focus/disabled application can
  wait for later render/style resolution work.
- The merge order is base, hover, focus, disabled. This lets disabled state win
  over active interaction state while still inheriting focus/hover fields that
  disabled does not override.
- `StyledElement` can store a full `StyleState` while keeping its old
  `style()` API as a base-style accessor. That preserves existing layout,
  paint, z-index, and tests while making state overlays available to later
  runtime/style-cascade slices.

## 2026-06-30 Steps 129-168 Forward Plan

- Steps 129-168 should remain a follow-on queue until Steps 95-128 finish,
  because the later context/widget/platform work depends on `View::render`,
  model helpers, propagation, text paint commands, cursor/clipboard/IME
  surfaces, and the public-prelude demo rewrite.
- The highest-leverage next 40 steps after Step 128 are not more authoring
  sugar; they are API-shape completion and runtime maturity: typed contexts,
  entity handles, globals, scoped actions, subscription ownership, deferred
  work, timers, async completions, keyed reconciliation, style cascade,
  widgets, text/font renderer depth, multi-window storage, and Win32/Wayland
  lifecycle hooks.
- The riskiest region is Steps 149-168, where text/font rendering and platform
  hooks cross from deterministic metadata into Vulkan, Win32, and Wayland
  behavior. Keep each step command-driven, fakeable in tests, and backed by
  Windows plus WSL verification.

## 2026-06-30 Pointer Handler Shortcuts

- Pointer handler shortcuts fit the existing wrapper pattern: keep element
  layout, paint, and hit testing transparent while intercepting only matching
  `PointerButton` or `PointerMoved` events.
- `.on_click(...)` remains source-compatible and still means pointer press; the
  new `.on_pointer_down(...)`, `.on_pointer_up(...)`, and
  `.on_pointer_move(...)` handlers expose concrete event data for authoring
  paths that need button or position details.
- Pointer handlers should return their handler result only when consumed or
  cancelled; otherwise the wrapper can continue forwarding to its child, which
  keeps future bubbling/ancestor propagation work from being boxed in too early.

## 2026-06-30 Element Builder Fluent Style Shortcuts

- Builder-level style shortcuts should remain thin mutations of the builder's
  retained `Style`; this keeps `.style(Style)` source-compatible and avoids a
  second style storage path.
- `.size(Size)` and `.size(float, float)` need to update both
  `style_.preferred_size` and `size_` when the builder kind is `fixed_size`,
  because `FixedSizeElement` build output reads `size_` rather than
  `style_.preferred_size`.
- `.gap(float)` can share the retained style path because row, column, and
  vertical stack builders already copy `style_.gap` into their concrete element
  before appending children.

## 2026-06-30 Remaining Steps 93-128 Plan Shape

- The user-facing "后 40 步" still refers to the Step 89-128 target band, but
  Steps 89-92 are already complete on `master`; the active implementation
  queue is therefore Steps 93-128.
- The safest order is still mostly sequential because Step 93-98 authoring and
  render entry unlock model/app lifecycle work, Step 99-108 lifecycle unlocks
  child-view ancestry, and Step 109-118 interaction/layout metadata feeds the
  Step 119-128 renderer/platform/demo hardening work.
- Step 124-127 are the highest platform-risk region because they cross from
  pure command/runtime metadata into Win32 and Wayland backend behavior; keep
  memory clipboard and deterministic tests available while platform backends
  mature.

## 2026-06-30 Style Unit And Color Helpers

- `px(float)` can remain a direct `float` identity helper for now because every
  current style/layout field already stores pixel-like lengths as `float`.
- Free `edges(...)` helpers should forward to the existing `EdgeSizes::all`,
  `EdgeSizes::axes`, and `EdgeSizes::trbl` constructors so Step 93 builder
  shortcuts can use concise authoring names without adding another edge type.
- `rgb` and `rgba` normalize integer 0-255 color channels into the existing
  floating `Color` representation; alpha remains a `float` so authored opacity
  composes with the current `Color::a` field directly.

## 2026-06-30 Steps 89-128 Execution Planning

- The remaining Windows/Linux core API work is best sequenced in four bands:
  authoring/render entry first, then model/app/window lifecycle, then
  event/layout depth, then renderer/text/platform hardening and the public demo.
- Step 92 should stay deliberately small: inline `px`, `rgb`, `rgba`, and
  `edges(...)` helpers can layer over the existing `float`, `Color`, and
  `EdgeSizes` primitives without adding a new length type yet.
- Step 93 can then mutate the builder's retained `Style` directly, because
  `ElementBuilder` already owns `style_` and builds `StyledElement`, flex,
  stack, and text variants from that data.
- Steps 96-98 should preserve the existing `View::paint(...)` contract while
  adding `View::render(ViewContext&)`; this lets current tests and demos keep
  compiling while the GPUI-like render path becomes available.
- Before implementing Step 92, the Step 91 feature worktree still needs the
  post-merge WSL verification/cleanup on `master`.

## 2026-06-30 Element Child Overloads

- `AnyElement` is already `std::unique_ptr<Element>`, so the existing
  base-ownership overload is the public `AnyElement` path; Step 91 only needs
  extra overloads for builder materialization and typed derived ownership.
- `.child(ElementBuilder)` should call `into_element(...)` internally, keeping
  all builder-to-owned conversion behavior in the same public convention added
  in Step 89.
- The typed ownership template should exclude `Element` itself so normal
  `std::unique_ptr<Element>` and `AnyElement` calls continue using the existing
  non-template overload and overload resolution stays predictable.

## 2026-06-30 Element Authoring Factories

- Free factories should return `ElementBuilder`, not `AnyElement`, because the
  existing fluent builder methods still need to compose before ownership is
  materialized through `into_element(...)` or `build()`.
- `div()` maps directly to the current styled-box builder, `h_flex()` and
  `v_flex()` map to row/column flex builders, `v_stack()` maps to the stack
  builder, and `text(TextModel&)` preserves the existing non-owning text-model
  binding convention.
- Keeping these helpers in `element.hpp` makes them visible through both
  `cgpui/ui/element.hpp` and the public prelude without creating another
  authoring header before the child-conversion overloads land.

## 2026-06-30 AnyElement Authoring

- `AnyElement` can be a public alias for the existing
  `std::unique_ptr<Element>` ownership model, which keeps Step 89 source
  compatible with all current tree, builder, and runtime installation APIs.
- `into_element(ElementBuilder)` should build by value from the existing
  rvalue-qualified `ElementBuilder::build()` path; this establishes the common
  conversion convention without adding Step 90 factory helpers yet.
- Keeping `into_element(AnyElement)` as the owned-element identity overload
  gives later child overloads and free factories one public spell to target
  while avoiding reference-counted ownership or type erasure churn.

## 2026-06-30 128-Step Planning

- The next highest-leverage path toward a GPUI-like core API is authoring
  ergonomics first: `AnyElement`, free element factories, child conversion, and
  fluent style shortcuts make later demos and tests express the intended public
  surface instead of the current lower-level builder plumbing.
- View/model/app lifecycle should follow once authoring helpers exist, because
  `View::render(ViewContext&)`, model update invalidation, and app window
  opening all need a stable element-returning convention.
- Renderer and platform work should remain command- and abstraction-driven:
  add paint/text/clip/cursor commands at the UI/runtime boundary before
  teaching Vulkan, Win32, or Wayland to consume them.
- Full macOS parity is still not part of Steps 89-128; each new API should stay
  Mac-neutral and avoid introducing fresh Vulkan/Win32/Wayland assumptions into
  public core headers.

## 2026-06-30 Mac Readiness Audit

- The repository already has macOS source slots:
  `src/platform/macos/macos_application.mm` creates a Cocoa `NSWindow` backed
  by `CAMetalLayer`, and `src/renderer/metal/metal_renderer.mm` accepts
  `MetalSurfaceHandle`.
- `xmake.lua` already gates macOS platform and Metal renderer sources under
  `is_plat("macosx")`, so Step 88 does not need to move files or alter
  Windows/Linux build branches.
- The useful boundary improvement is a small public target map:
  Windows/Win32 -> Vulkan, Linux/Wayland -> Vulkan, and macOS/Cocoa -> Metal.
- Full Mac parity is still intentionally out of scope for this slice; the
  next Windows/Linux API steps should stay renderer-neutral above the existing
  `Renderer` and `NativeSurfaceHandle` abstractions.

## 2026-06-29 Baseline

- Current UI runtime has a single root `View`, a stable root `ViewId{1}`, input snapshots, keyboard focus owner, and pointer capture owner.
- `View::handle_event` currently returns `void`; changing this to an event result is the next routing prerequisite.
- Existing tests are concentrated in `tests/ui/window_runtime_test.cpp` and can verify runtime behavior without real platform windows.
- The main worktree has a pre-existing untracked `.vscode/` directory that should remain untouched.

## 2026-06-29 Event Dispatch Observability

- A dispatch observation can stay entirely in the UI runtime layer: no Win32,
  Wayland, or Vulkan platform changes are needed.
- Runtime lifecycle events handled internally by `WindowRuntime` should not
  produce view dispatch records.
- `WindowRuntimeContext::last_event_dispatch` intentionally reports the
  previous view-dispatched event while a view is handling the next event; the
  after-event callback sees the current record after it has been stored.

## 2026-06-29 Event Routing Shell

- A root-only router is enough for the current single-view runtime while
  establishing the API shape that later element/view routing can replace.
- `WindowRuntimeContext::event_route` is current during `View::handle_event`,
  while `last_event_dispatch` remains the previous completed dispatch until the
  view returns.
- `EventDispatchRecord` now duplicates `view_id`/`event_kind` as compatibility
  fields and stores the full `EventRoute` for future routing expansion.

## 2026-06-29 Entity Store Skeleton

- The entity store can live in `cgpui/core/entity.hpp` as a header-only
  template without adding a source file to `cgpui_core`.
- Monotonic, non-reused ids keep the first skeleton simple and avoid stale-id
  aliasing until a future generation/index scheme is justified.
- A typed `EntityId<T>` gives compile-time separation between stores while
  preserving a simple numeric `.value` for tests and diagnostics.

## 2026-06-30 Context Entity Access

- `WindowRuntimeContext` is acting as a capability handle: it can stay `const`
  while still forwarding controlled mutations into runtime-owned state.
- `WindowRuntime` can hold per-type entity stores behind
  `std::unordered_map<std::type_index, std::any>` without adding new source
  files or platform-specific code.
- Missing or removed entities should stay soft-fail APIs for now:
  read/mutate return `nullptr`, and remove returns `false`.

## 2026-06-30 View Identity Allocation

- Step 6 only needs identity allocation, not a view tree or reconcile pass.
- Keeping `ViewId{0}` invalid, root at `ViewId{1}`, and runtime allocation
  from `ViewId{2}` gives later element/view storage a simple stable id source.
- A runtime-local monotonic counter is enough until later steps add removal,
  reconciliation, or generation/index semantics.

## 2026-06-30 Element Base API

- `ElementId` should stay separate from `ViewId`; element tree ids and view
  routing ids are related later, but they are not the same type at this layer.
- `Element` should not allocate its own ids. The next tree/reconcile steps can
  assign ids through `assign_id()` and preserve them across rebuilds.
- Keeping the base class free of layout/event methods avoids coupling Step 7 to
  the later layout and routing slices.

## 2026-06-30 Element Tree Container

- `ElementTree` can remain header-only for now because it stores generic
  `Element` ownership and simple relationship vectors.
- Replacing the root clears existing node relationships but keeps monotonic id
  allocation moving forward, which avoids stale-id aliasing.
- The container intentionally rejects unknown parents softly with
  `ElementId{0}`; richer diagnostics can wait until tree construction APIs are
  more expressive.

## 2026-06-30 Element Reconcile Pass

- A parent-local index reconcile is enough for the first pass because Step 8
  already records ordered children.
- Reconcile should replace element instances while preserving their assigned
  ids; this gives later layout/routing steps stable ids across simple rebuilds.
- Keyed diffing, pruning, and type-aware compatibility checks are intentionally
  outside this slice.

## 2026-06-30 Layout Primitives

- Layout primitives can live in a separate `cgpui/ui/layout.hpp` header so the
  element tree does not grow layout responsibilities before Step 11.
- Using zero min constraints and infinite max constraints gives unconstrained
  layout callers a simple default while preserving explicit min/max tests.
- `constrain_size` is intentionally only a per-axis clamp helper; element layout
  methods, bounds storage, and traversal remain later slices.

## 2026-06-30 Fixed Size Layout

- Adding layout as a virtual method on `Element` gives later stack and routing
  steps a common leaf/container contract without adding tree traversal yet.
- The base element layout returns constrained zero size, which keeps existing
  test-only elements usable while giving constraints a visible effect.
- `FixedSizeElement` is deliberately leaf-only: it stores a preferred size and
  delegates all min/max behavior to `constrain_size`; bounds persistence and
  child layout remain future slices.

## 2026-06-30 Vertical Stack Layout

- A self-contained `VerticalStackElement` is enough for Step 12; it avoids
  coupling layout traversal to `ElementTree` before bounds and hit testing
  exist.
- The first stack pass can lay out children with default unconstrained
  `LayoutInput` and only clamp the final stack size; constraint propagation can
  become a later refinement when richer layout rules exist.
- The stack intentionally does not persist child origins or bounds yet, because
  Step 13 is the first slice that needs retained layout geometry.

## 2026-06-30 Hit Testing

- Retained bounds can stay on `Element` as mutable state updated by const
  `layout()` calls, matching the existing const layout API without adding a
  renderer or runtime dependency.
- `hit_test(Point)` should soft-fail with `ElementId{0}` before layout or
  outside the retained bounds, which keeps Step 14 pointer routing simple.
- `VerticalStackElement` can record direct child bounds while laying out owned
  children and use child-first hit testing; richer nested coordinate propagation
  can wait until element-tree traversal or more complete layout context exists.

## 2026-06-30 Pointer Hit Routing

- The existing event routing shell can carry an optional `ElementId` without
  changing root-view delivery yet.
- A non-owning runtime element root is enough for this slice because ownership,
  reconcile, and layout scheduling are still outside the runtime.
- Pointer capture should not be folded into hit routing yet; Step 15 can decide
  when capture should override a fresh hit-test result.

## 2026-06-30 Pointer Capture Routing

- Element-level capture can coexist with the older root-view capture state by
  storing a separate optional `ElementId` owner.
- Capture should override fresh hit testing only for pointer events, preserving
  non-pointer routes.
- Owner-matched release keeps the API soft-fail and mirrors the existing
  view-owner release behavior.

## 2026-06-30 Keyboard Focus Routing

- Element-level keyboard focus can reuse `EventRoute::target_element_id`, just
  like pointer routing, because the route already represents the event's
  intended element target.
- Keyboard and text-input events should use the focused element owner before
  any pointer-specific routing logic; pointer hit testing and pointer capture
  remain unaffected.
- Keeping view-level and element-level focus owners as separate optional fields
  preserves old root-view focus tests while exposing the element owner needed
  for later key binding and text-input slices.

## 2026-06-30 Action Command Primitive

- A runtime-local action handler table is enough for the first primitive and
  avoids a global registry before key binding or view trees exist.
- Reusing `EventResult` for action handlers keeps consumed/cancelled semantics
  aligned with event handling.
- Recording the last `ActionDispatchResult` gives Step 18 a simple observable
  target when key bindings start dispatching named actions.

## 2026-06-30 Key Binding Table

- A runtime-local vector of bindings is enough for exact key/modifier matching
  in the first slice; scoped binding tables and priorities can wait.
- Dispatching the matched action before root view event handling gives action
  handlers access to the current `EventRoute` while preserving existing view
  event delivery.
- Matching only `KeyboardKey` events keeps text input independent until the text
  model and focused-text routing slices land.

## 2026-06-30 Text Model

- A byte-offset cursor keeps the first text model small while still allowing
  UTF-8-safe codepoint navigation by skipping continuation bytes.
- The source test should stay ASCII on Windows; UTF-8 examples can be expressed
  with byte escapes to avoid MSVC code-page warnings.
- `backspace()` deletes the codepoint before the cursor; the test originally
  moved the cursor to offset 0 before expecting a successful second backspace,
  which was a test expectation bug rather than an implementation bug.

## 2026-06-30 Text Input Routing

- Binding text models by `ElementId` lets the Step 16 keyboard-focus route
  become actionable without introducing element event handlers yet.
- Runtime-owned text model pointers should remain non-owning in this slice; the
  owning element/view layer does not exist yet.
- The text-input routing test must fire the third key event before expecting
  the existing helper to release element keyboard focus; otherwise the second
  text input correctly still targets the model.

## 2026-06-30 40-Step Milestone

- Step 21 should introduce style as inert data first, not layout or painting
  behavior, so later builder, layout, and paint-tree steps have a stable shared
  vocabulary.
- Style primitives can stay header-only in `include/cgpui/ui/style.hpp` and
  reuse `Color` from `core/geometry.hpp` to avoid duplicating color types.
- A single `EdgeSizes` structure is enough for padding/border-width symmetry
  and keeps Step 24 padding layout and Step 25 border primitives aligned.

## 2026-06-30 Element Builder API

- `StyledElement` can be introduced as an inert wrapper first: it owns style
  data and an optional child, but does not paint backgrounds or apply padding
  until the dedicated Step 23 and Step 24 slices.
- Keeping `ElementBuilder::build()` returning `std::unique_ptr<Element>` matches
  the existing tree/container APIs and avoids introducing reference-counted
  element ownership prematurely.

## 2026-06-30 Styled Background Paint

- Element-level painting can reuse the existing `PaintList` command vocabulary
  without changing renderer or platform code.
- Keeping `Element::paint(PaintList&)` as a no-op default preserves existing
  test elements while letting styled/container elements opt into paint output.
- `StyledElement` should emit its background before painting its child, matching
  normal parent-background-then-content ordering and setting up Step 26 paint
  tree traversal.

## 2026-06-30 Styled Padding Layout

- Padding can be handled inside `StyledElement::layout` before a separate box
  model abstraction exists: content size comes from the child or preferred size,
  then top/right/bottom/left padding expands the outer size.
- Child bounds should be rewritten after the child layout pass so hit testing
  and later paint traversal observe the padded child origin.
- Existing layout constraints in this codebase clamp container output after
  child measurement, so this slice keeps the same simple constraint behavior.

## 2026-06-30 Border Style Primitives

- Border color and radii can be introduced as inert `Style` data without
  touching layout or paint yet; Step 26/27 can decide how much of the box model
  to materialize in paint commands.
- `BorderRadii` mirrors `EdgeSizes` as a compact POD helper, keeping top-left,
  top-right, bottom-right, and bottom-left explicit for later rounded-rect work.
- `Style::border_width` already existed from Step 21, so this slice only needed
  color and radius fields plus fluent setters.

## 2026-06-30 Element Paint Tree

- `ElementTree::paint(PaintList&)` can be a thin recursive traversal over the
  existing parent/child relationship data; no renderer or platform changes are
  needed for this slice.
- Painting an element before its children preserves the Step 23 parent
  background before content ordering.
- Sibling paint order follows the stored child id vector, matching append and
  reconcile order until a later z-order primitive changes that policy.

## 2026-06-30 Clip Overflow Primitives

- Clip and overflow can start as inert `Style` data; applying clip stacks to
  paint traversal is a later renderer/painter concern.
- `Overflow::visible` as the default preserves all existing layout, hit-test,
  and paint behavior until explicit hidden overflow is requested.
- Keeping `clip_rect` optional distinguishes "no authored clip" from an
  authored zero-size clip rectangle, which will matter when paint traversal
  starts honoring clipping.

## 2026-06-30 Z Order Paint Order

- `Style::z_index` is the smallest API surface for authored paint ordering
  because `StyledElement` already owns style data and `ElementTree::paint`
  already controls sibling traversal.
- Sorting only same-parent children keeps the existing parent-before-children
  invariant from Step 26 while making sibling paint order deterministic.
- `std::stable_sort` preserves insertion/reconcile order for equal z-index
  siblings, so existing append-order paint tests remain meaningful.

## 2026-06-30 Hover State Tracking

- Hover can be modeled as input state first: no hover enter/leave event types
  are needed until element handlers exist.
- Pointer capture should override event routing, not the physical hover target;
  the runtime therefore computes hover from the live pointer hit test before
  applying capture to `EventRoute`.
- Clearing hover on pointer-move misses keeps stale element ids from surviving
  after the pointer leaves the current element root.

## 2026-06-30 Cursor Shape State

- Cursor shape can start as runtime-visible input state before platform cursor
  handles are wired into Win32 and Wayland.
- Binding cursor shapes by `ElementId` composes directly with the Step 29 hover
  state and avoids inventing element event handlers in this slice.
- Restoring `CursorShape::default_arrow` on hover misses prevents stale cursor
  state from outliving the hovered element.

## 2026-06-30 Scroll State Model

- The first scroll primitive can remain UI-layer and header-only: it only needs
  viewport size, content size, and a clamped offset.
- Offsets should reclamp whenever viewport or content size changes so stale
  scroll positions cannot survive after content shrinks.
- Axis scrollability is derived from positive max offset, not from the current
  offset, which keeps `can_scroll_x`/`can_scroll_y` useful at both ends of a
  scroll range.

## 2026-06-30 Flex Layout Basics

- `FlexElement` can start as a sibling to `VerticalStackElement` rather than a
  replacement: it keeps row/column semantics explicit without disturbing the
  existing vertical-stack tests.
- The first flex slice should only measure natural child sizes and clamp the
  container output; grow, shrink, gap, align, and justify behavior can remain
  future work.
- Reusing the same child-first hit testing pattern as `VerticalStackElement`
  keeps routed pointer behavior consistent once flex elements are installed as
  runtime element roots.

## 2026-06-30 Runtime Invalidation

- Runtime invalidation can start as observable dirty state without scheduling
  redraws yet; Step 34 can connect dirty state to platform redraw requests.
- A layout invalidation should imply paint invalidation because a layout pass
  changes paint bounds, while a paint invalidation alone should not imply
  layout work.
- Clearing invalidation explicitly keeps tests and future frame processing able
  to model "dirty work consumed" separately from "new work requested".

## 2026-06-30 Update Scheduling

- Invalidation-triggered redraw should be deferred until after view event and
  after-event callbacks finish; the test fake window synchronously emits redraw
  events, so immediate requests would reenter the runtime too early.
- A single `redraw_scheduled_` flag is enough to coalesce layout and paint
  requests made during the same event into one platform redraw request.
- Successful redraw should clear both invalidation and scheduling state so
  later model/view changes can schedule the next frame cleanly.

## 2026-06-30 View Model Subscriptions

- A first subscription skeleton can be type-erased with `std::type_index` plus
  the typed `EntityId<T>::value`; this keeps the API typed while avoiding a
  full dependency graph.
- Querying subscriptions by `ViewId` is enough for tests and future debugging;
  notification can scan the small vector until a richer storage model is
  needed.
- Entity-change notification should request layout rather than paint only,
  because model changes may affect both layout and paint output.

## 2026-06-30 Text Selection Range

- Text selection can live entirely in `TextModel` as byte offsets for this
  slice, matching the existing byte-offset cursor while preserving UTF-8-safe
  movement helpers.
- Storing anchor and head separately gives Step 37 enough state for
  Shift+movement behavior, while `selection()` exposes a normalized start/end
  range for edit operations.
- Plain insertion should replace a non-collapsed selection and then collapse to
  the inserted text end, matching typical text editing behavior without adding
  platform clipboard or IME coupling yet.

## 2026-06-30 Text Edit Actions

- A small `TextEditAction` enum keeps keyboard editing semantics in the
  platform-neutral text model before binding them to Win32 or Wayland key
  events.
- Selection extension should preserve the original anchor and move only the
  head/cursor, which gives later Shift+Arrow routing the right primitive.
- `backspace()` and `delete_forward()` should erase a non-collapsed selection
  before falling back to adjacent-codepoint deletion, so direct text-model calls
  and edit-action dispatch behave consistently.

## 2026-06-30 Clipboard Abstraction

- The first clipboard slice can be a platform-layer text contract, independent
  from `PlatformApplication`, so the API is usable in tests and can later be
  backed by Win32 or Wayland system clipboard protocols.
- A shared `MemoryClipboard` implementation is enough for the Step 38 skeleton;
  platform-specific system integration can replace the factory implementation
  without changing callers.
- Keeping the factory in `cgpui_platform` instead of the Win32/Wayland leaf
  targets avoids duplicate definitions when examples link both shared platform
  code and a platform-specific implementation.

## 2026-06-30 IME Composition Skeleton

- IME composition should be a separate platform event from `TextInput` because
  update/preedit text is transient while commit text mutates the text buffer.
- Composition events should use the same keyboard-focus element route as key
  and text-input events; this lets Win32 and Wayland feed future IME messages
  into the focused text model without inventing a parallel routing path.
- The first text-model composition state can stay as one optional UTF-8 string:
  update replaces the preedit text, commit inserts it through normal text
  insertion, and cancel clears it without changing the underlying buffer.

## 2026-06-30 New API Demo

- The demo can exercise the new API surface without waiting for text shaping:
  it paints element rects, a cursor rect derived from `TextModel::cursor`, and a
  composition underline derived from the IME composition state.
- Runtime installation can stay non-owning for Step 40 by passing the demo's
  owned root element to `WindowRuntime::set_element_root`; later runtime-owned
  element-tree steps can replace that with a safer ownership model.
- `CGPUI_DEMO_INJECT_TEXT` gives the demo a scripted state-change path for
  smoke tests while keeping real interactive behavior available in normal runs.

## 2026-06-30 ElementTree Root Layout

- `ElementTree::layout_root` is intentionally a root helper, not a full tree
  traversal: existing container elements still own their child layout behavior.
- Empty-tree layout should return constrained zero size, matching base element
  layout semantics and giving callers a safe no-root path.

## 2026-06-30 ElementTree Root Hit Testing

- `ElementTree::hit_test_root` mirrors `layout_root`: it is a safe root-level
  convenience, while element/container implementations still define hit-test
  traversal and child priority.
- Empty-tree hit testing should return `ElementId{0}` so runtime routing can
  distinguish "no element root" from a valid target.

## 2026-06-30 Runtime-Owned Element Tree

- Runtime can support both the old non-owning `set_element_root` path and the
  new owning `set_element_tree` path by making them mutually exclusive; this
  avoids stale pointers when ownership changes.
- Pointer routing should use `ElementTree::hit_test_root` when an owned tree is
  installed, preserving the legacy `Element::hit_test` path for external roots.

## 2026-06-30 Runtime-Owned Tree Layout

- Redraw is the first safe place to auto-layout an owned element tree because
  the runtime has current framebuffer/viewport size and can update retained
  bounds before later pointer events rely on hit testing.
- Using viewport size as the max layout constraint preserves existing element
  min defaults while preventing oversized preferred roots from exceeding the
  drawable area.

## 2026-06-30 Element Event Hook

- `EventResult` belongs in the element-visible API once elements can handle
  events; keeping `ui.hpp` as a consumer avoids duplicating result semantics.
- The first `ElementEventContext` only needs the routed target element id. View
  runtime/application access can be layered later through a richer context.
- `StyledElement` forwarding to its child preserves wrapper composition until
  styled boxes grow their own interaction behavior.

## 2026-06-30 Runtime Element Event Dispatch

- Runtime event dispatch can treat element handling as the first phase and the
  existing root view as fallback, preserving old view behavior for unhandled
  element events.
- Owned `ElementTree` can resolve arbitrary routed ids with `get(id)`, while
  the legacy non-owning root can only safely dispatch to the root id itself.
- `EventDispatchRecord::result` should represent the final handler result,
  whether that result came from an element or from the fallback view.

## 2026-06-30 Element Builder Flex Helpers

- `ElementBuilder` can grow a small kind discriminator without changing the
  existing `box()` call sites; `box()` still builds a `StyledElement`.
- Row and column helpers should build concrete `FlexElement` containers so
  existing flex layout, hit testing, and future gap support remain centralized.
- Keeping box as a single-child wrapper preserves the earlier styled-box API
  while allowing row/column helpers to accept multiple chained children.

## 2026-06-30 Element Builder Stack Helper

- The same builder kind discriminator used for flex helpers can cover
  `v_stack()` without changing `VerticalStackElement` itself.
- Building a concrete `VerticalStackElement` keeps older stack-specific tests
  and future stack-only gap behavior independent from generic flex containers.
- The builder's accumulated child vector is now the shared path for multi-child
  container helpers.

## 2026-06-30 Element Builder Fixed-Size Helper

- `fixed_size(Size)` fits the existing builder kind model as a leaf builder,
  not a styled box variant.
- Building a concrete `FixedSizeElement` preserves the existing constraint and
  retained-bounds behavior without adding a second preferred-size path.
- The helper intentionally ignores accumulated children because fixed-size
  elements are leaf nodes in the current element model.

## 2026-06-30 Gap Style Layout

- `Style::gap` can stay as a single main-axis spacing primitive for now:
  vertical stack and flex column apply it on Y, while flex row applies it on X.
- Applying gap only when `child_index > 0` preserves existing zero-gap layout
  and avoids adding trailing space after the final child.
- The existing builder `style(...)` hook is enough to author container gap for
  `row`, `column`, and `v_stack` without wrapping those multi-child containers
  inside an extra `StyledElement`.

## 2026-06-30 Margin Style Layout

- Reusing `EdgeSizes` for `Style::margin` keeps the box-model vocabulary aligned
  with padding and border width without adding another edge type.
- Margin is modeled as outer size in `StyledElement::layout`; existing no-margin
  tests preserve prior behavior because the default edge values are zero.
- Child layout bounds should include the margin offset before padding so later
  hit testing and paint commands see content positioned inside the outer box.

## 2026-06-30 Border Paint Emission

- Border rendering can reuse the current `SolidRect` paint command vocabulary:
  styled borders are four filled rects emitted after background paint.
- Requiring `border_color` before emitting border rects preserves the previous
  inert behavior for authored widths that do not yet specify a color.
- Keeping side rects between top and bottom avoids double-painting corners while
  still giving Vulkan/Wayland and Win32 the same command stream shape.

## 2026-06-30 Overflow Clip Metadata

- Clip handling can start as paint-command metadata: renderers can continue
  drawing `SolidRect` unchanged while future renderer slices learn to honor the
  optional `PaintCommand::clip_rect`.
- A simple `PaintList` clip stack is enough for nested styled elements and keeps
  clip state out of individual element implementations beyond push/pop scope.
- Hidden overflow should use the element's retained layout bounds by default,
  while authored `Style::clip_rect` can override the metadata rect when present.

## 2026-06-30 Text Element Skeleton

- The first `TextElement` can stay in the element layer as a non-owning
  `TextModel*` binding; runtime-owned text-model routing already uses the same
  lifetime convention.
- A fixed 8x16 skeleton metric is enough to make layout, hit testing, and paint
  observable without introducing shaping, fonts, glyph atlases, or renderer text
  commands in this slice.
- Paint can emit a placeholder `SolidRect` through the existing paint path so
  Win32/Vulkan and Wayland/Vulkan remain unchanged until a later text renderer
  command is justified.

## 2026-06-30 Runtime Text Edit Actions

- Runtime text edit routing can mirror key bindings as a platform-neutral
  binding table from `KeyboardKey` data to `TextEditAction`, avoiding hard-coded
  Win32 or Wayland key assumptions in the text model.
- The existing element keyboard-focus owner is the right target selector:
  keyboard events already route to it, and text input/IME routing already look
  up text models by that focused `ElementId`.
- Matching edit bindings before view fallback keeps the model updated before
  observers inspect the event, while leaving unbound keys and missing text
  models as no-op cases for compatibility.

## 2026-06-30 Runtime Clipboard Paste

- Runtime clipboard paste can use the existing platform-neutral `Clipboard`
  interface without introducing Win32 or Wayland system clipboard details into
  the UI layer.
- Keeping the clipboard pointer non-owning matches the runtime's current
  non-owning `TextModel*` binding convention and lets tests inject
  `MemoryClipboard` directly.
- Paste should target the focused element's bound `TextModel`, so it composes
  with the same keyboard-focus route used by text input, IME, and edit actions.

## 2026-06-30 Runtime Clipboard Copy

- Copying selection text should live on top of `TextModel` selection state
  rather than duplicating byte-range normalization in the runtime.
- The runtime clipboard copy path can share the same non-owning `Clipboard*`
  and focused-element text-model lookup introduced for paste.
- Collapsed selections should not overwrite clipboard contents; returning
  `false` gives callers a simple observable no-op.

## 2026-06-30 Runtime Clipboard Cut

- Cut can compose copy plus the existing selection-aware
  `TextModel::delete_forward`, which avoids adding a second selected-range
  deletion primitive in the runtime.
- The operation should only mutate text after clipboard write succeeds, so
  failed copy/no selection/no focused model leaves the buffer untouched.
- After deletion, existing `TextModel` behavior collapses the cursor to the
  removed selection start, matching the text-edit action path.

## 2026-06-30 ViewContext Convenience

- A type alias keeps the public `ViewContext` spelling GPUI-like without
  duplicating runtime context storage or breaking existing
  `WindowRuntimeContext` callbacks.
- Thin context methods are enough for common authoring paths: they forward to
  the existing runtime APIs and preserve all focus, capture, clipboard, and
  invalidation behavior.
- Invalidation convenience tests should observe state during view event
  handling, not after-event callbacks, because deferred redraw can flush and
  clear invalidation before `after_event` returns.

## 2026-06-30 App Runner Skeleton

- The first public runner can sit above `WindowRuntime` rather than replacing
  it: the runner owns the renderer, while `WindowRuntime` still owns event
  dispatch and redraw behavior.
- A setup callback is enough for this slice to let callers bind elements,
  models, actions, cursors, and callbacks before the platform event loop starts.
- Accepting an injected `PlatformApplication&` and renderer factory keeps the
  runner deterministic in tests and leaves default platform/renderer creation
  for a later convenience overload.

## 2026-06-30 Text Element Builder

- `ElementBuilder::text(TextModel&)` should mirror `TextElement`'s non-owning
  binding instead of copying text, so runtime text-model bindings and builder
  authored text share the same lifetime convention.
- Building text as a direct leaf keeps the initial API small; styling can stay
  in wrapper elements until text-specific font, color, and shaping primitives
  exist.

## 2026-06-30 Focusable Element Hook

- Focus activation belongs first at the element boundary: runtime click routing
  can query `Element::focusable()` and call `Element::focus(...)` without
  baking widget-specific behavior into `WindowRuntime`.
- Keeping the default element non-focusable preserves existing hit-test and
  event-routing behavior for passive layout and paint elements.

## 2026-06-30 Runtime Click Focus

- Runtime focus activation can reuse the already-computed event route: after
  pointer capture/hit-test routing chooses a target element, a left-button press
  is enough to request element keyboard focus and call its focus hook.
- Calling `focus(...)` before normal element event dispatch lets the current
  pointer event and the following keyboard event observe the updated focus
  owner through the existing `WindowRuntimeContext` shape.

## 2026-06-30 Element Enabled State

- Enabled/disabled state fits best on `Element` rather than `Style`: it is an
  interaction primitive first, and Step 65 can use the same base API to skip
  dispatch without inventing style-aware event routing.
- Builder-level `enabled(bool)` should apply to every concrete element kind so
  disabled state stays orthogonal to layout choice, text leaves, and styled
  wrappers.

## 2026-06-30 Disabled Element Event Gate

- Disabled state is now a routing/interaction gate rather than a hit-test gate:
  route metadata can still identify the hit element, but the disabled element's
  handler is skipped.
- View fallback still observes the original event route after a disabled routed
  element is skipped, which preserves existing dispatch diagnostics and lets
  parent/view-level behavior decide whether to consume the event.
- `StyledElement` treats a disabled child the same as no interactive child for
  event forwarding, returning unhandled without mutating child event state.

## 2026-06-30 ElementTree Preorder Traversal

- `ElementTree::preorder_ids()` should be structural, root first and then each
  stored child subtree in child-list order; it intentionally does not reuse
  paint order, which may sort siblings by z-index.
- Returning ids rather than element pointers keeps the helper simple,
  copyable, and compatible with later APIs that can call `get(id)` or filter by
  id without exposing internal nodes.
- Empty-tree traversal returns an empty vector, matching the existing soft-fail
  behavior of root layout, hit testing, and paint traversal.

## 2026-06-30 ElementTree Typed Lookup

- `ElementTree::find_as<T>(ElementId)` is a convenience over the existing
  id-based `get` contract, not a new ownership path; it returns null for
  missing ids and type mismatches.
- Providing both const and mutable overloads lets tests and future ViewContext
  helpers preserve const-correctness while avoiding repeated `dynamic_cast`
  boilerplate at call sites.

## 2026-06-30 ViewContext Text Model Binding

- ViewContext text binding is just a capability-forwarding helper over runtime
  text-model binding; it should not introduce a second ownership or lookup path.
- Binding during an event should be immediately visible to subsequent text input
  routed to the focused element, matching the existing runtime text input path.

## 2026-06-30 ViewContext Element Tree Installation

- ViewContext element tree installation should forward ownership directly into
  `WindowRuntime::set_element_tree`; the context remains a capability wrapper,
  not a second tree store.
- Installing a tree during an event composes with `request_layout()`: the
  deferred redraw lays out the owned tree before later pointer routing uses it.

## 2026-06-30 ViewContext Action Helpers

- Action registration and dispatch can live on `ViewContext` as thin forwards
  over the runtime action table, preserving the existing handler storage and
  last-dispatch observability.
- `ActionHandler` can be declared before `WindowRuntimeContext` is defined
  because it only needs the context by reference; this lets context methods use
  the public handler alias without adding another callback type.

## 2026-06-30 ViewContext Key Binding Helper

- Key binding through `ViewContext` can reuse the runtime binding table without
  changing keyboard dispatch order; a binding registered during one event is
  available to subsequent keyboard events.
- Keeping action registration and key binding as separate context helpers
  mirrors the runtime API and lets later authoring helpers compose them without
  adding a combined shortcut abstraction too early.

## 2026-06-30 ViewContext Text Edit Binding Helper

- Text-edit bindings registered from `ViewContext` can stay as direct forwards
  to the runtime binding table; current-key dispatch still happens before the
  view handler, so tests should assert effects on later key events.
- `TextModel(std::string)` already places the cursor at the end, which is enough
  for text-edit binding tests without adding a cursor-positioning helper.

## 2026-06-30 ViewContext Cursor Binding Helper

- Cursor binding through `ViewContext` should stay a thin capability forwarder
  over `WindowRuntime::set_element_cursor`; the runtime remains the single
  storage owner for element-to-cursor mappings.
- A cursor binding registered during one event is visible to later pointer
  hover routing, matching the existing runtime cursor state path and preserving
  platform-neutral behavior for Windows and Wayland.

## 2026-06-30 ViewContext Element Focus Helpers

- Short `ViewContext::focus(ElementId)` and `blur(ElementId)` helpers can layer
  over the existing element keyboard-focus request/release APIs without changing
  routing semantics.
- Keeping the long `request_keyboard_focus(ElementId)` APIs alongside the short
  helpers preserves compatibility while giving author code a more GPUI-like
  spelling for common element focus flows.

## 2026-06-30 ViewContext Pointer Capture Element Helpers

- Element pointer capture should be authorable with an `ElementId` overload on
  `ViewContext`, while the runtime continues storing the existing
  `PointerCaptureOwner` tagged owner.
- The overloads are intentionally additive: existing `PointerCaptureOwner`
  calls remain available for view-level capture and explicit owner matching.

## 2026-06-30 Element Builder Click Handler

- Builder-level click handling can be implemented as an opt-in wrapper element,
  preserving existing dynamic-cast expectations for elements built without a
  click handler.
- The wrapper should preserve child layout, paint, and hit-test behavior, while
  consuming pointer-press handling through the attached handler before falling
  back to the child for other events.
- Wrapper elements created by builder helpers must mirror the builder's
  enabled state on the wrapper itself, not only on the wrapped child, because
  direct event dispatch sees the outer element first.

## 2026-06-30 Element Builder Focusable Helper

- Builder-level focusability can also be an opt-in wrapper, preserving existing
  concrete element types unless the helper is requested.
- Wrapper composition order matters: behavior wrappers like click handling need
  to stay inside capability wrappers like focusability so `focusable()` remains
  visible on the built element while event handling still reaches the click
  handler.

## 2026-06-30 Element Builder Key Handler

- Element-level key handling can use the same opt-in wrapper pattern as click
  handling, but it should accept the concrete `KeyboardKey` alongside the
  element event context so author code does not need to inspect the variant.
- Key and click handlers should remain behavior wrappers inside focusability,
  preserving `focusable()` visibility when helpers are composed.

## 2026-06-30 Element Builder Disabled Helper

- `ElementBuilder::disabled()` is pure authoring sugar over `enabled(false)`;
  keeping it as a flag flip lets the existing wrapper enabled propagation
  handle composed elements consistently.

## 2026-06-30 ElementTree Enabled Traversal

- `enabled_preorder_ids()` filters by each node's current `enabled()` state at
  push time but still traverses descendants, which makes it a general enabled
  query rather than a disabled-subtree pruning policy.

## 2026-06-30 Disabled Focus Activation

- Disabled elements remain routable and observable through hit testing, but
  runtime click-to-focus activation should use the same interaction gate as
  element event dispatch: the element must be both enabled and focusable.

## 2026-06-30 Disabled Hover Cursor

- Hover state should continue to describe the physical hit target, but cursor
  shape is an interaction affordance and should only be driven by enabled
  elements when the runtime can resolve the hovered id to a live element.
- Legacy non-owning roots can hit-test child ids that `routed_element()` cannot
  safely resolve, so cursor lookup keeps the old behavior for unresolved ids
  and only suppresses cursor bindings for resolved disabled elements.

## 2026-06-30 Focused Text Model Lookup

- The runtime already treats element keyboard focus as the owner selector for
  text input, IME, text edit actions, and clipboard operations, so a public
  `focused_text_model()` helper can be the single soft-fail lookup for that
  binding.
- Clipboard paste/copy/cut can reuse the helper directly, reducing duplicate
  `keyboard_focus_element_owner_` and `text_models_` lookup logic before
  ViewContext grows focused-text mutation helpers.

## 2026-06-30 ViewContext Focused Text Mutation

- A focused-text mutation helper fits the existing capability-forwarding
  `ViewContext` style: it looks up the runtime's focused text model, invokes an
  author callback when present, and returns false for no callback or no focused
  model.
- Tests for the helper should use key-only event sequences because generic text
  input routing tests intentionally insert text between keyboard events.

## 2026-06-30 Public Prelude Header

- `include/cgpui/cgpui.hpp` can stay as a simple aggregate header over the
  existing module headers for now; no new source target or platform-specific
  selection is needed for this authoring convenience.
- Header-cleanliness coverage should instantiate a concrete `View`, which means
  the test fixture must implement both `paint` and `handle_event` from the
  current public `View` contract.
- The prelude intentionally exposes the existing API surface without changing
  ownership or runtime behavior, keeping this slice an additive source
  compatibility improvement.

## 2026-06-30 ViewContext Event Route Helper

- `ViewContext::current_event_route()` should be a snapshot accessor over the
  existing `event_route` field, not a live recomputation from `WindowRuntime`.
- Keeping the public field and adding the helper preserves compatibility while
  giving author code a more intentional GPUI-like route lookup spelling.
- The helper is valid in both view event handlers and after-event callbacks
  because `WindowRuntime::context()` already snapshots `current_event_route_`
  into the context object for both paths.

## 2026-06-30 ViewContext Input State Helper

- `ViewContext::input_state()` should mirror the existing `input` snapshot
  field, preserving the event-time view of focus, pointer, hover, and cursor
  state instead of querying mutable runtime state later.
- Returning the snapshot by value keeps the helper simple and avoids exposing a
  mutable alias into the context object.
- The helper is intentionally additive: existing direct `context.input` access
  remains available while author code gains a clearer accessor spelling.

## 2026-06-30 Steps 129-168 Follow-On Planning

- Steps 129-168 should remain queued behind completion of Steps 99-128 unless
  the roadmap is explicitly reprioritized; Step 129 depends on the model,
  window, route, text, platform, and demo surfaces that Steps 99-128 still need
  to land.
- The post-Step-128 queue is best treated as four 10-step product bands:
  context/entity/global/async ergonomics, keyed reconciliation and reusable
  widgets, text/font/renderer maturity, then Windows/Wayland platform closure
  and parity documentation.
- Step 168 should not claim full GPUI parity. Its exit condition is a practical
  Windows/Linux GPUI-core foundation plus an explicit audit of implemented,
  partial, missing, and Mac/Metal-deferred areas.

## 2026-06-30 Model Entity Aliases

- `Model<T>` and `Entity<T>` should remain pure public authoring aliases over
  `EntityId<T>` at this stage; introducing wrapper objects before Step 100/101
  would create ownership and upgrade semantics before the plan has the model
  helpers and weak handles to support them.
- Header-cleanliness coverage belongs in both `core_header_cleanliness` and the
  public prelude because the aliases are part of the low-level entity API and
  also need to be available to normal GPUI-like authoring code through
  `cgpui/cgpui.hpp`.
- Reusing `EntityId<T>` preserves the existing typed-id separation between
  unrelated model/entity payload types and keeps `EntityStore<T>` monotonic id
  behavior unchanged.

## 2026-06-30 ViewContext Model Helpers

- `new_model`, `read_model`, `update_model`, and `remove_model` fit best as
  header-only `ViewContext` authoring helpers over the existing entity store;
  this keeps Step 100 additive and avoids adding a second model storage path.
- `update_model` should soft-fail when the model id is missing, and should only
  notify model changes after it finds the model and runs the author callback.
  That lets subscribed views receive layout/paint invalidation without making
  failed updates schedule redraws.
- `remove_model` can delegate to existing entity removal semantics, including
  notifying subscribed views only when an actual model is removed.

## 2026-06-30 Back-40 Plan Boundary

- The "后40步" plan is Steps 129-168, not a replacement for the active
  Steps 101-128 queue. Step 129 remains gated on Step 128 being merged and
  verified on both Windows and WSL Arch Linux.
- The post-Step-128 work should keep the current four-band shape:
  context/entity/global/async first, keyed reconciliation/widgets second,
  text/font/renderer maturity third, and Windows/Wayland platform closure plus
  parity audit last.
- The exit condition for Step 168 is a practical Windows/Linux GPUI-core
  foundation with an explicit audit of completed, partial, missing, and
  Mac/Metal-deferred areas; it should not claim full upstream GPUI parity.

## 2026-06-30 Weak Entity And View Handles

- `WeakEntity<T>` should remain a lightweight typed-id wrapper at this stage.
  Upgrade behavior belongs on `WindowRuntime`/`ViewContext`, where the runtime
  can check whether the model/entity store still contains the id.
- `WeakView` can mirror that shape for `ViewId`; the current liveness check is
  `is_view_id_allocated(...)`, which matches the existing monotonic view-id
  allocation model until Step 107 introduces a real multi-view registry.
- Step 101 intentionally does not add observer ownership, subscription tokens,
  or entity handle convenience methods. Those remain Step 102/133/130 work and
  would overbuild this slice.

## 2026-06-30 Back-40 Entry Contract Refresh

- The后 40 步 plan should stay an execution-ready queue for Steps 129-168, not
  an active implementation plan while Step 102-128 still have unresolved
  runtime, model, event, layout, renderer, platform, and demo work.
- The important boundary is the Step 128 handoff: Step 129 should start only
  after the public-prelude demo, model invalidation, view/app lifecycle,
  routing/focus/scroll/layout depth, text command metadata, cursor/clipboard,
  and IME geometry are all merged and verified on Windows and WSL Arch Linux.
- Since Step 102 already implements observation callbacks in its feature
  worktree, the next active implementation step is Step 103: make model
  updates/removals invalidate subscribed views automatically while preserving
  Step 102 observer callback semantics.

## 2026-06-30 Model Observe Helper

- Model observers should be public typed callbacks at the authoring boundary
  (`ModelObserver<T>`) and erased only inside the runtime storage layer, where
  `std::type_index` plus the raw entity id can match notifications without
  duplicating stores per model type.
- Observing a missing model should soft-fail and avoid registering the
  callback. That keeps stale ids from creating future callbacks that look
  alive but cannot read their model.
- Step 102 intentionally keeps automatic subscribed-view invalidation out of
  the observe helper. `notify_entity_changed(...)` still preserves the
  existing layout invalidation behavior, while Step 103 can formalize the
  model-driven subscribed-view invalidation contract without changing the
  observer callback shape.

## 2026-06-30 Model Update Invalidates Subscribed Views

- Once `View::render(ViewContext&)` is the GPUI-like element-tree authoring
  entry point, model/entity notifications for subscribed views need render
  invalidation, not only layout/paint invalidation. Otherwise a model update
  can schedule a frame without rebuilding the rendered element tree.
- `request_render()` is the narrowest existing runtime primitive for this
  contract because it already sets render, layout, and paint invalidation and
  schedules redraw. Reusing it keeps Step 103 small and avoids introducing a
  separate subscribed-view invalidation queue before the view registry lands.
- Observer callbacks remain part of `notify_entity_changed(...)`; Step 103
  changes the invalidation level after a match, while preserving Step 102's
  callback timing and missing-model soft-fail behavior.

## 2026-06-30 Back-40 Planning After Step 103

- The post-Step-128 "back 40" plan is Steps 129-168. It should remain a
  follow-on execution queue, not a replacement for the active Step 104-128
  work.
- Step 129 must stay gated on a clean Step 128 merge plus Windows and WSL Arch
  full debug verification, because the back-40 depends on the AppContext,
  WindowOptions, view registry, routing, layout, text command, cursor,
  clipboard, IME, and public-demo surfaces still scheduled in Steps 104-128.
- The current state for planning is Step 103 complete on `master` at
  `5949c84`, with Step 104 already opened in `.worktrees/app-context-wrapper`
  and baseline targeted tests passing.

## 2026-06-30 AppContext Setup Wrapper

- `AppContext` can start as a deliberately thin public setup wrapper containing
  `WindowRuntime& runtime`; this matches the current context-as-capability
  pattern without duplicating app/window ownership before Step 105.
- Keeping the existing `AppSetupCallback(WindowRuntime&)` field and adding an
  `AppContextSetupCallback` field preserves source compatibility for existing
  runner callers while exposing the GPUI-like setup spelling.
- Running the legacy runtime setup first and the new app-context setup second
  keeps existing behavior stable and gives callers a deterministic order when
  both callbacks are supplied.

## 2026-06-30 Back-40 Planning After Step 104

- The post-Step-128 "后40步" remains Steps 129-168, not a replacement for the
  active Step 105-128 execution queue. Starting Step 129 early would skip
  `WindowOptions`, root-view storage, view registry, child-view placeholders,
  routing ancestry, propagation, focus/scroll/layout depth, renderer text
  metadata, cursor/clipboard/IME surfaces, and the public demo rewrite.
- The next actionable implementation is already set up: continue
  `.worktrees/window-options-open-window` for Step 105 and add RED coverage for
  `WindowOptions` plus `AppContext::open_window(...)`. Recreating the Step 105
  worktree would be churn.
- The back-40 plan is useful as an execution profile now: 129-138 context and
  async, 139-148 keyed elements/widgets, 149-158 text/font/renderer maturity,
  and 159-168 Windows/Wayland closure plus the parity audit. Its entry gate
  stays Step 128 merged and Windows/WSL verified on a clean `master`.

## 2026-06-30 WindowOptions And Open Window Skeleton

- `WindowOptions` can start as a direct wrapper around `WindowDescriptor`.
  That keeps defaults (`CGPUI`, 1280x720) and platform-neutral descriptor
  mapping intact while adding the GPUI-like fluent authoring spelling needed by
  later app/window lifecycle steps.
- Step 105 should record app-opened window descriptors rather than create real
  platform windows. `run_app` still owns one `WindowRuntime`, one platform
  window, and one renderer; Step 106/159 can grow actual root-view and
  multi-window ownership without pretending this skeleton already owns extra
  native windows.
- The setup-time open-window record must remain visible after `run(...)`
  starts because `setup_context` runs before platform window creation. Clearing
  records at the start of `WindowRuntime::run(...)` would erase the only Step
  105 observable state and weaken the handoff to Step 106.

## 2026-06-30 Back-40 Planning After Step 112 GREEN

- The后 40 步 plan remains Steps 129-168 and should still wait behind the Step
  128 gate. Step 112 being GREEN in its feature worktree improves the distance
  estimate but does not open Step 129 yet because full verification, merge, and
  Steps 113-128 are still required.
- The immediate action is to finish Step 112 verification and merge from
  `.worktrees/scroll-element-binding`, then continue Step 113: wheel and
  trackpad scroll routing into bound scroll state.
- After Step 112 merges, the remaining route to the back-40 entry is 16
  implementation steps, Steps 113-128, plus post-Step-128 targeted, Windows
  full debug, and WSL Arch full debug verification on a clean `master`.

## 2026-06-30 Scroll Element Binding Merged

- `ScrollState` is intentionally an alias over `ScrollModel` at Step 112. That
  gives the public authoring API the GPUI-like name without duplicating scroll
  model storage or creating a second offset/update path before event routing
  lands in Step 113.
- `ScrollElement` binds a `ScrollState&` to one child, syncs viewport/content
  sizes during layout, and preserves child hit testing and event forwarding.
  Wheel/trackpad deltas are intentionally still Step 113 so the binding surface
  can be verified before runtime scroll routing mutates it.
- After Step 112 merged and post-merge verification passed, the active 128-step
  route is Step 113 through Step 128. The back-40 entry remains gated behind
  Step 128 and Windows/WSL full verification.

## 2026-06-30 Back-40 Planning After Step 115 Merge

- The后 40 步 plan remains Steps 129-168 and is still gated behind Step 128.
  Step 115 is now fully merged and post-merge verified, so the active
  pre-back-40 route is Steps 116-128 rather than Steps 115-128.
- From `master` at `c443592 feat: add flex alignment justification`, the
  effective distance to Step 129 is 13 implementation steps plus the
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. The distance through Step 168 is 53 implementation steps plus
  the Step 128 exit verification and four 10-step band checkpoint reviews.
- Step 116 should start from `.worktrees/flex-grow-shrink` on
  `codex/flex-grow-shrink` and should first add RED coverage for child flex
  grow/shrink factors affecting main-axis allocation while preserving default
  zero-grow behavior.

## 2026-06-30 Back-40 Planning After Step 121 Docs Closeout

- The后 40 步 plan remains Steps 129-168 and is still gated behind Step 128.
  Step 121 is now behavior-merged at `cf180f4 feat: add text paint command`
  and docs-closed at `dc010b6 docs: mark step 121 merged`, with post-merge
  targeted, Windows full debug, and WSL Arch Linux full debug verification
  already recorded.
- The active pre-back-40 route is exactly Steps 122-128 plus the
  post-Step-128 exit verification. Step 122 should continue the existing clean
  worktree `.worktrees/font-descriptor-font-size` on
  `codex/font-descriptor-font-size`; recreating that worktree would be churn.
- The后 40 步 planning target is four sequential bands: Steps 129-138 for
  context/entity/global/action/subscription/defer/timer/async/batching/
  diagnostics, Steps 139-148 for keyed reconciliation/widgets/style cascade,
  Steps 149-158 for text/font/renderer diagnostics, and Steps 159-168 for
  Win32/Wayland platform completion plus the parity audit.
- The effective distance to Step 129 is 7 implementation slices plus the
  Step 128 exit verification. The effective distance through Step 168 is 47
  implementation slices plus four band checkpoint reviews.

## 2026-07-01 Platform Cursor Application

- `CursorShape` needs to live below the UI layer once platform windows apply
  cursors directly. Moving it from `include/cgpui/ui/ui.hpp` to
  `include/cgpui/core/events.hpp` lets both `WindowRuntime` and
  `PlatformWindow` use the same platform-neutral enum without making platform
  depend on UI.
- The narrow runtime contract is to apply platform cursor changes after routed
  pointer hover recomputes `cursor_shape_`, and only when the shape changes.
  This keeps existing input snapshots intact while avoiding redundant platform
  calls for repeated pointer moves over the same cursor shape.
- Win32 can map the existing cursor shapes to system cursor resources with
  `LoadCursorW`, update the window class cursor through `SetClassLongPtrW`, and
  call `SetCursor` for immediate feedback.
- Wayland should stay a skeleton at Step 124: store the requested shape and
  issue a testable `wl_pointer.set_cursor` request when the pointer is over the
  window, but defer cursor theme loading and cursor surfaces to a later
  platform-depth step.
- The Wayland test compositor must count `set_cursor` requests, not merely
  record a boolean, because pointer enter can apply the default cursor before
  the explicit `PlatformWindow::set_cursor(text)` call.

## 2026-07-01 Runtime Update Batching

- Step 137 keeps update batching inside `WindowRuntime` rather than changing
  model observer ordering. `batch_updates(...)` increments a runtime-local
  depth counter and flushes deferred redraw requests only when the outermost
  batch exits.
- `schedule_redraw()` already had deferral hooks for event dispatch, deferred
  callbacks, timers, and task completions. Extending that single deferral path
  to `update_batch_depth_ > 0` preserves existing redraw semantics while
  allowing model/global changes inside a batch to coalesce.
- Typed global updates were previously state-only in runtime tests. Step 137
  makes `set_global(...)` and `update_global(...)` request render invalidation
  so batched global changes participate in the same redraw coalescing behavior
  as model updates.
- The public surface is deliberately narrow: `UpdateBatchCallback`,
  `WindowRuntimeContext::batch_updates(...)`, and
  `WindowRuntime::batch_updates(...)`. It does not introduce transaction
  rollback, observer reordering, or cross-thread mutation.

## 2026-07-01 Runtime Diagnostics Snapshot

- Step 138 should expose a read-only public snapshot from both
  `WindowRuntime` and `WindowRuntimeContext`, because app code should not need
  to stitch together raw runtime internals like `subscriptions_for_view(...)`,
  `invalidation_state()`, `last_render_record()`, and `frame_index`.
- Entity storage is type-erased in `WindowRuntime::entity_stores_`, so the
  narrow diagnostic route is to maintain a runtime-level `entity_count_` as
  entities are inserted/emplaced/removed. This avoids adding reflection or
  virtual diagnostics to `EntityStore<T>` during the context-side API band.
- Step 138 intentionally counts observer records separately from connected
  owned subscriptions. Permanent observers have a zero `SubscriptionId`, while
  owned subscriptions remain connected only while their callback is still
  present.
- The snapshot includes current invalidation and last render/frame state, but
  not timing, command counts, or profiling fields. Those belong to the later
  frame statistics diagnostics slice.

## 2026-07-01 Keyed Element Identity

- Step 139 keeps keyed identity parent-local and additive: `ElementKey`,
  `Element::key()`, and `ElementBuilder::key(...)` annotate elements without
  changing the existing `ElementId` allocation contract.
- `ElementTree::reconcile_children(...)` is the new batch path for keyed
  reorder/insert/remove. Existing `reconcile_child(parent, index, element)`
  remains index-based so older unkeyed tests and call sites keep their current
  behavior.
- Reused keyed nodes keep their `ElementId` and existing child subtrees while
  replacing the node's element payload. Removed direct children are pruned with
  their descendants, which gives Step 140 lifecycle hooks and Step 141 element
  state storage a clear mount/update/unmount boundary to build on.
- Builder keys are propagated to click, pointer, key, and focus wrappers so
  event/focus-decorated authored elements reconcile by the public key attached
  at the builder level, not by an inner implementation detail.
- The first Step 139 feature-worktree Windows full debug run saw the known
  transient `clipboard_test/default` failure; an immediate targeted rerun
  passed 1/1 and the Windows full debug rerun passed 29/29. No Step 139 code
  touches clipboard paths.

## 2026-07-01 Element Lifecycle Hooks

- Step 140 keeps lifecycle callbacks on the element reconciliation boundary:
  `on_mount(...)` fires when a fresh `ElementId` is allocated, `on_update(...)`
  fires when a new element payload reuses an existing `ElementId`, and
  `on_unmount(...)` fires before a node is pruned from the tree.
- Reused keyed children receive `on_update(...)` on the replacement element
  instead of unmounting the previous payload. This preserves the element-id
  continuity that Step 141 will use for per-element state storage.
- `set_root(...)` must unmount the previous root subtree before clearing
  `nodes_`; otherwise full root replacement skips lifecycle teardown for the
  old root and descendants.
- The first RED check initially appeared to pass because the RED patch was
  accidentally applied to the main worktree instead of the Step 140 feature
  worktree. The main worktree was restored to only the known untracked
  `.vscode/`, and the RED patch was reapplied to
  `.worktrees/element-lifecycle-hooks` before validating the expected missing
  API failure.

## 2026-07-01 Element State Storage

- Step 141 stores reusable-widget state on `ElementTree::Node`, keyed by
  `std::type_index` with `std::any` payloads. This makes state follow the
  stable `ElementId` rather than the replaceable `Element` payload used during
  keyed reconciliation.
- `ElementTree::state<T>(...)` soft-fails with `nullptr` for missing element
  ids, missing state, and wrong types. `state_or_init<T>(...)` creates state
  only for live nodes, while `emplace_state<T>(...)` explicitly replaces the
  state for that element/type pair.
- Removed element subtrees drop their state naturally because `remove_subtree`
  erases the owning node after lifecycle unmount. Reused keyed children retain
  their state across `reconcile_children(...)` updates and reorders.
- `WindowRuntime` and `WindowRuntimeContext` forward element-state access only
  when the runtime owns an installed `ElementTree`; legacy raw
  `set_element_root(...)` paths intentionally soft-fail until they migrate to
  tree ownership.

## 2026-07-01 Style Classes And Theme Tokens

- Step 142 keeps style vocabulary inert: `StyleClassId`, `StyleClasses`,
  `ThemeTokenId`, and `Theme` are public authoring data structures, but they do
  not affect `resolved_style(...)` or element rendering yet.
- `StyleClasses` preserves insertion order and de-duplicates repeated class
  ids. This gives Step 143 a deterministic class application order without
  introducing selectors or CSS-like matching in this slice.
- `Theme` has separate typed maps for color and spacing tokens. Missing token
  lookups soft-fail with `std::nullopt`, and replacing an existing token value
  is explicit through `set_color(...)` / `set_spacing(...)`.
- Cascade resolution, runtime theme switching, class-to-style rules, and
  inherited style behavior remain out of scope until Step 143 and later
  widget/style slices.

## 2026-07-01 Style Cascade Resolution

- Step 143 introduces a deterministic style-resolution primitive rather than a
  runtime-installed cascade. Callers pass a `StyleCascade`, local `StyleState`,
  ordered `StyleClasses`, an inline `StyleOverlay`, and state flags to
  `resolved_style(...)`.
- The merge order is local base, class rules in `StyleClasses` insertion order,
  local state overlays, then inline overlay. Missing class rules soft-fail so
  authors can share class lists across partial cascades.
- Class base styles are converted into sparse overlays by comparing against the
  default `Style`. This preserves local base fields that a class did not
  author, but it means Step 143 cannot intentionally reset a non-optional field
  back to its default value through a class base rule. A later richer rule type
  should make authored-default resets explicit if widgets need that behavior.
- `StyledElement` stores authored classes and inline styles, and the box builder
  exposes `.class_name(...)` plus `.inline_style(...)`. Normal layout and paint
  still use the local style state until a later slice supplies cascade context
  to runtime traversal.

## 2026-07-01 FocusHandle Primitive

- Step 144 deliberately keeps `FocusHandle` as a lightweight public handle over
  `ElementId`, not as a new focus graph or owner type. The existing
  keyboard-focus owner remains authoritative.
- `FocusHandle::request(...)` and `release(...)` forward through
  `WindowRuntime` / `WindowRuntimeContext`, so widget code can carry handles
  without reaching into runtime internals.
- `contains(...)` and `focused(...)` are snapshot-friendly queries over
  `ViewInputState`, `WindowRuntime`, and `WindowRuntimeContext`. This keeps
  focus inspection cheap for reusable widgets and tests.
- The new public `WindowRuntime::input_state()` snapshot mirrors the context
  access path and avoids duplicating focus/hover/pointer-capture state assembly.
- Step 145 should build the button primitive from these public pieces:
  focusable/disabled/click/action behavior, existing style-state overlays, and
  element builders. It should not introduce private runtime hooks unless a
  missing general primitive first becomes explicit in RED coverage.

## 2026-07-01 Button Widget Primitive

- Step 145 keeps the button primitive as a public reusable element/widget
  surface, not a new runtime event special case. `ButtonElement` is focusable,
  honors `enabled()`, runs an optional local click handler first, and then
  dispatches its action name through the general `ElementEventContext`
  `dispatch_action` callback wired to `WindowRuntime::dispatch_action(...)`.
- Button layout intentionally follows the same child-plus-padding/margin
  semantics as `StyledElement`: when a child is present, the child layout size
  becomes the content size. `preferred_size` remains the no-child base content
  size path rather than a forced outer-box override.
- Button paint now shares the styled-box background, border, border-radius, and
  overflow-clip command generation path with `StyledElement`, then paints its
  child afterward. This makes the button a visible widget primitive instead of
  only an event/action wrapper.
- The button stores hover/focus/disabled `StyleState` metadata and exposes it
  for authoring/tests, but normal paint still uses the local base style. A
  later slice that installs cascade/state flags into runtime traversal should
  teach widgets to resolve hovered/focused/disabled styles at layout/paint time.
- Step 146 can build `label(...)` as a thin text-widget helper over the
  existing `TextElement`, text style fields, and text paint command metadata;
  it should include header cleanliness coverage for the public widget helper.

## 2026-07-01 Text Shaping Run Abstraction

- Step 150 intentionally introduces deterministic fallback shaping, not full
  HarfBuzz/text-complex shaping. Each leading UTF-8 byte plus its continuation
  bytes becomes one `TextGlyphRun`, with byte offsets/lengths preserved for
  later glyph-atlas and caret/selection work.
- `shape_text(...)` stores the original text bytes, font descriptor, font size,
  total byte length, glyph advances, total advance, and line height. This gives
  Step 151 a stable bridge from UI text metadata to renderer-facing glyph cache
  keys without depending on platform font discovery yet.
- `TextElement` and `LabelElement` now derive layout width/height from
  `TextShapeRun::total_advance` and `line_height`. This fixes the old
  byte-count metric for UTF-8 text while preserving the existing ASCII fallback
  behavior of `font_size * 0.5F` per glyph.
- The public header-cleanliness test now directly exercises `TextShapeRun` and
  `shape_text(...)`; future text/glyph APIs should keep that coverage current
  so author-facing headers remain standalone.

## 2026-07-01 Glyph Atlas Cache Interface

- Step 151 keeps glyph caching renderer-facing but allocation-free:
  `GlyphCache` records lookup hits/misses and stores `GlyphAtlasEntry`
  metadata, but it does not allocate Vulkan images, upload glyph pixels, or
  depend on platform font discovery yet.
- `GlyphAtlasKey` is derived from the fallback shaping run: font family, font
  size, glyph index, byte offset, and byte length. This makes text paint
  metadata deterministic for ASCII and UTF-8 fallback glyph runs while leaving
  room for a later real shaper to provide stable glyph ids.
- `PaintList::fill_text(...)` now shapes text once and emits `TextPaint::glyphs`
  with per-glyph origin and advance metadata. Step 152 should consume this
  metadata in the Vulkan text path instead of reshaping raw text in the
  renderer.
- Header-cleanliness coverage now exercises the renderer-facing cache types and
  UI text glyph metadata. Future text-rendering slices should keep the public
  surface split: `ui/text.hpp` owns shaping/glyph paint metadata, while
  `renderer/renderer.hpp` owns atlas/cache lookup records.

## 2026-07-01 Vulkan Text Draw Path

- Step 152 makes text commands reach the renderer frame for the first time:
  `render_view(...)` now forwards `PaintCommandKind::text` as `TextDraw`
  instead of skipping it, while text selection and caret commands still remain
  metadata-only and skipped by the renderer bridge.
- `RenderFrame::draw_text(...)` intentionally has a default no-op body so
  existing fake frames and the deferred Metal skeleton remain source-compatible.
  Vulkan overrides it and stores `TextDraw` commands for frame presentation.
- The Vulkan path consumes `TextDraw::glyphs` through a persistent
  `GlyphCache` before recording the swapchain command buffers. This proves the
  renderer uses cache metadata, but it still does not rasterize glyph pixels,
  allocate atlas textures, bind pipelines, or emit textured quads.
- `vulkan_consume_text_draw(...)` stores deterministic atlas placeholder bounds
  from each glyph origin, advance, and font size. Step 153 can build opacity and
  transform command metadata on top of this command stream without depending on
  real font upload yet.

## 2026-07-01 Opacity and Transform Paint Metadata

- Step 153 keeps opacity and transform as deterministic command metadata, not a
  GPU pipeline or animation feature. `AffineTransform` lives in core geometry so
  style, UI paint, renderer commands, and future Metal/Vulkan paths can share
  the same public value type.
- `PaintList` owns metadata scoping: nested scopes multiply opacity and compose
  transforms parent-to-child. This keeps `StyledElement`, `ButtonElement`, text
  widgets, and scroll containers from needing renderer-specific knowledge while
  still producing stable command metadata.
- `render_view(...)` now forwards command metadata into both `SolidRect` and
  `TextDraw`. Text selection and caret remain metadata-only and skipped by the
  renderer bridge, matching the Step 152 behavior.
- Step 154 can use `PaintMetadata` directly when defining batching keys by clip,
  opacity, transform, and primitive kind; it should remain diagnostic grouping,
  not GPU optimization.

## 2026-07-02 Window Lifecycle Events

- Step 160 treats activation, minimize, restore, and close as lifecycle
  observability events on the existing root route. They produce
  `EventDispatchRecord` values and after-event callbacks, but they do not force
  view fallback handling the way input events do.
- `WindowFocused` intentionally stays on the existing view-dispatch path so
  prior focus behavior and view-level focus handling remain source-compatible.
- Close requests now record the close lifecycle event before invoking the close
  callback and `PlatformApplication::quit()`. This lets the close callback see
  `context.last_event_dispatch` for `window_close_requested` while preserving
  the existing quit semantics.
- Minimize/restore update the lightweight `ViewInputState::focused` snapshot
  only enough for lifecycle callbacks to observe focus loss. They do not yet
  model platform window state, visibility, occlusion, or renderer suspension;
  those should remain separate future platform/runtime decisions if needed.

## 2026-07-02 Win32 IME Placement

- Step 161 keeps UI-level `ImeCandidateRect` tied to `ElementId`, but the
  platform layer receives only `ImeTextInputPlacement` with a screen-space
  `Rect` and `byte_offset`. This keeps platform IME handling independent of
  element-tree ownership and focused-text lookup details.
- Runtime placement sync runs after focused text geometry can change: event
  dispatch, layout/redraw, focus request/release, and rendered-tree updates all
  converge through a cached platform placement update path so redundant native
  calls are avoided.
- Win32 stores the latest placement in `WindowState` and applies it to both IMM
  composition and candidate windows. Missing `HIMC` is treated as a soft skip,
  which keeps tests and non-IME scenarios deterministic.
- Wayland currently preserves the same placement state without protocol
  side effects. Step 162 should consume this state in an isolated text-input
  skeleton and report graceful unsupported behavior before attempting a full
  text-input-v3 implementation.

## 2026-07-02 Wayland IME Skeleton

- Step 162 makes IME capability explicit through `ImeTextInputSupport` on
  `WindowState`. The default remains `unsupported`; Win32 marks the existing
  IMM placement path as `available`, while Wayland reports `unsupported` until
  a text-input protocol global is actually bound.
- The Wayland `WaylandTextInput` skeleton is intentionally state-only: it
  accepts `ImeTextInputPlacement`, stores the latest placement, and exposes a
  support snapshot without sending protocol requests. This preserves focused
  text geometry through the platform boundary without pretending that
  text-input-v3 is implemented.
- `WaylandWindow::set_ime_text_input_placement(...)` now delegates to the
  skeleton and mirrors both support and placement into `WindowState`, giving
  runtime/tests a single platform-neutral observation point for supported and
  graceful-unsupported paths.
- Step 163 can now focus on Win32 drag/drop event shapes without reworking IME
  geometry. A future full Wayland text-input slice should replace only the
  internals of `WaylandTextInput` with protocol binding, enter/leave, content
  type, surrounding text, and cursor-rect requests.

## 2026-07-02 Win32 Drag-and-Drop Skeleton

- Step 163 defines platform-neutral drag/drop event shapes in the core event
  surface: `DragDropPayloadKind`, `DragDropPayload`, `DragEntered`,
  `DragUpdated`, `DragDropped`, and `DragExited`. Payloads are intentionally
  narrow for now: either text, files, or empty/none.
- Runtime routing treats drag/drop events as pointer-positioned events for
  hit-testing and `input.pointer_position` updates, but it does not update
  hover cursor state. This keeps DnD routing observable without merging drag
  state into the existing pointer hover state machine.
- Win32 currently uses deterministic `RegisterWindowMessageW` test hooks to
  translate UTF-16 text and file paths into public drag/drop events. This is a
  skeleton and does not yet claim real `IDropTarget`, shell/OLE data-object
  parsing, drag effects, or file-manager integration.
- Step 164 should reuse the public event shapes for Wayland data-device work
  and add Wayland-specific graceful no-data behavior without changing the
  Win32 test hook surface.

## 2026-07-02 Wayland Data-Device Drag-and-Drop Skeleton

- Step 164 reuses the existing public drag/drop event shapes rather than
  adding Wayland-specific public API: `DragEntered`, `DragUpdated`,
  `DragDropped`, and `DragExited` are emitted from Wayland data-device
  notifications.
- Production Wayland now binds `wl_data_device_manager`, creates a seat data
  device, tracks the active drag target surface through the existing
  window-lookup path, and maps data-device enter/motion/drop/leave into
  platform events. `DragExited` uses the last known drag position because
  Wayland leave has no coordinates.
- Payload handling intentionally soft-fails to `DragDropPayloadKind::none`.
  The slice does not implement MIME negotiation, `wl_data_offer_receive`, pipe
  reading, text extraction, URI-list parsing, file-manager integration, or drag
  effects.
- The Wayland test compositor now exposes a deterministic
  `wl_data_device_manager`/`wl_data_device` path and request helpers for
  drag-enter, drag-motion, drop, and leave. This keeps Step 164 testable
  without depending on an external compositor or desktop file manager.
- Step 165 should bridge runtime deferred callbacks, timers, and async
  completions to platform wakeups without changing the Step 164 DnD payload
  boundary.

## 2026-07-02 Platform Event Loop Wakeup

- Step 165 adds a platform-neutral wakeup event rather than overloading redraw:
  `WindowWakeupRequested` asks the runtime to drain queued non-render work,
  while `WindowRedrawRequested` remains the render path.
- Runtime queue producers now request a platform wakeup when the window is
  live: `defer(...)`, `schedule_timer(...)`,
  `schedule_repeating_timer(...)`, and `complete_task(...)`. Existing
  deterministic test hooks such as `advance_time(...)` remain available and
  keep their direct timer-drain behavior.
- Wakeup drain order is deterministic: task completions first, due timers
  second, deferred callbacks third. This prevents async completions from
  racing timer/defer callbacks inside the same wakeup turn.
- Wakeup handling is an outer drain batch. `handling_wakeup_` suppresses
  intermediate deferred-redraw flushes from task/timer/defer drains so one
  redraw is requested after all queued wakeup work drains. This fixed the
  initial Step 165 GREEN failure where a task completion flushed redraw before
  timers and deferred callbacks ran.
- Win32 implements the wakeup bridge by posting a private thread message to the
  event-loop thread and dispatching `WindowWakeupRequested` to tracked windows.
  Wayland implements the bridge with a nonblocking close-on-exec pipe polled
  alongside the display fd. The base/empty platform implementation stays a
  no-op for source compatibility.
- Step 166 can now focus on platform-neutral accessibility snapshots without
  needing to revisit runtime wakeup plumbing.

## 2026-07-02 Windows/Linux Demo Smoke Flows

- Step 167 keeps demo smoke coverage deterministic and bounded through
  environment variables rather than introducing broad desktop UI automation.
  `CGPUI_DEMO_SMOKE_FLOW` runs inside the existing hello-window runtime
  callbacks and exits by requesting close on the second frame.
- The full-flow smoke deliberately uses the public demo/runtime surface:
  `cgpui::MemoryClipboard`, a `cgpui::TextInput` event object, focused text
  mutation, runtime clipboard paste/copy helpers, `context.request_render()`,
  and `context.window.request_close()`. It does not add private platform hooks
  or a separate test-only demo binary.
- Existing demo smoke controls remain intact:
  `CGPUI_EXIT_AFTER_FIRST_FRAME`, `CGPUI_RESIZE_AFTER_FIRST_FRAME`,
  `CGPUI_CLOSE_AFTER_FIRST_FRAME`, `CGPUI_DEMO_INJECT_TEXT`, and
  `CGPUI_DEMO_PAINT_SNAPSHOT_SMOKE` still cover their narrower paths.
- The `hello_window` xmake target now has one additional platform smoke test on
  each active target: `windows_demo_smoke_flow` and `linux_demo_smoke_flow`.
  This raises full-suite counts to Windows 30/30 and WSL Arch Linux 27/27.

## 2026-07-02 Glyph Atlas Upload Records

- Step 170 keeps glyph upload preparation CPU-side and deterministic:
  `GlyphCache::allocate(...)` consumes `RasterizedGlyph`, creates fixed-size
  atlas pages as needed, row-packs glyph bitmaps, and records alpha upload
  bytes without creating Vulkan texture objects yet.
- `GlyphAtlasEntry` now carries `page_index`, so renderer-facing cache entries
  can distinguish atlas pages while preserving existing lookup and store
  behavior for manually supplied entries.
- Repeat allocation of an existing glyph is a cache hit for atlas storage:
  it returns the existing atlas bounds with `created == false` and does not
  append duplicate upload records.
- `vulkan_consume_text_draw(...)` now runs missing glyph metadata through
  `rasterize_fallback_glyph(...)` and atlas allocation, moving the text path
  beyond placeholder metadata toward uploadable bitmap data.
- Step 171 can build textured glyph quad records from these atlas entries and
  upload records; actual GPU texture creation remains a later renderer
  integration boundary.

## 2026-07-02 Textured Glyph Quad Records

- Step 171 adds a renderer-facing `TexturedGlyphQuad` data record rather than
  creating Vulkan image/sampler objects. Each quad carries the glyph key,
  atlas page index, device-space bounds, atlas pixel bounds, normalized atlas
  UV bounds, color, optional clip rect, and paint metadata.
- `vulkan_build_textured_glyph_quads(...)` is the bridge from `TextDraw` to
  render-preparable glyph geometry. It performs the existing glyph-cache lookup
  and fallback raster allocation path, then derives one stable quad per
  `TextGlyphPaint`.
- `vulkan_consume_text_draw(...)` now delegates to the textured-quad builder,
  so text preparation uses one path for lookup, rasterization, atlas
  allocation, and quad generation.
- Header-only/UI tests should instantiate `TexturedGlyphQuad` records directly
  and avoid calling Vulkan implementation functions unless their xmake target
  links `cgpui_renderer_vulkan`.
- Step 172 can add render-report counters over cache hits, rasterized glyphs,
  upload records, and emitted quads without yet requiring real GPU texture
  uploads.

## 2026-07-02 Vulkan Text Render Report Counters

- Step 172 adds `RendererTextRenderReport` inside `RendererCommandReport` so
  text commands can be reported as glyph-backed preparation instead of only
  generic metadata-backed command batches.
- The draw-data overload of `vulkan_build_renderer_command_report(...)`
  consumes solid rects, text draws, and a `GlyphCache`, then reports both the
  ordinary command batches and text-depth counters.
- Text-depth counters are delta-based for each report build: cache hits are
  counted from new lookup records, rasterized glyphs and upload records are
  counted from new upload records, and textured glyph quads are counted from
  the emitted quad vector.
- The parity audit now states the current renderer text status precisely:
  CPU fallback raster data, atlas allocation/upload records, textured glyph
  quads, and report counters exist; real Vulkan atlas textures, GPU upload,
  shader sampling, subpixel positioning, and full font fallback shaping remain
  incomplete.
- Step 173 can now pivot away from Vulkan text metadata and start the Wayland
  clipboard MIME payload extraction path.

## 2026-07-02 Wayland Clipboard MIME Payload Extraction

- Step 173 keeps `create_platform_clipboard()` conservative while adding an
  opt-in Wayland clipboard connection for tests and future integration:
  `WaylandClipboardOptions::connect_to_display` connects to the active
  Wayland display, binds `wl_data_device_manager`, `wl_seat`, and
  `wl_data_device`, and stores the current selection offer behind a PIMPL so
  public headers do not include Wayland protocol headers.
- Text MIME selection is deterministic: `text/plain;charset=utf-8` is chosen
  before `text/plain`, unsupported MIME types are ignored, and payload bytes
  are read through a pipe passed to `wl_data_offer_receive`.
- The Wayland test compositor now has a deterministic clipboard selection
  path: tests can set MIME/payload pairs, the compositor sends a
  `wl_data_offer`, writes the selected payload to the received fd, and records
  the requested MIME type for assertions.
- This is not full Wayland clipboard parity yet. Clipboard ownership/write
  offers, default platform-factory connection behavior, non-text MIME formats,
  and production desktop edge cases remain future work.
- Step 174 can reuse the same offer/MIME/payload direction for Wayland
  drag/drop, then add text and `text/uri-list` parsing through the drag
  data-device path.

## 2026-07-02 Wayland Drag/Drop MIME Payload Extraction

- Step 174 extends the Wayland data-device path from event routing to payload
  extraction. `WaylandDataDevice` now listens to `wl_data_offer.offer`, stores
  offered MIME types with the active drag offer, and reads payload bytes with
  `wl_data_offer_receive` through deterministic pipes.
- Drag payload preference is text first: `text/plain;charset=utf-8` then
  `text/plain`, followed by `text/uri-list` for files. Unsupported or empty
  offers continue to produce `DragDropPayloadKind::none`.
- URI-list parsing intentionally handles the practical local-file subset:
  comments and blank lines are ignored, `file:///...` and
  `file://localhost/...` are accepted, and percent escapes are decoded before
  exposing public file paths.
- The Wayland test compositor now uses a neutral `WaylandMimePayload` test
  type, retains the old clipboard payload alias, and can send drag offers as
  well as clipboard selection offers from the same deterministic data-offer
  receive path.
- Remaining drag/drop depth includes accept/finish/action negotiation,
  non-local URI policy, richer MIME formats, and production desktop-file
  manager edge cases.

## 2026-07-02 Wayland Text-Input State Machine

- Step 175 moves Wayland IME beyond placement-only storage. The Linux backend
  now has a minimal handwritten `zwp_text_input_v3` client binding and the
  test compositor exposes the matching server-side protocol surface for
  deterministic tests.
- `WaylandTextInputState` is protocol-independent window state: it tracks
  protocol availability, placement, enter/leave state, surrounding-text cursor
  records, content-type records, preedit text, and committed text.
- IME placement is now converted into protocol requests when text-input v3 is
  available: enable/disable, empty surrounding text with cursor/anchor byte
  offset, default content type, cursor rectangle, and protocol commit.
- Deterministic compositor enter/preedit/commit/leave events now route into
  public `ImeComposition` update/commit events through the platform callback,
  while the test compositor records the client-side surrounding/cursor/content
  commit state for assertions.
- This is still partial Wayland IME parity. Production input-method behavior,
  delete-surrounding editing, richer content hints/purposes, serial policy, and
  compositor-specific edge cases remain future work.

## 2026-07-02 Native Additional Window Creation Scaffold

- Step 178 turns app-opened child-window records from metadata-only registry
  entries into native `PlatformWindow` creation attempts owned by
  `WindowRuntime`.
- Additional windows are activated through the existing
  `PlatformApplication::create_window(...)` boundary, so the slice does not add
  Win32/Wayland-specific public APIs or fork the GPUI-like authoring surface.
- `WindowRuntimeRecord::native_window_error` preserves graceful failure
  semantics for child windows: a backend/test double can fail the child native
  window creation without failing the root `run_app(...)` path.
- Child platform-window callbacks currently update descriptor size on resize
  and mark the child record inactive on close request; shutdown clears child
  window/renderer pointers and releases the owned native-window vector.
- This remains partial multi-window parity. Child windows still do not own
  independent renderers, independent render loops, full event routing, or
  production activation/focus/lifecycle behavior.

## 2026-07-02 Steps 179-218 Production-Depth Track

- The next 40-step track should stay Windows/Linux first rather than starting
  macOS parity. The highest-value ordering is Vulkan text/rendering depth,
  text/font/editing depth, Win32/Wayland production adapter depth, then
  accessibility, multi-window, theme, asset, animation, and async depth.
- Step 179 should start with renderer-facing glyph atlas image descriptors and
  upload batches. This is the smallest next move from the current text path:
  glyph bitmap, atlas allocation, upload records, textured quads, and text
  render reports already exist, but there is no atlas image/upload planning
  surface yet.
- The 179-218 plan intentionally keeps real Vulkan image allocation, production
  UIA/AT-SPI providers, full child-window event loops, and macOS/Cocoa + Metal
  outside Step 179 so the next RED/GREEN slice stays small and verifiable.

## 2026-07-02 Vulkan Glyph Atlas Image Upload Planning

- Step 179 adds renderer-facing glyph atlas image descriptors and upload
  batches as the bridge between CPU-side `GlyphUploadRecord`s and later Vulkan
  texture resources.
- `vulkan_plan_glyph_atlas_uploads(...)` groups upload records by atlas page,
  keeps deterministic page image metadata, appends alpha payload bytes into a
  per-page upload buffer, and records each upload region's byte offset and
  size.
- This remains a planning slice: it does not allocate Vulkan images, create
  image views, bind device memory, issue transfer commands, or sample atlas
  textures. Step 180 should add texture resource lifetime records on top of
  these batches.

## 2026-07-02 Vulkan Glyph Atlas Texture Resource State

- Step 180 adds a deterministic atlas texture resource state object keyed by
  atlas page index. It turns Step 179 upload batches into create/reuse/drop
  diagnostics with stable generations.
- Repeated updates with the same page reuse the existing record and preserve
  its generation; updates without a previously live page emit a dropped record
  and remove it from the live resource span.
- This is still a resource-lifetime skeleton. It does not create real
  `VkImage`, `VkImageView`, device memory, staging buffers, descriptor sets, or
  transfer commands. Step 181 should build dirty-range upload tracking on top
  of this page-level resource state.

## 2026-07-02 Vulkan Glyph Atlas Dirty Upload Ranges

- Step 181 adds deterministic dirty upload ranges over Step 179 upload batches
  and Step 180 texture resource state. The range planner uses each live page's
  uploaded count to skip already-planned glyph regions.
- Dirty ranges are coalesced per atlas page into byte spans over the upload
  batch alpha buffer. Repeating the same batch emits no ranges; adding one
  glyph emits a range starting at that glyph's upload index and byte offset.
- This remains CPU-side upload planning. It does not allocate staging buffers,
  record Vulkan transfer commands, transition image layouts, or update
  descriptor sets.

## 2026-07-02 Vulkan Text Sampler Pipeline Readiness Report

- Step 182 adds a deterministic `TextSamplerPipelineDescriptor` to the renderer
  public surface and wires it into `RendererTextRenderReport`.
- Text render reports now distinguish glyph quad preparation from sampler
  pipeline readiness: non-empty textured glyph draws increment the sampler
  descriptor count and pending draw count while the descriptor reports shader
  modules, descriptor set layout, pipeline layout, and graphics pipeline as not
  ready.
- This is still a readiness-report slice. It does not create shader modules,
  descriptor set layouts, pipeline layouts, graphics pipelines, descriptor
  sets, or sampled atlas draw calls.

## 2026-07-02 Vulkan Rounded-Rect Tessellation Records

- Step 183 adds renderer-facing rounded-rect draw and tessellation records so
  rounded rectangles are no longer only unsupported diagnostics or solid-rect
  fallbacks.
- `render_view(...)` now forwards rounded-rect paint commands through
  `RenderFrame::draw_rounded_rect(...)`, preserving bounds, radius, color,
  clip, opacity, and transform metadata for Vulkan reporting.
- The Vulkan helper path can build deterministic rounded-rect tessellation
  records alongside solid rectangles and text draws, and frame statistics now
  count rounded-rect commands separately.
- This is still a geometry/reporting slice. It does not create a real Vulkan
  rounded-rect pipeline, shader path, anti-aliased edge generation, or GPU draw
  submission.

## 2026-07-02 Vulkan Text Selection And Caret Geometry Records

- Step 184 adds renderer-facing text selection and caret draw/geometry records
  so those text paint commands are no longer skipped or reported as unsupported.
- `render_view(...)` now forwards text selection and caret paint commands
  through `RenderFrame::draw_text_selection(...)` and
  `RenderFrame::draw_text_caret(...)`, preserving bounds, color, clip, opacity,
  and transform metadata for Vulkan reporting.
- Renderer command reports and frame statistics now count selection and caret
  commands separately from text glyph draws, matching the existing rounded-rect
  and text command depth.
- This is still a geometry/reporting slice. It does not create real Vulkan
  selection/caret draw pipelines, shader paths, or GPU submissions.

## 2026-07-02 Renderer Clip Stack Metadata

- Step 185 adds a bounded `RendererClipStackRecord` that preserves nested clip
  stack metadata while keeping the existing per-command `clip_rect` field as
  the current innermost clip.
- `PaintList` now records the full active clip stack on paint commands, and
  `render_view(...)` forwards that stack through solid rect, rounded rect,
  text, text selection, and caret draw records.
- Vulkan renderer command reports now include clip-stack counts, maximum stack
  depth, and batch keys that distinguish commands with the same current clip
  but different outer clip ancestors.
- This remains metadata/reporting work. It does not implement real Vulkan
  scissor-stack command emission or GPU clipping beyond the existing solid
  rectangle clip handling.

## 2026-07-03 Renderer Composition Stack Reports

- Step 186 adds a bounded `RendererCompositionStackRecord` that preserves
  nested opacity/transform metadata while keeping the existing per-command
  `metadata` field as the current composed metadata.
- `PaintList` now records the active metadata stack on paint commands, and
  `render_view(...)` forwards that stack through solid rect, rounded rect,
  text, text selection, and caret draw records.
- Vulkan renderer reports now count composition-stack records, track maximum
  stack depth, propagate the stack into rounded/text selection/caret geometry,
  and include the stack in batch keys so otherwise identical commands with
  different ancestors remain distinguishable.
- This remains diagnostics/reporting work. It does not implement real Vulkan
  transform stack emission, opacity compositing, or GPU pipeline state changes.

## 2026-07-03 Renderer Batch Submission Plan Records

- Step 187 adds renderer submission-plan records on `RendererCommandReport`
  that summarize command batches into deterministic submission groups.
- Submission plan keys preserve primitive kind, current clip, clip stack, and
  optional glyph atlas page index. Text submissions are split by atlas page and
  carry the current text sampler pipeline descriptor, while non-text
  submissions carry primitive-specific pipeline descriptors.
- Reports now expose submission record count, command count, and glyph quad
  count so later frame snapshots can summarize planned GPU work without
  re-walking raw command batches.
- This remains a deterministic planning/reporting slice. It does not allocate
  Vulkan command buffers, descriptor sets, render passes, pipeline objects, or
  submit work to a GPU queue.

## 2026-07-03 Renderer Frame Snapshot Report

- Step 188 adds `RendererFrameReport`, a frame-level aggregation over
  `RendererCommandReport` that preserves the underlying command report while
  summarizing supported/unsupported primitives, renderer batches, submission
  plans, glyph upload records, textured glyph quads, and renderer gaps.
- Frame gaps currently cover unsupported commands, pending text sampler
  pipeline readiness, metadata-only text draws, and missing submission plans.
  The gap list gives later diagnostics a single source for "what is still not
  real GPU work" in a frame.
- Vulkan exposes `vulkan_build_renderer_frame_report(...)` overloads that build
  the existing command report and wrap it into the new frame report. The parity
  audit now calls out frame-level text/render reports and planned submissions.
- This remains a reporting slice. It does not create real Vulkan command
  buffers, submit GPU work, or replace the existing deterministic report
  building path.

## 2026-07-03 Font Fallback Chain Resolution

- Step 189 adds a public `FontFallbackChain` over `FontDatabase` faces so text
  code can inspect ordered fallback candidates instead of only receiving the
  single face from `FontDatabase::resolve(...)`.
- `FontDatabase` now stores deterministic generic fallback families and
  resolves chains in requested-family, generic-fallback, first-available order
  with duplicate face pointers removed.
- Empty-family chain resolution intentionally preserves the existing
  `resolve({})` behavior by returning only the first registered face.
- This is still deterministic fallback planning. It does not perform
  DirectWrite/fontconfig discovery, per-codepoint coverage checks, shaping
  fallback splits, or platform font enumeration.

## 2026-07-03 Platform Font Discovery Records

- Step 190 adds `PlatformApplication::discover_font_records()` as the
  platform-facing record hook and keeps `discover_fonts()` as the database
  adapter over those records.
- Win32 now exposes a deterministic platform `FontFaceDescriptor` for
  `Segoe UI`; Wayland/Linux exposes a deterministic `sans-serif` fontconfig
  record. Both records keep `FontSource::platform`, family, postscript/name,
  and path-like metadata visible to tests without enumerating system fonts.
- Text helpers now include `font_database_from_discovered_faces(...)`, with
  `discover_test_fonts(...)` delegating to the same builder so platform and
  test records preserve source/path/name metadata consistently.
- This is still conservative discovery plumbing. It is not real DirectWrite or
  fontconfig enumeration, does not probe files, and does not verify glyph
  coverage.

## 2026-07-03 Grapheme-Aware Cursor Movement

- Step 191 moves `TextModel` cursor movement, selection extension, backspace,
  and delete from raw UTF-8 codepoint boundaries to deterministic grapheme
  boundaries.
- The helper intentionally covers the current editing-depth skeleton cases:
  ASCII, combining-mark clusters, variation selectors, regional indicator
  pairs, and emoji ZWJ sequences. This is enough to prevent common cursor and
  delete splits in those clusters without claiming full Unicode UAX #29
  segmentation.
- The implementation stays local to `TextModel` and does not introduce a
  shaping dependency. Later rich text work can replace or extend the boundary
  helper when full shaping and per-codepoint fallback coverage arrive.
- Step 192 should build word movement and word-selection actions on top of
  these grapheme-safe cursor primitives rather than reverting to byte or raw
  codepoint stepping.

## 2026-07-03 Text Word Navigation Actions

- Step 192 adds `TextEditAction` variants and public `TextModel` helpers for
  previous/next word movement plus previous/next word selection extension.
- Word movement reuses the Step 191 grapheme-boundary helpers. This keeps word
  navigation from splitting combining marks, regional indicator pairs, or ZWJ
  skeleton clusters while adding word-level jumps.
- The current word separator class is intentionally deterministic: ASCII
  whitespace, C1 next-line, no-break and Ogham spaces, Unicode U+2000 space
  range, line/paragraph separators, narrow no-break, medium mathematical, and
  ideographic space. Non-space punctuation remains part of the surrounding
  non-space run in this skeleton.
- Step 193 can add undo/redo over the expanded edit-action surface without
  revisiting word-boundary semantics.

## 2026-07-03 Text Undo And Redo Stack

- Step 193 adds undo/redo as `TextModel` behavior and `TextEditAction`
  variants, not as a separate widget-private editor layer. This keeps text
  input widgets, runtime edit actions, and direct text-model users on one edit
  history surface.
- History records snapshot text, cursor, selection anchor, and selection head
  before and after mutating edits. Insert, selection replacement, backspace,
  forward delete, and IME composition commit are recorded through the existing
  mutation paths.
- Undo restores the before snapshot and moves the record to redo; redo restores
  the after snapshot and returns the same record to the bounded undo stack.
  New edits clear redo, matching linear edit-history semantics.
- Restoring a history snapshot clears composition state. This avoids reviving a
  stale preedit session after undo/redo and keeps committed composition text as
  an ordinary edit record for the next IME slice.
- The history cap is currently a deterministic 100 undo records. This is a
  bounded model-level stack, not persistent document history, grouped typing,
  collaborative editing, or platform undo-manager integration.

## 2026-07-03 IME Delete-Surrounding Text Action

- Step 194 adds `ImeDeleteSurroundingText` as a platform event and
  `EventKind::ime_delete_surrounding_text` as the public runtime route kind, so
  IME delete-surrounding edits travel through the same keyboard-focused event
  path as preedit and commit.
- `TextModel::delete_surrounding_text(...)` interprets before/after lengths as
  byte lengths around the current cursor and clamps deletion to UTF-8 codepoint
  boundaries before recording the edit in the existing undo stack. This keeps
  Wayland text-input v3 byte-oriented semantics deterministic without claiming
  full grapheme-aware surrounding-text deletion yet.
- Runtime application is intentionally focused-text-model scoped: the event is
  still dispatched to the focused route, but model mutation only happens when
  the keyboard focus has an element owner with a bound or installed text model.
- The Wayland backend stores pending delete-surrounding protocol events until
  `done`, then emits them between pending preedit and commit events with the
  current keyboard modifiers. The test compositor mirrors that protocol shape
  by sending `delete_surrounding_text` followed by `done`.
- This slice does not implement Win32 TSF delete-surrounding integration,
  grapheme-length conversion, grouped IME history, or rich multiline
  surrounding-text context. Step 195 should build multiline navigation on top
  of the now-shared text mutation/history surface.

## 2026-07-03 Multiline Text Model And Line Navigation

- Step 195 adds deterministic hard-line helpers to `TextModel`:
  `line_count()`, `line_index_at(...)`, `line_start_offset(...)`, and
  `line_end_offset(...)`. Lines are currently delimited by LF bytes, and line
  ends exclude the newline byte.
- New `TextEditAction` variants cover line start/end movement, previous/next
  line movement, and matching selection extension. Vertical movement preserves
  the current byte column and clamps to the destination line end when the target
  line is shorter.
- This is intentionally model-level editing depth. It does not implement soft
  wraps, bidi visual lines, grapheme column accounting, pixel hit testing,
  paragraph layout, or a persistent preferred visual column. Step 196 can add
  measurement caching without treating these hard-line helpers as full text
  layout.

## 2026-07-03 Text Measurement Cache

- Step 196 adds deterministic text measurement primitives in `ui/text.hpp`:
  `measure_text(...)`, `TextMeasurementKey`, `TextMeasurement`,
  `TextMeasurementResult`, and `TextMeasurementCache`.
- Cache keys cover the current deterministic measurement tuple: text content,
  font descriptor, font size, and normalized DPI scale. Hits and misses are
  observable through `cache_hit`, `entry_count()`, `lookup_count()`,
  `hit_count()`, and `miss_count()`.
- `PaintList` and `render_view` now accept an optional measurement cache so
  renderer-facing tests can exercise repeated text paint measurement reuse
  without changing the default no-cache render path.
- This remains a deterministic fallback measurement cache over the existing
  `shape_text(...)` skeleton. It does not add platform shaping, glyph coverage
  fallback splits, soft wrapping, paragraph layout, cache eviction, or
  cross-frame runtime ownership yet. Step 197 can build pointer-selection
  geometry against these reusable measured glyph positions.

## 2026-07-03 Structural Optimization Task 1 Private Header Surgery

- The first aggressive optimization pass moves the largest private declaration
  clusters out of old internal aggregate headers. The remaining aggregates are
  intentionally compatibility entry points, not ownership locations:
  `wayland_application_internal.hpp` is 2 lines,
  `wayland_window_internal.hpp` is 94 lines, and `vulkan_internal.hpp` is 5
  lines.
- Because C++ class declarations are single continuous declarations, the
  Wayland application/window splits use class-body declaration slices for
  private member groups. The actual ownership boundaries are now visible in
  focused headers: application registry/input/cursor, registered window,
  window configure lifecycle, Vulkan platform helpers, Vulkan state, Vulkan
  swapchain resources, and Vulkan device queue search types.
- Task 1 stayed structure-only. No runtime behavior was intended to change.
  Windows focused verification passed 6/6 available targets, WSL Arch Linux
  focused verification passed 5/5 available targets, and `git diff --check`
  exited 0 with only existing CRLF normalization warnings.
- Task 2 should now attack the remaining long bridge implementation files:
  Win32 window proc dispatch, Wayland application window registry/creation,
  Vulkan swapchain creation, and Vulkan report submission.

## 2026-07-03 Structural Optimization Task 2 Bridge Implementation Split

- Remaining long bridge files can be split without changing public behavior by
  introducing focused helper implementation files and keeping the old files as
  dispatch-only compatibility points.
- Win32 `win32_window_proc.cpp` is now a 33-line dispatcher. Test drag/drop,
  lifecycle/paint/destroy, pointer, and keyboard/text messages live in their
  own `win32_window_proc_*` files.
- Wayland `wayland_application_windows.cpp` is now a 5-line placeholder entry.
  Registry lookup/global handling lives in
  `wayland_application_window_registry.cpp`; window creation lives in
  `wayland_application_window_creation.cpp`; cursor and IME placement setters
  moved back to cursor/input owners.
- Vulkan `vulkan_swapchain_create.cpp` now delegates surface querying and
  create-info construction to focused helpers. The create-info helper must keep
  `VkSurfaceTransformFlagBitsKHR pre_transform` in the plan so returned
  `VkSwapchainCreateInfoKHR` does not depend on discarded surface-capability
  storage.
- Vulkan report submission now keeps batch aggregation in
  `vulkan_report_submission.cpp`, text page expansion in
  `vulkan_report_text_submission.cpp`, and summary counters in
  `vulkan_report_submission_stats.cpp`.

## 2026-07-03 Structural Optimization Task 3 Public UI Header Surgery

- Public text APIs now have focused leaf headers. `text_layout.hpp` is only the
  compatibility aggregate over shape, glyph, measurement, wrapping, and
  hit-testing domains, while `TextEditAction` is no longer owned by
  `text_model.hpp`.
- `TextModel` is now closer to a normal public class declaration:
  construction, editing, history, navigation, and selection behavior live in
  focused `src/ui/text_model*.cpp` units. The header keeps private helper
  declarations because the class is still value-owned rather than PIMPL-backed.
- Runtime public types are no longer concentrated in `runtime_types.hpp`.
  The new runtime leaf headers map to callbacks, ids, handles, window options,
  app context, actions, events, diagnostics, input state, runtime context, and
  rendering/app-runner free functions.
- `WindowRuntime` private data and helper declarations are now isolated in
  `src/ui/window_runtime_internal.hpp` as a class-body declaration slice. This
  avoids a full storage/PIMPL migration while still removing the private bulk
  from the public facade header.
- Current Task 3 line counts are: `text_layout.hpp` 6,
  `text_model.hpp` 118, `runtime_types.hpp` 11, `window_runtime.hpp` 207,
  and `src/ui/window_runtime_internal.hpp` 235. The largest new text leaf is
  `text_glyphs.hpp` at 133 lines, and the largest new runtime leaf is
  `runtime_context.hpp` at 166 lines.
- Windows and WSL Arch Linux focused public-header verification both passed
  8/8, and `git diff --check` exited 0 with only existing CRLF normalization
  warnings.

## 2026-07-03 Structural Optimization Task 4 Public Element And Style Header Surgery

- The public element/style surface now uses thin compatibility aggregates plus
  focused leaves. The key old public headers are all below the 220-line guard:
  `element_tree.hpp` 104, `element_containers.hpp` 4,
  `element_interaction_nodes.hpp` 5, `element_builder.hpp` 3,
  `widget_builders.hpp` 6, and `style_core.hpp` 8.
- Moving only the public header declarations was not enough for the user's
  "thorough optimization" bar, because it created large replacement
  implementation files. The architecture test now also guards the new
  element/style implementation split: old implementation entry files stay
  1-line shells, `element_tree_reconcile.cpp` is 124 LF, and
  `element_flex_layout.cpp` is 144 LF after the second split.
- `Style` and `StyleOverlay` chain setters are no longer inline in the public
  headers. This intentionally trades some constexpr/header-only behavior for
  cleaner ABI and compile boundaries, matching the stated preference to
  optimize structure over compatibility conservatism.
- Windows and WSL Arch Linux focused verification for Task 4 both passed 6/6:
  `ui_source_structure_test`, `ui_header_cleanliness`, `element_test`,
  `style_test`, `render_view_test`, and `window_runtime_test`.
- `git diff --check` exited 0 with only existing CRLF normalization warnings
  and no whitespace errors after the Task 4 split.

## 2026-07-04 Structural Optimization Task 5 Renderer Platform Core Header Surgery

- Renderer, platform, and core public headers now follow the same aggregate plus
  leaf-header structure as the UI surface. The compatibility aggregates are
  intentionally tiny: `renderer.hpp` 7 lines, `glyph_atlas.hpp` 7,
  `renderer_reports.hpp` 7, `platform.hpp` 9, and `events.hpp` 8.
- The largest Task 5 renderer leaf headers are still comfortably under the
  220-line public-leaf target: `renderer_frame_reports.hpp` 184 and
  `glyph_cache.hpp` 180. Platform and core event leaves are all under 60 lines.
- Header-cleanliness verification caught two real split-boundary problems:
  `renderer_frame_reports.hpp` depended on `GlyphCache` without a direct
  include, and `glyph_cache.hpp` exposed command-facing signatures without
  directly including `renderer_commands.hpp`. The fix keeps leaf headers
  self-contained instead of relying on aggregate include order.
- The renderer/platform/core split stayed structure-only. Windows focused Task
  5 verification passed 8/8, WSL Arch Linux focused verification passed 5/5
  available targets, and Task 6 can now move to splitting the oversized
  `tests/ui/window_runtime_test.cpp` by runtime domain.

## 2026-07-04 Structural Optimization Task 6 Test Suite Structure Split

- `tests/ui/window_runtime_test.cpp` is no longer the runtime-test monolith. It
  is now a 41-line smoke target, while the behavioral coverage lives in focused
  binaries for input, focus, actions, text, rendering, scheduling, multiwindow,
  and theme domains.
- A shared `window_runtime_test_support.hpp` carries the common fake
  platform/renderer/view fixture. It is still the largest test-support file at
  1485 lines, but it replaces repeated fixture copies while the actual test
  ownership is now distributed across independently runnable targets.
- The split exposed two mechanical boundary hazards that are worth remembering:
  dispatch helper globals must move with their dispatch functions, and the last
  extracted range must not include the original file's anonymous-namespace
  closing brace when the new wrapper closes its own namespace.
- `window_runtime_focus_test.cpp` was added beyond the original seven-file
  plan because focus, tab traversal, keyboard-focus ownership, and scroll
  routing were large enough to deserve their own domain instead of inflating
  `window_runtime_input_test.cpp`.
- Windows and WSL Arch Linux focused Task 6 verification both passed 10/10,
  including `ui_source_structure_test` and every split runtime test target.
  Task 7 can now run final aggregation on Windows and WSL.

## 2026-07-04 Structural Optimization Task 7 Final Aggregation

- The earlier handoff-reported `vulkan_solid_rect_test/default` exit-5 failure
  was not reproducible in the final verification state. A focused rerun passed,
  and the full Windows aggregation later passed the same target as part of
  41/41 tests. No production Vulkan synchronization change was made without a
  reproducible root cause.
- Windows full debug verification passed 41/41 after `xmake f -c -m debug -P .`
  and `xmake test -P .`. WSL Arch Linux full debug verification passed 38/38
  after the corresponding `XMAKE_ROOT=y xmake f -y -c -m debug -P .` and
  `XMAKE_ROOT=y xmake test -y -P .` run.
- `git diff --check` exits 0 for the final structural branch. The only output
  is Git's expected LF-to-CRLF normalization warnings for touched text files.
- The aggressive structure pass leaves the old public and private aggregates
  thin while keeping leaf files under the encoded guards: public UI hot headers
  are now `text_model.hpp` 131, `runtime_types.hpp` 12,
  `text_layout.hpp` 7, `window_runtime.hpp` 219,
  `element_tree.hpp` 104, and `style_core.hpp` 8; renderer/platform/core
  aggregates are `renderer.hpp` 7, `platform.hpp` 10, and `events.hpp` 8.
- The main private/bridge hotspots are also reduced to small dispatch or
  declaration boundaries: `wayland_application_internal.hpp` 3,
  `wayland_window_internal.hpp` 107, `vulkan_internal.hpp` 6,
  `win32_window_proc.cpp` 33, `wayland_application_windows.cpp` 5,
  `vulkan_swapchain_create.cpp` 65, and
  `vulkan_report_submission.cpp` 100.
- The remaining large files are intentionally concentrated in test fixture
  support and split test domains rather than production API or implementation
  monoliths. `tests/ui/window_runtime_test.cpp` is now 41 lines, while
  `window_runtime_test_support.hpp` and the focused runtime test targets carry
  the detailed behavioral coverage.

## 2026-07-04 Structural Optimization Task 3 Follow-up Audit

- Task 3's public text header split was not complete until the text layout
  helper implementations also moved out of public leaf headers. The durable
  guard is now in `ui_source_structure_test`: it requires
  `src/ui/text_shape.cpp`, `src/ui/text_glyph_raster.cpp`,
  `src/ui/text_measurement.cpp`, `src/ui/text_wrapping.cpp`, and
  `src/ui/text_hit_testing.cpp`, and it rejects the `" inline "` token in the
  matching public text headers.
- The focused text implementation files intentionally live under `src/ui/` to
  match the Task 3 structure boundary, but they are compiled into the
  `cgpui_renderer` target. The Vulkan renderer consumes glyph rasterization and
  text paint metadata directly, so compiling them only into `cgpui_ui` would
  invert the dependency boundary.
- The follow-up verification passed Windows full debug 41/41 and WSL Arch Linux
  full debug 38/38 after the text implementation migration. `git diff --check`
  produced only expected LF-to-CRLF normalization warnings.

## 2026-07-04 GPUI Upstream Parity Ledger Phase A

- The concrete full-replication baseline is now pinned to upstream Zed/GPUI
  commit `5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`, with recorded crate
  versions `gpui = 0.2.2` and `gpui_platform = 0.1.0`. This avoids chasing a
  moving pre-1.0 upstream target while implementing the remaining parity rows.
- The Phase A ledger establishes four status classes: `Required`, `Adapted`,
  `Deferred`, and `Non-goal`. Windows Win32 + Vulkan and Linux Wayland +
  Vulkan are required; macOS Cocoa + Metal is deferred to the later Mac phase;
  X11 remains deferred unless strict upstream Linux backend parity is
  explicitly accepted; wasm is a non-goal for the desktop target.
- The extractor is intentionally best-effort and deterministic. It fetches the
  pinned upstream crate root and Cargo.toml when network is available, but the
  example inventory falls back to the pinned default set if the network cannot
  be reached. On Windows, the fresh snapshot contained 55 public re-exports and
  20 examples.
- The first API parity gate is documentation/tooling/example oriented rather
  than a broad runtime behavior test. It proves the pin, ledger, JSON export,
  extractor, Phase A plan, xmake targets, and public hello-world parity example
  are present and wired through the build.
- Phase B should close ledger rows rather than inventing scope from memory.
  The immediate next implementation band is public `Application` / `App` /
  `Window` / `Context<T>` / entity / action / key-dispatch / test-context
  parity for Windows/Linux.

## 2026-07-04 Phase B Step 259 Application Facade

- `Application` belongs in a new app module, not in `ui.cpp`, runtime
  implementation files, or platform backends. The Step 259 structure is:
  `include/cgpui/app/application.hpp`, `include/cgpui/app/app.hpp`, and
  `src/app/application.cpp`, compiled by the new `cgpui_app` target.
- The facade intentionally wraps existing primitives instead of replacing them:
  `Application::create()` delegates to `create_platform_application()`, and
  `Application::run(...)` delegates to `run_app(...)`. This keeps existing
  low-level callers source-compatible while adding GPUI-shaped entry spelling.
- The WSL `application_facade_test/default` failure was a test lifetime bug,
  not a Linux platform failure. The test kept a raw pointer to a renderer owned
  by `run_app`'s local renderer vector and read it after `Application::run`
  returned. Direct execution happened to read stale memory that still looked
  valid; xmake's test runner exposed the use-after-free. The durable pattern is
  to record renderer observations through external counters, as
  `tests/ui/app_runner_test.cpp` already does.
