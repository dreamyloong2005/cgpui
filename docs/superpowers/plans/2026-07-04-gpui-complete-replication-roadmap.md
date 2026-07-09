# CGPUI Complete GPUI Replication Roadmap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Drive CGPUI from the current post-Step-218 Windows/Linux GPUI-core state to a complete, audited GPUI replication in C++23.

**Architecture:** Pin an upstream GPUI revision, maintain a parity ledger, then close each API and behavior gap with RED/GREEN implementation slices. Windows and Linux continue on Vulkan, Linux remains Wayland-first, macOS starts later with Cocoa + Metal, and every new feature must land in its intended leaf module from the first version.

**Tech Stack:** C++23, Xmake, Win32 + Vulkan, Wayland + Vulkan, Cocoa + Metal, DirectWrite, fontconfig/FreeType/HarfBuzz, CoreText, architecture/header-cleanliness tests, Windows and WSL Arch Linux verification.

---

## Source Baseline

Upstream GPUI remains pre-1.0 and can break between versions, so "complete"
must mean complete against a pinned upstream revision, not a moving target.
The current official sources checked for this roadmap are:

- Zed GPUI README: `https://github.com/zed-industries/zed/blob/main/crates/gpui/README.md`
- GPUI website and examples index: `https://gpui.rs/`
- GPUI crate root: `https://github.com/zed-industries/zed/blob/main/crates/gpui/src/gpui.rs`
- Context docs: `https://github.com/zed-industries/zed/blob/main/crates/gpui/docs/contexts.md`
- Key dispatch docs: `https://github.com/zed-industries/zed/blob/main/crates/gpui/docs/key_dispatch.md`

The official surface to replicate includes the three GPUI registers:
entities/state, views/rendering, and low-level elements. It also includes
actions/key dispatch, platform services, async executor integration, test
context support, and examples such as hello world, animation, image/GIF, input,
opacity, menus, shadow, SVG, text wrapping, uniform lists, and window behavior.

## Current CGPUI Baseline

- Steps 1-218 are merged to `master`.
- The latest completed production-depth step is Step 218, threaded async
  executor and cancellation.
- The aggressive structural optimization pass is merged and post-merge
  verified: Windows full debug passed 41/41, WSL Arch Linux full debug passed
  38/38.
- `AGENTS.md` is now authoritative for future development: implement in the
  intended module from the first version, keep aggregate headers thin, avoid
  dumping into `ui.cpp` or broad runtime/platform/renderer files, and add
  structure tests whenever boundaries matter.
- Windows and Linux are the active targets. Windows uses Win32 + Vulkan.
  Linux uses Wayland + Vulkan. macOS uses Cocoa + Metal later.
- X11 is not part of the active Windows/Linux core track. If "full GPUI
  platform matrix parity" is later interpreted literally, X11 should be added
  as a late optional backend after Wayland production behavior is stable.

## User Scope Decision After Phase C

The post-Phase-C execution target is a desktop C++23 GPUI runtime, not a game
engine integration project. The following scope decision is binding for future
slices unless the user explicitly changes it:

- Deferred: game-engine-specific integration, engine runtime embedding, shared
  engine renderer/resource/asset/command systems, game editor/runtime UI depth,
  Android, iOS, and X11.
- Required for the active track: C++23-native public APIs without Rust FFI,
  modern C++ result/optional/concepts/constexpr/ranges usage, Vulkan-first
  production renderer depth, declarative widgets, reactive state/subscription
  depth, low-allocation runtime structures, static and dynamic widget support,
  editor/AI-IDE-class text and tool UI primitives, Win32 and Wayland production
  platform behavior, and later macOS Cocoa + Metal parity.
- DirectX/WebGPU/other renderer backends are not active implementation tracks
  yet, but the renderer architecture must keep backend extension boundaries
  clean enough that they can be added later without rewriting public UI APIs.

## Zero-Cost Abstraction Principle

Future slices must treat zero-cost abstraction as an engineering constraint,
not a slogan. Ergonomic public APIs are welcome, but hot runtime paths must not
pay hidden costs that a direct C++ implementation would avoid.

- Static fast paths come first for layout, element diff/reconcile, render
  command generation, event routing, and reactive invalidation.
- Templates, concepts, `constexpr`, `std::expected`, `std::optional`, and
  ranges should express type-safe APIs without forcing extra heap allocation,
  virtual dispatch, broad type erasure, or cross-module handle wrapping on hot
  paths.
- Dynamic widget/plugin/editor escape hatches are allowed, but they must be
  explicit boundaries with measurable cost; they must not leak into the static
  widget fast path by default.
- Renderer and UI internals should lower declarative builders into compact
  data-oriented structures, retained node storage, or command buffers before
  hot traversal. Avoid per-frame tree-wide scans, repeated string lookups, and
  avoidable `std::function`/heap churn.
- When a slice introduces or depends on an abstraction boundary, add behavior,
  structure, or micro-benchmark style tests that can catch accidental
  allocation, type-erasure, virtual-dispatch, or command-buffer regressions.

The first enforcement slice after Phase C is the static element fast path:
dense `StaticElementNode` records, span-backed child lists,
`StaticElementTreeView`, and `StaticRender` provide a static composition route
while `AnyElement` and the polymorphic `ElementTree` remain an explicit
dynamic escape hatch.

## Pre-Phase-D Entry Gate

Phase D feature work may continue only after the post-Phase-C scope decision,
zero-cost abstraction principle, and first static fast path are all preserved
by focused gates on Windows and WSL Arch Linux. This prevents new text, IME,
renderer, or platform work from drifting back toward hidden allocation,
unbounded dynamic dispatch, or user-deferred platform/engine scope.

- Scope gate: game-engine-specific integration, engine runtime embedding,
  shared engine renderer/resource/asset/command systems, game editor/runtime
  UI depth, Android, iOS, and X11 remain deferred unless the user explicitly
  reopens them.
- Active-track gate: C++23-native public APIs without Rust FFI,
  Vulkan-first production renderer depth, declarative widgets, reactive
  state/subscription depth, low-allocation runtime structures, static and
  dynamic widget support, editor/AI-IDE-class text and tool UI primitives,
  Win32 and Wayland production platform behavior, and later macOS Cocoa +
  Metal parity remain required.
- Zero-cost gate: static fast paths, explicit dynamic escape hatches, compact
  retained records/command buffers, and tests for abstraction boundaries must
  stay present; hot paths must avoid hidden allocation, broad type erasure,
  avoidable virtual dispatch, per-frame tree-wide scans, repeated string
  lookups, and avoidable `std::function`/heap churn.
- Verification gate: `pre_phase_d_entry_gate_test/default`,
  `phase_c_final_ledger_audit_test/default`,
  `static_render_runtime_test/default`, `ui_source_structure_test/default`,
  and `gpui_parity_ledger_test/default` must pass before continuing Phase D.

## Verification Cadence and WSL Cost Control

WSL verification is required, but full WSL rebuilds must be batched so coding
time is not dominated by repeated Linux compile/link work. Future slices use
this cadence unless the user explicitly asks for stricter verification:

- Every implementation slice runs Windows focused behavior/structure tests,
  ledger or JSON checks when touched, and `git diff --check`.
- WSL focused tests run when the slice touches shared public headers,
  templates/constexpr/concepts, `xmake.lua`, shared UI runtime, renderer,
  Vulkan, platform, Wayland, or cross-platform file structure.
- WSL full debug runs at Phase closeout, before declaring a Phase complete,
  and at intentionally batched renderer/platform milestones. It is not the
  default after every small step.
- Windows full debug remains the default post-step full-suite gate because it
  is much cheaper on this machine and catches most API/structure regressions.
- WSL build/cache output must stay on D-drive paths such as
  `.build-wsl/master`; transient temp should use `/dev/shm/cgpui` where
  practical. Do not put WSL package caches, global xmake state, or temp output
  on C: for this project.
- Docs-only, ledger-only, and test-only slices may skip WSL when they do not
  affect compiled cross-platform code, but the skipped scope must be recorded
  in the final verification summary.

## Completion Definition

CGPUI is not "fully replicated" until all of these are true:

- [ ] A pinned upstream GPUI revision is recorded and every public upstream
  module, type, trait concept, function, method, macro-equivalent, example, and
  documented behavior has a C++ parity status.
- [ ] Every required GPUI public API has a C++23 equivalent or a documented
  intentional C++ adaptation with compatibility tests.
- [ ] The examples from `gpui.rs` compile and run through public CGPUI APIs on
  Windows and Linux, and later on macOS after the Mac track opens.
- [ ] Windows full debug passes after every merge.
- [ ] WSL Arch Linux focused tests pass for every shared, renderer, Wayland,
  platform, build, or public-header-facing slice, and WSL Arch Linux full
  debug passes at each Phase closeout or intentional batched milestone.
- [ ] macOS full debug passes after the Mac track opens.
- [ ] Architecture/header-cleanliness tests prove new work did not collapse
  back into broad files or fat aggregate headers.
- [ ] Renderer output is no longer metadata-only for core primitives: solid
  rect, rounded rect, text glyphs, images/SVG, clips, opacity, transforms, and
  batching all reach real backend draw/upload paths.
- [ ] Win32 and Wayland have production native behavior for window lifecycle,
  input, IME, clipboard, drag/drop, menus, dialogs, accessibility, cursors,
  multi-window, and platform diagnostics.
- [ ] Cocoa + Metal reaches the same public behavior as Windows/Linux.
- [ ] The parity ledger has no required gaps. Any non-goal, such as active X11,
  Android/iOS, or game-engine-specific integration exclusion, is explicitly
  accepted by the user.

## Step Count Estimate

The current codebase is close to a GPUI-core-shaped API, but not close to a
complete upstream GPUI replication. From Step 218:

- Windows/Linux requested-target production parity is roughly Steps 219-678.
- macOS Cocoa + Metal parity is roughly Steps 679-758.
- Strict upstream X11 platform parity, if accepted later, is roughly
  Steps 759-798.
- Final upstream audit closure and release hardening is roughly Steps 799-840+.

That means about 460 implementation slices remain for strong Windows/Linux
production parity, about 540 slices for Windows/Linux plus macOS, and about
580-620 slices for a strict "everything upstream GPUI exposes" interpretation.
The exact count must be corrected after the pinned upstream parity ledger is
generated in Phase A.

## Universal Execution Protocol

Every implementation slice below follows the same loop:

- [ ] Start in a feature worktree under `.worktrees/`.
- [ ] Decide the ownership boundary before writing code: public leaf header,
  compatibility aggregate, private/internal header, and focused `.cpp` file.
- [ ] Add RED behavior or structure coverage first.
- [ ] Implement the smallest GREEN slice in the intended module.
- [ ] Run targeted Windows tests.
- [ ] Run WSL focused tests when the slice touches shared UI, renderer,
  platform, Wayland, build, templates/constexpr/concepts, or public headers.
- [ ] Run `git diff --check`.
- [ ] Fast-forward merge to `master`.
- [ ] Run post-merge Windows full debug. Run WSL full debug only at Phase
  closeout, user-requested verification points, or intentional batched
  renderer/platform milestones; otherwise record the WSL focused gate or the
  docs-only/test-only reason for skipping WSL.
- [ ] Update `task_plan.md`, `findings.md`, `progress.md`, and the parity
  ledger before deleting the feature worktree.

## Phase A: Steps 219-258 - Upstream Parity Ledger

**Goal:** Stop estimating from memory and create the source-of-truth gap list
for full replication.

**Primary files:**
- Create: `docs/gpui-upstream-pinned-revision.md`
- Create: `docs/gpui-complete-parity-ledger.md`
- Create: `tools/gpui_parity/README.md`
- Create: `tools/gpui_parity/extract_upstream_symbols.*`
- Modify: `docs/gpui-core-api-parity.md`
- Add tests under: `tests/architecture/`, `tests/header_cleanliness/`,
  `tests/api_parity/`

- [ ] Steps 219-222: Pin the upstream Zed/GPUI revision, record URLs, commit
  hash, crate versions if available, and the exact docs/examples snapshot used
  for parity.
- [ ] Steps 223-226: Build a symbol and docs extractor for upstream GPUI public
  modules, re-exports, examples, documented concepts, and platform services.
- [ ] Steps 227-230: Build the C++ API ledger by mapping each upstream concept
  to a CGPUI header, source file, test target, status, and missing behavior.
- [ ] Steps 231-234: Add `api_parity` tests that fail when required public
  leaf headers are missing, when aggregate headers grow, or when examples avoid
  the public prelude.
- [ ] Steps 235-238: Split the ledger into required, adapted, deferred, and
  non-goal categories. X11 must stay deferred unless the user explicitly
  chooses strict upstream Linux backend parity.
- [ ] Steps 239-242: Update `docs/gpui-core-api-parity.md` from the new
  ledger, replacing stale Step 168 wording and distinguishing Step 218
  skeletons from production behavior.
- [ ] Steps 243-246: Add a `hello_world` API parity example that mirrors the
  official GPUI hello-world shape in C++ terms: `Application`, `App`, `Window`,
  `Context<T>`, `Render`, `div`, pixels, colors, bounds, and children.
- [ ] Steps 247-250: Add parity gates for official example inventory:
  animation, GIF, image, input, opacity, menus, shadow, SVG, text wrapping,
  uniform list, window positioning, window shadow, and window operations.
- [ ] Steps 251-254: Add a machine-readable status export so later phases can
  close one ledger row per implementation slice.
- [ ] Steps 255-258: Run full Windows and WSL verification, then mark Phase A
  complete only when the ledger is good enough to drive the remaining phases.

## Phase B: Steps 259-318 - Public Application, Context, Entity, And Action API

**Goal:** Bring the public C++ authoring model close to GPUI's app/context
shape before deeper native work expands platform behavior.

**Primary modules:**
- `include/cgpui/cgpui.hpp`
- `include/cgpui/app/*`
- `include/cgpui/ui/runtime_*`
- `include/cgpui/ui/view*`
- `include/cgpui/ui/entity*`
- `include/cgpui/ui/action*`
- `src/ui/runtime_*`
- `src/ui/app_context_*`
- `tests/api_parity/*`
- `tests/ui/*`

- [x] Steps 259-264: Align public names and C++ idioms for `Application`,
  `App`, `Window`, `Context<T>`, `Render`, `IntoElement`, and `View` without
  breaking the existing public prelude. Step 259 is complete: `Application`
  now owns a `PlatformApplication`, exposes `Application::create()`, and
  forwards `run(...)` to the existing app runner from a focused app module.
  Step 260 is merged on `master` at `d3a501c`: `AppContext::app()`,
  `WindowRuntimeContext::app()`, and `WindowRuntimeContext::current_window()`
  return public `App`/`Window` facades for window opening and runtime window
  inspection without moving implementation into UI/runtime monoliths.
  Step 261 adds the focused public render spelling leaf
  `include/cgpui/ui/render.hpp`, exposing `Context<T>`, `IntoElement`, and the
  `Render<T>` concept while keeping `View` in `view.hpp` and preserving the
  existing `ViewContext` ABI spelling. Step 262 adds public
  `Context<T>` authoring helpers for `window()`, entity handles, and weak
  entity creation; the lower-level `PlatformWindow&` field is now named
  `platform_window` so the GPUI-shaped `window()` facade spelling is available
  without losing explicit platform access. Step 263 adds a focused
  `view_handle.hpp` public leaf with typed `ViewHandle<T>` and
  `WeakViewHandle<T>` wrappers over the existing `ViewId` registry, plus
  context helpers for current-view handle creation, weak handles, typed
  upgrade, and read-only lookup. Step 264 closes the public-name band with
  `cgpui/prelude.hpp`, keeps `cgpui/cgpui.hpp` as a thin compatibility
  aggregate, and adds an integrated public authoring compile gate that uses
  only the public prelude for `Application`, `App`, `Window`, `Context<T>`,
  `Render`, `IntoElement`, and typed view/entity handles.
- [x] Steps 265-270: Complete entity lifecycle semantics: creation, weak
  handles, observation, update transactions, invalidation, deletion, and
  cross-context access rules. Step 265 starts this band with public
  `Context<T>::new_entity<T>(...) -> EntityHandle<T>` creation and
  `insert_entity_handle(...)` over the existing runtime store while keeping
  observation, update transactions, deletion, and cross-context rules for the
  later slices in this band. Step 266 adds public
  `WeakEntity<T>::upgrade(context) -> std::optional<EntityHandle<T>>` and
  `WeakEntity<T>::read(context)` soft-failure semantics on top of the existing
  runtime weak lookup. Step 267 adds public entity observation helpers through
  `EntityHandle<T>::observe(...)`, `EntityHandle<T>::observe_subscription(...)`,
  `Context<T>::observe_entity(...)`, and
  `Context<T>::observe_entity_subscription(...)`, delegating to the existing
  runtime observer path without pulling update transactions, deletion, or
  cross-context access rules forward. Step 268 adds `Context<T>::update_entity(...)`
  and value-returning `EntityHandle<T>::update(...)` transaction helpers whose
  callbacks can receive the entity state and a typed `Context<T>` view, while
  preserving the old `bool` result for void updates and keeping invalidation,
  deletion, and cross-context rules for later slices. Step 269 adds explicit
  `EntityHandle<T>::invalidate(context)` and
  `Context<T>::invalidate_entity(...)` helpers that notify observers and
  subscribed views for existing entities and request redraw even when no
  observer exists. Step 270 adds public
  `EntityHandle<T>::remove(context)` and
  `Context<T>::remove_entity(handle)` deletion helpers plus runtime-token
  cross-context boundaries so context-created handles soft-fail when used
  against another runtime, even when the typed numeric id matches.
- [x] Steps 271-276: Add GPUI-like context capabilities by domain:
  app context, view context, window context, element context, async context,
  and test context. Step 271 starts this band with
  `Context<T>::app_context() -> AppContext`, reusing the existing app-domain
  facade for app/global/theme/window-opening capability access from authoring
  contexts. Step 272 adds the view-domain capability
  `Context<T>::view_context<T>() -> ViewContextCapability<T>`, layered over
  the existing typed view handle registry without introducing new view
  lifecycle or subscription semantics. Step 273 adds the window-domain
  capability `Context<T>::window_context() -> WindowContextCapability`,
  grouping existing `Window` facade queries and invalidation requests without
  adding native window lifecycle state. Step 274 adds
  `Context<T>::element_context(ElementId) -> ElementContextCapability`,
  grouping existing element focus, pointer capture, cursor, and state helpers
  without weakening runtime-token entity boundaries. Step 275 adds
  `Context<T>::async_context() -> AsyncContextCapability`, grouping existing
  defer, timer, animation, foreground task, background task, and update-batch
  scheduling without moving executor state or runtime-token entity rules.
  Step 276 adds `Context<T>::test_context() -> TestContextCapability`,
  grouping existing deterministic runtime observability and queue controls
  without claiming the later Step 295-300 simulated input, clipboard, timer,
  async, and redraw behavior band.
- [x] Steps 277-282: Deepen subscriptions and observations: scoped lifetime,
  entity-to-entity observation, window/view observation, and deterministic
  unsubscribe behavior. Step 277 adds deterministic `Subscription` lifetime:
  explicit `release()` removes observer records once, moved-from subscriptions
  soft-fail, scoped destruction unsubscribes, diagnostics drop disconnected
  observers, and the implementation is split into `src/ui/subscription.cpp`
  and `src/ui/runtime_subscriptions.cpp`. Step 278 adds entity-to-entity
  observation: an `EntityHandle<ObserverT>` can observe another
  `EntityHandle<ObservedT>`, callbacks receive mutable observer state, the
  observed handle, and the current context, and subscription lifetime continues
  to use the Step 277 token path. Step 279 adds window/view observation
  helpers through `Context<T>::observe_window(...)`,
  `Context<T>::observe_view(...)`, `WindowContextCapability::observe(...)`,
  `ViewContextCapability<T>::observe(...)`, and `ViewHandle<T>::observe(...)`,
  with deterministic `Subscription::release()` cleanup through the same
  subscription path and focused runtime storage in
  `src/ui/runtime_observations.cpp`. Step 280 adds observer diagnostics:
  `RuntimeDiagnosticsSnapshot` exposes entity, window, and view observer
  counts separately, and a focused runtime test verifies window/view
  subscription release updates those counts without adding new observer
  ownership paths. Step 281 adds runtime-token boundaries to `ViewHandle<T>`,
  `WeakViewHandle<T>`, and `WeakView`, so read, weak upgrade, and observation
  helpers reject handles captured from a different `WindowRuntime` even when
  numeric `ViewId` values collide. Step 282 closes the
  observation/subscription band with a focused parity audit and
  `WindowRuntime::remove_view(...)` cleanup for removed-view subscriptions;
  its guard verifies removed views release both view observers and view-owned
  entity subscriptions before actions and key dispatch begin.
- [x] Steps 283-288: Bring actions closer to upstream: typed action structs,
  action registration, action dispatch, action scope, command metadata,
  enablement, and bubbling through focused routes. Step 283 landed the public
  `Action<T>` typed action concept and `action_name<T>()` authoring surface in
  `include/cgpui/ui/action.hpp`; Step 284 landed typed action
  registration/dispatch overloads through
  `include/cgpui/ui/runtime_action_templates.hpp` over the existing string
  registry. Step 285 adds `ActionRegistration` scope metadata in
  `include/cgpui/ui/runtime_actions.hpp` and
  `src/ui/runtime_action_metadata.cpp`; Step 286 adds typed action
  command-palette metadata binding in
  `include/cgpui/ui/runtime_command_palette_templates.hpp`; Step 287 adds
  action enablement metadata through `ActionRegistrationOptions`,
  `include/cgpui/ui/runtime_action_enablement_templates.hpp`,
  `src/ui/runtime_action_registration.cpp`, and disabled-dispatch coverage.
  Step 288 adds action bubbling through focused routes in
  `src/ui/runtime_action_dispatch.cpp`, where unhandled or disabled
  focused/view/window handlers continue outward and consumed or cancelled
  handlers stop dispatch.
- [x] Steps 289-294: Expand key dispatch parity: key binding grammar,
  platform modifiers, keymap contexts, partial matches, disabled scopes, and
  command palette integration. Step 289 adds the focused
  `include/cgpui/ui/key_binding.hpp` leaf, `parse_key_binding(...)`, and
  `Context<T>::bind_key("ctrl-shift-s", "action")` grammar registration over
  the existing dispatch path. Step 290 adds platform modifier semantics:
  `secondary-*` maps to Ctrl on Windows/Linux and Super on macOS, while
  `platform-*` / `cmd-*` / `win-*` bind the platform key through the focused
  `src/ui/runtime_key_binding_modifiers.cpp` boundary. Step 291 adds
  `KeyBindingContext` app/window/view/focused-element activation and routes
  same-chord bindings through the most specific active context in
  `src/ui/runtime_key_binding_contexts.cpp`. Step 292 adds multi-chord
  `KeyBindingChord` sequences, whitespace-separated key grammar such as
  `ctrl-k ctrl-s`, and pending partial-match dispatch state in
  `src/ui/runtime_key_binding_sequences.cpp`. Step 293 adds disabled
  `KeyBindingContext` scope filtering so disabled view/window/focused key
  scopes are skipped without blocking outer enabled bindings. Step 294 adds
  command-palette-owned `key_binding` and optional explicit `key_context`
  metadata, derives app/window/view/focused-element key contexts from palette
  scope when needed, installs valid enabled entries through
  `src/ui/runtime_command_palette_keys.cpp`, and keeps fuller test-context
  simulation for later slices.
- [x] Steps 295-300: Implement test-context equivalents for simulating
  keystrokes, pointer input, window focus, clipboard, timers, async tasks, and
  redraws. Step 295 is merged on `master` at `04cfa7a`: `TestContextCapability`
  now exposes direct `dispatch_keystroke(...)` plus grammar-backed
  `simulate_keystrokes(...)`, with the implementation isolated in
  `src/ui/test_context_keystrokes.cpp` and verified through the real runtime
  keyboard/keymap/action dispatch path. Step 296 adds direct pointer
  move/button/scroll simulation through the real runtime event path, with the
  implementation isolated in `src/ui/test_context_pointer.cpp`. Step 297 adds
  direct window activation/focus dispatch plus element focus/release helpers,
  with the implementation isolated in `src/ui/test_context_focus.cpp`. Step
  298 is merged on `master` at `96c7c96`: it adds direct clipboard read/write
  helpers plus copy/cut/paste forwarding over the existing runtime clipboard
  operations, with the implementation isolated in
  `src/ui/test_context_clipboard.cpp` and runtime text read/write ownership
  kept in `src/ui/runtime_clipboard.cpp`. Step 299 adds
  `run_until_parked()` and `advance_time_until_parked(...)` over the real
  runtime wakeup order for ready task completions, due timers, deferred
  callbacks, and deferred redraw flushing without advancing future timers;
  scheduling helper ownership is isolated in
  `src/ui/test_context_scheduling.cpp`. Step 300 is merged on `master` at
  `8ac5aa0`: it adds `request_redraw()` and `draw_frame()` test-context
  helpers over the real runtime redraw scheduling and
  `WindowRedrawRequested` frame path, with rendering helper ownership isolated
  in `src/ui/test_context_rendering.cpp`.
- [x] Step 301: Add public window-opening result conventions. Merged on
  `master` at `53e625a` and post-merge verified with Windows full-debug 87/87
  and WSL Arch Linux full-debug 84/84. `App`, `AppContext`, and `WindowRuntime`
  now expose
  `try_open_window(...) -> Result<AppOpenedWindow>` over a focused
  `src/ui/runtime_window_results.cpp` implementation. Failed platform-window
  or renderer creation returns `Error` without publishing an app-opened window
  record, while existing `open_window(...)` compatibility behavior remains.
- [x] Step 302: Add public platform-service result conventions. Merged on
  `master` at `8998ec2` and post-merge verified with Windows full-debug 88/88
  and WSL Arch Linux full-debug 85/85. `WindowRuntime`, `AppContext`, and
  `ViewContext`/`WindowRuntimeContext` expose
  `try_install_native_menu(...) -> Result<NativeMenuInstallation>` and
  `try_show_native_file_dialog(...) -> Result<NativeFileDialogResult>` through
  focused `src/ui/runtime_platform_service_results.cpp` ownership. Unsupported
  services return `ErrorCode::unsupported_platform` without overwriting the
  last successful runtime service state; supported file-dialog cancellation
  remains a value, not an error. Existing compatibility methods remain.
- [x] Step 303: Add async-spawn result conventions. Completed on `master` at
  `085cd30` and post-merge verified with Windows full-debug 89/89 and WSL Arch
  Linux full-debug 86/86. `WindowRuntime`, `WindowRuntimeContext`, and
  `AsyncContextCapability` expose
  `try_spawn_task(...) -> Result<TaskHandle>` and
  `try_spawn_background_task(...) -> Result<TaskHandle>` through focused
  `src/ui/runtime_task_results.cpp` ownership. Empty foreground completion,
  background work, or background completion callbacks return
  `ErrorCode::invalid_argument` without creating task records, while existing
  `spawn_*` compatibility methods still return an empty `TaskHandle`.
- [x] Step 304: Add renderer-creation result conventions. Completed on
  `master` at `5f09830` and post-merge verified with Windows full-debug 90/90
  and WSL Arch Linux full-debug 87/87. `WindowRuntime::try_create_renderer(...) ->
  Result<Renderer*>` owns the explicit renderer factory Result boundary in
  focused `src/ui/runtime_renderer_results.cpp`; `run(...)` and child-window
  activation now call that boundary instead of calling `renderer_factory_(...)`
  directly.
- [x] Step 305: Add renderer-resize result conventions. Merged on `master` at
  `a147283` and post-merge verified with Windows full-debug 91/91 and WSL Arch
  Linux full-debug 88/88. `WindowRuntime::try_resize_surface(...) ->
  Result<void>` owns the renderer-resize Result boundary in focused
  `src/ui/runtime_renderer_resize_results.cpp`, while `resize_surface(...)`
  remains the fail-and-quit compatibility wrapper.
- [x] Step 306: Finish renderer frame/redraw result conventions. Merged on
  `master` at `4649f71` with
  `WindowRuntime::try_draw_frame() -> Result<void>` and
  `TestContextCapability::try_draw_frame() -> Result<void>`, isolated frame
  Result ownership in `src/ui/runtime_renderer_frame_results.cpp`, and kept
  existing redraw event / `draw_frame()` compatibility as fail-and-quit
  wrappers. Focused Windows GREEN verification passed
  `renderer_frame_result_conventions_test/default` and
  `ui_source_structure_test/default`; post-merge verification passed Windows
  full-debug 92/92 and WSL Arch Linux full-debug 89/89.
- [x] Step 307: Add the first public API compatibility example. Merged on
  `master` at `8ffc1bd` with
  `examples/api_parity/public_api_compatibility/main.cpp`,
  `api_parity_public_api_compatibility`, and
  `tests/api_parity/public_api_compatibility_examples_test.cpp`. The example
  compiles using only `cgpui/prelude.hpp`, covers the public
  `Application`/`App`/`Window`/`Context<T>` authoring vocabulary plus
  action/key, async/test Result, and platform-service Result spellings, and the
  guard fails if the example includes private headers or names
  `WindowRuntime` internals directly. Post-merge verification passed Windows
  full-debug 93/93, WSL Arch Linux example build, and WSL Arch Linux
  full-debug 90/90.
- [x] Step 308: Freeze the public authoring vocabulary before Phase C.
  Merged on `master` at `83bbe62` and post-merge verified with Windows
  full-debug 94/94, WSL Arch Linux public API example build, and WSL Arch
  Linux full-debug 91/91. It adds
  `docs/gpui-public-authoring-vocabulary.md` and
  `tests/api_parity/public_authoring_vocabulary_freeze_test.cpp`. The freeze
  guard requires the prelude include boundary, the public app/context/entity,
  action/key, async/test, Result, and platform-service names, and the explicit
  Phase B exclusions for `ClipboardItem` payload parity, upstream `gpui::test`
  macro equivalents, action macro payloads, task priorities, and structured
  task groups.
- [x] Step 309: Expand the public API compatibility examples against the
  frozen public authoring vocabulary with a second prelude-only authoring
  workflow example. Merged on `master` at `2cfb166` with
  `examples/api_parity/public_authoring_workflow/main.cpp`,
  `api_parity_public_authoring_workflow`, and
  `tests/api_parity/public_api_example_expansion_test.cpp`; post-merge
  verification passed Windows full-debug 95/95, WSL Arch Linux public API
  example builds, and WSL Arch Linux full-debug 92/92.
- [x] Step 310: Expand public API compatibility examples with a prelude-only
  public context capability workflow. Merged on `master` at `7f7838b` and
  post-merge verified with Windows full-debug 96/96 and WSL Arch Linux
  full-debug 93/93. Added
  `examples/api_parity/public_context_capabilities/main.cpp`,
  `api_parity_public_context_capabilities`, and
  `tests/api_parity/public_context_capability_example_test.cpp`; the guard
  keeps `AppContext`, `ViewContextCapability<T>`,
  `WindowContextCapability`, and `ElementContextCapability` authoring examples
  out of private headers and `WindowRuntime` internals.
- [x] Step 311: Expand public API compatibility examples with a prelude-only
  public async/test workflow. Added
  `examples/api_parity/public_async_test_workflow/main.cpp`,
  `api_parity_public_async_test_workflow`, and
  `tests/api_parity/public_async_test_workflow_example_test.cpp`; the guard
  keeps `AsyncContextCapability`, `TestContextCapability`, Result-returning
  task/frame helpers, simulated input, clipboard, and frame-pump examples out
  of private headers and `WindowRuntime` internals.
- [x] Step 312: Finish the public API compatibility example expansion queue
  against the frozen public authoring vocabulary before Phase C. Merged on
  `master` at `a9ad4dc` and post-merge verified with Windows full-debug 98/98,
  WSL Arch Linux full-debug 95/95, JSON validation, focused public/ledger
  gates, and all five public API example builds. Adds
  `examples/api_parity/public_phase_b_surface_closure/main.cpp`,
  `api_parity_public_phase_b_surface_closure`, and
  `tests/api_parity/public_phase_b_surface_closure_example_test.cpp` as the
  final prelude-only public surface closure example covering app/context,
  entity/view, action/key, async/test, Result, platform-service, frame, and
  pointer-capture public vocabulary without private headers or `WindowRuntime`
  internals.
- [x] Steps 313-318: Run full Windows/WSL verification and freeze the public
  authoring vocabulary before Phase C. Merged on `master` at `5a3e029` and
  post-merge verified with Windows JSON validation, focused public/freeze
  gates 8/8, all five public API example builds, and full debug 99/99; WSL
  Arch Linux JSON validation, focused public/freeze gates 8/8, all five public
  API example builds, and full debug 96/96. Adds
  `tests/api_parity/public_phase_b_completion_audit_test.cpp` as the final
  Phase B public vocabulary completion audit so the roadmap, public vocabulary
  document, parity ledger, public examples, public example tests, and xmake
  targets stay aligned without introducing deferred Phase B exclusions.
  Feature-worktree verification passed Windows JSON validation, focused
  public/freeze gates 8/8, all five public API example builds, and full debug
  99/99; WSL Arch Linux JSON validation, focused public/freeze gates 8/8, all
  five public API example builds, and full debug 96/96.

## Phase C: Steps 319-378 - Elements, Style, Layout, Widgets, And Uniform Lists

**Goal:** Fill the high-level declarative element system that real GPUI apps
expect, keeping each widget in its own module from the first version.

**Primary modules:**
- `include/cgpui/ui/element_*`
- `include/cgpui/ui/style_*`
- `include/cgpui/ui/*_builder.hpp`
- `src/ui/element_*`
- `src/ui/style_*`
- `src/ui/widgets/*`
- `tests/ui/element_*`
- `tests/ui/style_*`
- `tests/examples/*`

- [x] Steps 319-324: Add the upstream-style `div` element vocabulary:
  child/children handling, flex helpers, sizing, colors, borders, radius,
  shadow, text styling, overflow, and layout shortcuts.
  Step 319 adds `div` child-list ownership: repeated `.child(...)` retains
  all children, `ElementBuilder::children(...)` accepts owned collections, and
  `StyledElement::children()` exposes the focused style-node child list while
  preserving `child()` as the first-child compatibility view. Merged on
  `master` at `14aaff0` and post-merge verified with JSON validation, Windows
  full-debug 99/99, and WSL Arch Linux full-debug 96/96. Step 320 adds
  flex vocabulary aliases `items_start()`, `items_center()`, `items_end()`,
  `justify_start()`, `justify_center()`, `justify_end()`,
  `justify_between()`, and `flex_1()` over the existing align/justify and
  grow/shrink style values, with focused behavior coverage in
  `tests/ui/element_test.cpp` and structure coverage in
  `tests/architecture/ui_source_structure_test.cpp`. Merged on `master` at
  `5040365` and post-merge verified with JSON validation, Windows full-debug
  99/99, and WSL Arch Linux full-debug 96/96. Step 321 adds
  sizing/color/border aliases `w(...)`, `h(...)`, `bg(...)`,
  `text_color(...)`, `border_1()`, and `rounded(...)` over the existing style
  values, with behavior coverage in `tests/ui/element_test.cpp` and structure
  coverage in `tests/architecture/ui_source_structure_test.cpp`. Merged on
  `master` at `694d64e` and post-merge verified with JSON validation, Windows
  full-debug 99/99, and WSL Arch Linux full-debug 96/96. Step 322 adds
  overflow/opacity/position aliases `overflow(...)`, `overflow_hidden()`,
  `overflow_visible()`, `opacity(...)`, `z_index(...)`, `relative()`,
  `top(...)`, `right(...)`, `bottom(...)`, and `left(...)` over existing
  `Style` fields, with behavior coverage in `tests/ui/element_test.cpp` and
  structure coverage in `tests/architecture/ui_source_structure_test.cpp`.
  Merged on `master` at `f4f2fc2` and post-merge verified with JSON
  validation, Windows full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 323 adds text-style aliases `text_size(...)` and `font_family(...)`
  over the existing `Style::font_size` and `Style::font` fields, with behavior
  coverage in `tests/ui/element_test.cpp` and structure coverage in
  `tests/architecture/ui_source_structure_test.cpp`. Merged on `master` at
  `78f2f05` and post-merge verified with JSON validation, Windows full-debug
  99/99, and WSL Arch Linux full-debug 96/96. Step 324 added focused shadow
  vocabulary/storage through `BoxShadow`, `Style::box_shadow`,
  `StyleOverlay::box_shadow`, `ElementBuilder::shadow(...)`,
  `ElementBuilder::shadow_sm()`, and `PaintCommandKind::box_shadow`
  paint-list observability without broad style or renderer rewrites. Behavior
  coverage lives in `tests/ui/element_test.cpp` and `tests/ui/style_test.cpp`,
  with structure coverage in `tests/architecture/ui_source_structure_test.cpp`.
  It merged on `master` at `ef306c0` and was post-merge verified with JSON
  validation, Windows full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 325 adds focused layout constraint vocabulary/storage through
  `Style::min_size`, `Style::max_size`, `StyleOverlay::min_size`,
  `StyleOverlay::max_size`, `ElementBuilder::min_size(...)`,
  `ElementBuilder::max_size(...)`, `ElementBuilder::min_w(...)`,
  `ElementBuilder::min_h(...)`, `ElementBuilder::max_w(...)`, and
  `ElementBuilder::max_h(...)`; `StyledElement::layout` now merges authored
  min/max constraints with external `LayoutInput` constraints before applying
  the existing clamp path. Behavior coverage lives in
  `tests/ui/element_test.cpp` and `tests/ui/style_test.cpp`, with structure
  coverage in `tests/architecture/ui_source_structure_test.cpp`. It merged on
  `master` at `a1c945e` and was post-merge verified with JSON validation,
  Windows full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 326 adds focused percentage-like sizing through `PercentageSize`,
  `Style::percentage_size`, `StyleOverlay::percentage_size`,
  `ElementBuilder::size_pct(...)`, `ElementBuilder::w_pct(...)`,
  `ElementBuilder::h_pct(...)`, and `StyledElement::layout` resolution against
  finite external `LayoutInput` max constraints with unconstrained fallback.
  Behavior coverage lives in `tests/ui/element_test.cpp` and
  `tests/ui/style_test.cpp`, with structure coverage in
  `tests/architecture/ui_source_structure_test.cpp`. It merged on `master` at
  `7b748b7` and was post-merge verified with JSON validation, Windows
  full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 327 adds focused margin/padding shorthand vocabulary through
  `ElementBuilder::p(...)`, `ElementBuilder::px(...)`,
  `ElementBuilder::py(...)`, `ElementBuilder::pt(...)`,
  `ElementBuilder::pr(...)`, `ElementBuilder::pb(...)`,
  `ElementBuilder::pl(...)`, `ElementBuilder::m(...)`,
  `ElementBuilder::mx(...)`, `ElementBuilder::my(...)`,
  `ElementBuilder::mt(...)`, `ElementBuilder::mr(...)`,
  `ElementBuilder::mb(...)`, and `ElementBuilder::ml(...)` over existing
  `Style::padding` and `Style::margin`, while preserving existing
  `StyledElement::layout` gap composition. Behavior coverage lives in
  `tests/ui/element_test.cpp`, with structure coverage in
  `tests/architecture/ui_source_structure_test.cpp`. It merged on `master` at
  `32ec5e1` and was post-merge verified with JSON validation, Windows
  full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 328 adds focused absolute/fixed positioning through `Position::fixed`,
  `ElementBuilder::fixed()`, and out-of-flow absolute/fixed child layout in
  `StyledElement::layout` and `FlexElement::layout`. Behavior coverage lives
  in `tests/ui/element_test.cpp`, with structure coverage in
  `tests/architecture/ui_source_structure_test.cpp`. It merged on `master` at
  `38574a2` and was post-merge verified with JSON validation, Windows
  full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 329 adds focused overlay-layer ordering for direct container children:
  `src/ui/element_layer_ordering.cpp` owns stable `z_order()` sorting for
  direct-child paint, hit-test, and event dispatch through
  `paint_ordered_children(...)`, `hit_test_ordered_children(...)`, and
  `event_ordered_children(...)`; `StyledElement`, flex, and vertical stack
  containers call that private helper instead of open-coding ordering.
  Behavior coverage lives in `tests/ui/element_test.cpp`, with structure
  coverage in `tests/architecture/ui_source_structure_test.cpp`. It merged on
  `master` at `6c1bfe4` and was post-merge verified with JSON validation,
  Windows full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 330 adds focused nested scroll clipping: `src/ui/paint_clip.cpp` owns
  effective clip intersection, and `PaintList::push_clip(...)` stores the
  intersection with the current active clip so hidden-overflow parents and
  nested scrollable-list clips clamp paint command metadata together. Behavior
  coverage lives in `tests/ui/element_test.cpp`, with structure coverage in
  `tests/architecture/ui_source_structure_test.cpp`. It merged on `master` at
  `266bc9f` and was post-merge verified with JSON validation, Windows
  full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 331 adds focused active-state style cascade support: `StyleState` and
  `StyleStateFlags` now carry `active`, `resolved_style(...)` applies hover,
  focus, active, then disabled overlays for local and class rules before
  inline overlays, and `ElementBuilder::active_style(...)` plus
  `ButtonBuilder::active_style(...)` store active overlays. Behavior coverage
  lives in `tests/ui/style_test.cpp` and `tests/ui/element_test.cpp`, with
  structure coverage in `tests/architecture/ui_source_structure_test.cpp`.
  This deliberately does not wire pointer-pressed active semantics; the
  focusable/interactable band owns real active input behavior in Steps 337-342.
  Step 332 adds focused class-style reuse depth: `StyleClassRule` carries
  reused class ids plus a local `StyleState`, `StyleCascade::set_class_rule(...)`
  and `class_rule(...)` expose that rule boundary, and cascade resolution
  applies reused classes depth-first with cycle protection before each class's
  own hover/focus/active/disabled overlays. Non-template cascade bodies now
  live in `src/ui/style_cascade.cpp`; behavior coverage lives in
  `tests/ui/style_test.cpp` and `tests/ui/element_test.cpp`, with structure
  coverage in `tests/architecture/ui_source_structure_test.cpp`.
  Step 333 adds focused theme token fallback during style cascade resolution:
  `StyleThemeTokens` stores color and spacing-like token references for
  `Style` and `StyleOverlay`, theme-aware `resolved_style(...)` overloads and
  `StyledElement::resolved_style(..., Theme)` resolve those references through
  `Theme`, and missing tokens preserve existing concrete style values. Token
  lookup lives in `src/ui/style_theme_tokens.cpp`, while theme-aware cascade
  ordering lives in `src/ui/style_theme_cascade.cpp`.
  Step 334 adds focused inherited text style: `StyleAuthoredTextFields`
  distinguishes default text values from explicit authoring, private
  `src/ui/text_style_inheritance.cpp` helpers merge foreground color, font
  family, and font size, `LabelElement` / `TextElement` keep effective text
  style for layout and paint, and styled/flex/vertical-stack plus common
  wrapper elements forward inherited text style without adding runtime theme
  switching or dynamic invalidation.
  Step 335 adds focused dynamic style invalidation when runtime
  style-affecting element state changes: hover-target transitions from
  `src/ui/runtime_event_input.cpp` and keyboard-focus element transitions from
  `src/ui/runtime_focus.cpp` call
  `WindowRuntime::request_style_state_invalidation(...)`, while
  `src/ui/runtime_style_invalidation.cpp` owns the transition comparison and
  `request_render()` delegation. Behavior coverage lives in
  `tests/ui/window_runtime_input_test.cpp` and
  `tests/ui/window_runtime_focus_test.cpp`, with structure coverage in
  `tests/architecture/ui_source_structure_test.cpp`. It deliberately leaves
  runtime theme switching, real pointer-active semantics, widget behavior, and
  full resolved-style layout/paint application and broad resolved-style
  layout/paint rewrites to later focused slices. It merged on `master` at
  `e88bd78` and was post-merge verified with JSON validation, Windows
  full-debug 99/99, and WSL Arch Linux full-debug 96/96.
  Step 336 closes the Phase C style-cascade depth band with
  `tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp`, which guards
  the Steps 331-335 evidence, the explicit handoff to Step 337, and the
  deferred exclusions that belong to later focused slices. Feature-worktree
  verification passed JSON validation, focused audit gates 3/3, Windows
  full-debug 100/100, and WSL Arch Linux full-debug 97/97.
  Step 337 starts the focusable/interactable band with focused pointer-active
  input semantics: `ViewInputState::active_element_id` exposes active element
  state, `src/ui/runtime_active_state.cpp` owns left-button press/release
  tracking through `WindowRuntime::update_active_state_for_event(...)`, and
  active transitions reuse `request_style_state_invalidation(...)`. This keeps
  tab order, focus ring metadata, click/drag gestures, keyboard activation,
  widget behavior, runtime theme switching, and broad resolved-style
  layout/paint rewrites in later focused slices. Step 338 adds
  tab-order/focus-ring metadata through `FocusMetadata`,
  `ElementBuilder::tab_index(...)`, `ElementBuilder::focus_ring(...)`,
  accessibility metadata reporting, and `src/ui/runtime_focus_order.cpp`
  traversal ordering. Step 339 adds focused click/drag gesture synthesis:
  `ElementGestureKind::click` and `ElementEventContext::gesture` distinguish
  synthesized clicks from raw pointer events, `ViewInputState` records
  pointer-down/click/drag gesture metadata, `src/ui/runtime_gesture_synthesis.cpp`
  owns synthesis and dispatch helpers, runtime pointer moves suppress the later
  click as dragging, and `ClickElement` / `ButtonElement` run click handlers
  only for synthesized click gestures. Step 340 adds keyboard activation semantics:
  focused Enter/Space key presses synthesize the same
  `ElementGestureKind::click` path after raw `on_key(...)` handlers decline,
  and `KeyElement` delegates synthesized click gestures to child handlers
  instead of re-running raw key handlers. Step 341 adds disabled interaction
  semantics: `WindowRuntime::refresh_disabled_interaction_state()` in focused
  `src/ui/runtime_disabled_interaction.cpp` clears stale hover, active, focus,
  element-owned pointer capture, pointer-down, click, and drag state when the
  owning element becomes disabled or missing, restores the default cursor, and
  keeps broad widget behavior, runtime theme switching, and broad
  resolved-style layout/paint rewrites out. Step 342 closes the focusable/interactable band through
  `tests/api_parity/phase_c_focusable_interactable_audit_test.cpp`, guarding
  the Steps 337-341 evidence and the explicit Step 343 built-in widget
  expansion handoff while keeping broad widget behavior, runtime theme
  switching, and broad resolved-style layout/paint rewrites out.
- [x] Steps 325-330: Complete layout behavior beyond the current primitives:
  min/max constraints, percentage-like sizing, margins, padding, gaps,
  absolute/fixed positioning, overlay layers, and nested scroll clipping.
- [x] Steps 331-336: Add style cascade depth: pseudo/state selectors,
  class-like reuse, theme token fallback, inherited text style, and dynamic
  invalidation when style-affecting state changes.
- [x] Steps 337-342: Complete focusable/interactable element semantics:
  hover, active, disabled, tab order, focus ring metadata, pointer capture,
  click/drag gestures, and keyboard activation.
- [x] Steps 343-348: Expand built-in widgets: button, label, text input,
  checkbox, radio, switch, slider, list item, menu item, icon/image, and
  container primitives. Step 343 starts this band with focused button, label,
  and text-input builder module ownership: `ButtonBuilder::label(...)` composes
  a label child, `src/ui/widgets/button_builder.cpp`,
  `src/ui/widgets/label_builder.cpp`, and
  `src/ui/widgets/text_input_builder.cpp` own widget builder implementation
  bodies, `xmake.lua` compiles `src/ui/widgets/*.cpp`, and
  `tests/ui/builtin_widget_test.cpp` plus
  `tests/architecture/widget_source_structure_test.cpp` guard the public
  behavior and source boundary. Step 344 adds checkbox, radio, and switch
  widget authoring through `ToggleBuilder`, `checkbox(...)`, `radio(...)`,
  and `toggle_switch(...)`, focused `src/ui/widgets/toggle_builder.cpp`
  ownership, focused `ToggleControlElement` element behavior split across
  `src/ui/element_choice_nodes.cpp`, `src/ui/element_choice_layout.cpp`, and
  `src/ui/element_choice_paint.cpp`, and accessibility role/value metadata for
  checked/selected/on state. Step 345 adds slider widget authoring through
  `SliderBuilder`, `slider(...)`, focused `src/ui/widgets/slider_builder.cpp`
  ownership, focused `SliderElement` element behavior split across
  `src/ui/element_slider_nodes.cpp`, `src/ui/element_slider_layout.cpp`, and
  `src/ui/element_slider_paint.cpp`, and accessibility role/value metadata for
  the current slider value. Step 346 adds list item and menu item widget
  authoring through `ItemBuilder`, `list_item(...)`, `menu_item(...)`, focused
  `src/ui/widgets/item_builder.cpp` ownership, focused `ItemElement` behavior
  split across `src/ui/element_item_nodes.cpp`,
  `src/ui/element_item_layout.cpp`, and `src/ui/element_item_paint.cpp`, and
  accessibility role/value metadata for selected list items and menu actions
  before icon/image and container primitive gaps. Step 347 adds icon/image
  widget authoring through `ImageBuilder`, `image(...)`, `icon(...)`, focused
  `src/ui/widgets/image_builder.cpp` ownership, focused `ImageElement`
  behavior split across `src/ui/element_image_nodes.cpp`,
  `src/ui/element_image_layout.cpp`, and `src/ui/element_image_paint.cpp`, and
  source-rect, alternate-text, and optional icon tint metadata before
  container primitive gaps. Step 348 closes this band by moving container
  primitive free-function authoring into
  `include/cgpui/ui/container_builder.hpp` and
  `src/ui/widgets/container_builder.cpp`, exposing `div()`, `h_flex()`,
  `v_flex()`, `h_stack()`, and `v_stack()` through the focused widget builder
  aggregate and guarding the module boundary with
  `tests/architecture/widget_source_structure_test.cpp`.
- [x] Steps 349-354: Implement uniform list parity: stable item identity,
  virtualized range calculation, scroll anchoring, item measurement cache,
  large-list recycling, and keyboard/pointer selection. Step 349 starts this
  band with stable item identity and the first virtualized visible-range
  calculation boundary over the existing `scrollable_list` path. Step 350 adds
  keyed scroll anchoring over that snapshot boundary. Step 351 adds keyed item
  measurement caching over the same layout snapshot boundary. Step 352 adds
  retained/recycled large-list windows with focused paint skipping outside the
  visible-plus-overscan range. Step 353 adds keyboard/pointer selection over
  the same snapshot boundary. Step 354 closes the uniform-list band with audit
  evidence before the window/examples widget band starts.
- [x] Steps 355-360: Implement window/examples widgets for menu demos, shadow,
  window positioning, window shadow, and input examples using public APIs.
  Phase C Step 355 window/examples widgets starts this band by adding
  `examples/api_parity/public_window_examples/main.cpp`,
  `api_parity_public_window_examples`, and
  `tests/api_parity/phase_c_window_examples_public_api_test.cpp`. The example
  covers menu demos, shadow, window positioning, window shadow, and input
  examples using public APIs and remains prelude-only. Phase C Step 356 window/examples workflow deepens the window/examples widget band with
  `examples/api_parity/public_window_examples_workflow/main.cpp`,
  `api_parity_public_window_examples_workflow`, and
  `tests/api_parity/phase_c_window_examples_workflow_test.cpp`, using a public
  test-context workflow for menu installation, window activation/focus, key
  binding simulation, pointer dispatch, text input, and shadow/fixed positioning
  examples. This is the public test-context workflow checkpoint for the
  window/examples band. Phase C Step 357 window/examples widget catalog adds
  `examples/api_parity/public_window_examples_widget_catalog/main.cpp`,
  `api_parity_public_window_examples_widget_catalog`, and
  `tests/api_parity/phase_c_window_examples_widget_catalog_test.cpp` for a
  prelude-only widget catalog covering checkbox/radio/switch, slider,
  list/menu, image/icon, and container widgets inside the public window example
  band. Step 358 should continue the window/examples widget band without
  introducing private runtime headers. Phase C Step 358 window/examples interaction states adds
  `examples/api_parity/public_window_examples_interaction_states/main.cpp`,
  `api_parity_public_window_examples_interaction_states`, and
  `tests/api_parity/phase_c_window_examples_interaction_states_test.cpp` for
  hover/focus/active/disabled interaction states, focus ring and tab-index examples,
  keyed controls, and click handlers inside a prelude-only public
  window example. Phase C Step 359 window/examples service matrix adds
  `examples/api_parity/public_window_examples_service_matrix/main.cpp`,
  `api_parity_public_window_examples_service_matrix`, and
  `tests/api_parity/phase_c_window_examples_service_matrix_test.cpp` for
  menu accelerators and command palette service matrix plus window options, shadow, fixed positioning, and text input service examples.
  Phase C Step 360 window/examples closeout closes the window/examples widget band through
  `tests/api_parity/phase_c_window_examples_closeout_test.cpp`, which guards
  the Steps 355-359 public example evidence, keeps the examples prelude-only,
  and hands off to Step 361. Step 361 starts SVG/image element front-end APIs.
- [x] Steps 361-366: Add SVG/image element front-end APIs that feed the asset
  pipeline without adding renderer details to public element headers.
  Phase C Step 361 SVG/image front-end source APIs adds the public
  `ImageSource`, `ImageSourceKind`, `image_source(...)`,
  `svg_image_source(...)`, and `svg(...)` authoring boundary, with
  `ImageElement` preserving SVG source metadata while paint still feeds the
  existing image descriptor pipeline. Phase C Step 362 SVG/image asset
  registration adds `ImageAssetRegistry`, `RegisteredImageAsset`,
  `register_image(...)`, and `register_svg(...)` as the deterministic public
  registration boundary over `ImageSource`, without SVG decoding, renderer
  upload, or GPU lifetime behavior. Phase C Step 363 SVG/image public example coverage adds `examples/api_parity/public_svg_image_sources/main.cpp`,
  `api_parity_public_svg_image_sources`, and
  `tests/api_parity/phase_c_svg_image_public_examples_test.cpp` as a
  prelude-only registered raster/SVG source example. It feeds
  `RegisteredImageAsset::source()` into `image(...)` and `svg(...)` without
  exposing decoder, upload, or runtime internals. Phase C Step 364 closes the SVG/image band through
  `tests/api_parity/phase_c_svg_image_closeout_test.cpp`, which guards the
  Steps 361-363 public source, asset registry, and registered-source example
  evidence. The closeout keeps SVG decoding, PNG/JPEG loading, renderer upload, GPU texture lifetime, private runtime headers, and direct
  `WindowRuntime` use out of this band. Step 367 starts widget family structure tests.
- [x] Steps 367-372: Add structure tests requiring every widget family to have
  a public leaf header, a focused source file, and focused behavior tests.
  Phase C Step 367 widget family structure tests extend
  `tests/architecture/widget_source_structure_test.cpp` with a
  `WidgetFamilyBoundary` table for label, button, text input, toggle controls,
  slider, list/menu items, image/icon/SVG, container primitives, and
  scrollable list families. Each row names its public leaf header, focused
  source file, and focused behavior tests; the new
  `tests/api_parity/phase_c_widget_family_structure_test.cpp` keeps the
  roadmap, ledger, and public vocabulary handoff aligned. Phase C Step 373
  final element/style/widget ledger audit is next.
- [x] Steps 373-378: Run full Windows/WSL verification and update the ledger
  so element/style/widget rows are either complete or explicitly deferred.
  Phase C Step 373 final element/style/widget ledger audit closes Phase C with
  `tests/api_parity/phase_c_final_ledger_audit_test.cpp`, which checks the
  already-landed closeout evidence from
  `tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp`,
  `tests/api_parity/phase_c_focusable_interactable_audit_test.cpp`,
  `tests/api_parity/phase_c_uniform_list_audit_test.cpp`,
  `tests/api_parity/phase_c_window_examples_closeout_test.cpp`,
  `tests/api_parity/phase_c_svg_image_closeout_test.cpp`, and
  `tests/api_parity/phase_c_widget_family_structure_test.cpp`. The audit keeps
  `gpui::div` and `gpui uniform_list` adapted/closed, leaves image/SVG
  production decoding/loading/upload/lifetime work explicitly deferred, and
  does not add `ClipboardItem`, upstream `gpui::test` macros, action macro
  payloads, task priorities, structured task groups, private runtime headers,
  or direct `WindowRuntime` use. Phase D Step 379 text/font shaping follows.

## Phase D: Steps 379-458 - Text, Font, Editing, IME, And Rich Text

**Goal:** Move from deterministic text skeletons to production-grade text
behavior that can support GPUI examples and editor-like widgets.

**Primary modules:**
- `include/cgpui/ui/text_*`
- `src/ui/text_*`
- `src/platform/win32/*text*`
- `src/platform/linux/*text*`
- `src/platform/macos/*text*` after Phase H opens
- `tests/ui/text_*`
- `tests/platform/*text*`

- [x] Steps 379-386: Replace fallback-only shaping with HarfBuzz-backed
  shaping on Windows/Linux while preserving deterministic test fallbacks.
  Step 379 starts this band with an explicit text-shaping backend boundary:
  `TextShapingBackend`, `TextShapingOptions`, capability reporting, fallback
  reason metadata, and glyph ids are recorded on `TextShapeRun`, while the
  current Windows/WSL hosts continue to use deterministic fallback because
  HarfBuzz is not installed yet.
  Step 380 splits shaping dispatch and deterministic fallback shaping into
  focused internal backend files so a later HarfBuzz backend can plug into the
  same path without changing public text authoring APIs.
  Step 381 preserves shaped glyph ids through wrapped text paint metadata so
  atlas keys remain stable after soft wrapping.
  Step 382 adds shaped glyph positioning offsets and applies them in wrapped
  and unwrapped glyph paint metadata; deterministic fallback offsets stay zero.
  Step 383 adds direction/script/language shaping metadata to options and
  shape runs so the future HarfBuzz backend has explicit input records.
  Step 384 adds a guarded HarfBuzz backend source boundary and dispatch
  insertion point while preserving deterministic fallback when the real
  HarfBuzz backend is not compiled in.
  Step 385 adds lightweight backend diagnostic snapshots on shaping
  selections and shape runs so unavailable/fallback paths are observable
  without string diagnostics or hot-path allocation.
  Step 386 adds this text-shaping readiness audit to freeze the backend
  boundary, fallback diagnostics, and explicit production-HarfBuzz gap before
  later dependency-backed shaping work.
  Phase D guarded HarfBuzz backend now shapes through hb_shape in
  `src/ui/text_shaping_harfbuzz.cpp` when
  `CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND` is enabled, using file-backed font faces when available and deterministic fallback on shaping failure.
  `tests/api_parity/phase_d_harfbuzz_backend_audit_test.cpp` freezes the
  source boundary so the guarded backend is no longer a `#error` insertion point.
  DirectWrite font-file extraction remains later work, along with native ZWJ
  ligature shaping depth, full Unicode script data, bidirectional shaping, and
  paragraph shaping.
- [x] Steps 387-394: Add real font discovery and fallback: DirectWrite on
  Windows, fontconfig/FreeType on Linux, and later CoreText on macOS.
  Step 387 starts this band by moving `FontDatabase`, `FontFallbackChain`, and
  deterministic discovery helper bodies out of `text_font.hpp` into
  `src/ui/text_font.cpp`, compiled through `cgpui_platform` because platform
  applications produce discovered font records, giving native
  DirectWrite/fontconfig adapters a focused font boundary to feed later.
  Step 388 adds `PlatformFontDiscoveryResult`, backend/status diagnostics,
  `PlatformApplication::discover_font_discovery()`, and focused
  `win32_font_discovery.cpp` / `wayland_font_discovery.cpp` boundaries so the
  current deterministic platform fallback records are observable without
  claiming real DirectWrite/fontconfig enumeration yet.
  Step 389 promotes the Win32 side to real DirectWrite system font-family
  enumeration with native-available diagnostics and keeps the Segoe UI
  deterministic fallback only for DirectWrite failures; Linux
  fontconfig/FreeType enumeration remains pending.
  Step 390 adds the guarded Linux fontconfig backend insertion point through
  `wayland_fontconfig_discovery.cpp`, `wayland_font_discovery_internal.hpp`,
  and `wayland_font_discovery_test/default`. The current WSL host has neither
  fontconfig/FreeType pkg-config metadata nor visible headers/runtime
  libraries, so default builds still report deterministic `sans-serif`
  fallback; dependency-backed native Linux enumeration remains pending.
  Step 391 adds coverage-aware font fallback records through
  `FontUnicodeRange`, `font_face_declares_coverage(...)`,
  `font_face_covers_codepoint(...)`, and
  `FontDatabase::resolve_chain_for_codepoint(...)`, so a later shaping run can
  choose a fallback chain for a specific Unicode codepoint while retaining the
  existing ordered fallback behavior when coverage metadata is absent.
  Step 392 threads that explicit chain into shaping with a
  `shape_text(..., FontFallbackChain, ...)` overload and
  `TextShapeRun::font_fallback_faces` diagnostics, keeping fallback selection
  caller-controlled instead of adding hidden global font lookup to the shaping
  hot path.
  Step 393 adds `TextGlyphRun::font_fallback_face_index` and deterministic
  UTF-8 codepoint-to-face selection over the explicit fallback chain, so glyph
  records can identify which fallback face covers each codepoint before later
  multi-font run splitting lands.
  Step 394 closes the font discovery/fallback band with an audit-only guard
  over Steps 387-393, preserving the explicit remaining gaps for
  dependency-backed Linux fontconfig/FreeType enumeration and production
  HarfBuzz shaping before the next Phase D fallback-splitting band starts.
  Phase D font discovery now has system-optional fontconfig package wiring in
  `xmake.lua`: when the system `fontconfig` package is available, the Wayland
  platform target links it and defines `CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND`,
  enabling the existing `FcFontList` native enumeration path; missing
  fontconfig keeps the deterministic fallback path. FreeType metrics extraction
  and richer per-face coverage remain later work.
- [x] Steps 395-402: Add per-script and per-codepoint fallback splitting,
  font coverage checks, emoji/color glyph planning, and missing-glyph
  diagnostics.
  Step 395 starts this band by adding contiguous fallback font spans through
  `TextFontFallbackRun` and `TextShapeRun::font_runs`, so deterministic
  shaping exposes byte/glyph/advance ranges for each selected fallback face
  before renderer-side font switching, emoji/color glyph planning, and
  missing-glyph diagnostics land.
  Step 396 adds explicit missing-glyph diagnostics through
  `TextMissingGlyphDiagnostic` and `TextShapeRun::missing_glyphs` when every
  face in the explicit fallback chain declares coverage and none covers the
  codepoint. Unknown platform coverage remains non-diagnostic until native
  coverage extraction lands.
  Step 397 adds emoji-plane color glyph planning through `TextColorGlyphPlan`
  and `TextShapeRun::color_glyphs`, recording byte/glyph/codepoint/fallback
  face metadata for default emoji-plane codepoints while leaving real color
  glyph rendering to later focused steps.
  Step 398 adds emoji presentation selector planning for `U+FE0F`, associating
  the selector with the immediately preceding emoji-capable base glyph for
  color glyph metadata while suppressing false missing-glyph diagnostics for
  the selector itself. Full Unicode emoji data, HarfBuzz variation shaping, and
  native color glyph rendering remain later work.
  Step 399 extends `TextColorGlyphPlan` with selector-span metadata through
  `has_emoji_presentation_selector`,
  `emoji_presentation_selector_byte_offset`, and
  `emoji_presentation_selector_byte_length`, so both BMP symbol emoji
  presentation and emoji-plane codepoints followed by `U+FE0F` preserve the
  selector bytes for future native shaping/rendering.
  Step 400 suppresses false missing-glyph diagnostics for `U+200D` only when
  it is acting as an emoji ZWJ sequence joiner between emoji-capable
  codepoints, keeping deterministic glyph records and per-codepoint color
  plans without claiming native ZWJ ligature shaping.
  Step 401 adds deterministic script-run metadata through `TextScriptRun` and
  `TextShapeRun::script_runs`, coalescing contiguous fallback glyphs into
  lightweight latin/han/emoji/etc. spans for later HarfBuzz itemization without
  changing run-level shaping script semantics.
  Step 402 closes the fallback metadata band through
  `tests/api_parity/phase_d_fallback_splitting_audit_test.cpp`, freezing the
  Step 395-401 font-run, missing-glyph, color-glyph, emoji selector, ZWJ, and
  script-run metadata evidence before text measurement and wrapping work
  starts.
- [x] Steps 403-410: Complete text measurement and wrapping: grapheme columns,
  soft wraps, hard wraps, bidirectional text planning, line boxes, baseline,
  ascent/descent, and paragraph caches.
  Step 403 starts text measurement and wrapping by adding
  `TextGraphemeColumn` and `TextMeasurement::grapheme_columns`, so measured
  fallback text records deterministic grapheme columns for combining marks,
  variation selectors, and regional-indicator pairs without claiming full
  bidirectional layout or paragraph shaping in the first slice.
  Step 404 makes soft wrapping respect those columns through
  `TextWrapLine::column_start`, `TextWrapLine::column_end`,
  `text_wrap_line_for_column_range(...)`, and `wrap_text_measurement(...)`,
  keeping measured grapheme columns atomic while preserving byte/glyph ranges
  for existing wrapped paint metadata.
  Step 405 adds hard-wrap line records through `TextWrapBreakKind`,
  `TextWrapLine::break_kind`, and `text_wrap_column_is_hard_break(...)`, so
  explicit newline columns split wrapped layout and stay out of glyph paint
  ranges while soft and hard line endings remain distinguishable.
  Step 406 adds deterministic bidirectional planning metadata through
  `TextBidiRun`, `TextMeasurement::base_direction`,
  `TextMeasurement::bidi_runs`, `TextWrapLine::bidi_run_start`,
  `TextWrapLine::bidi_run_end`, `TextWrapLayout::base_direction`,
  `build_text_bidi_runs(...)`, and `classify_text_bidi_direction(...)`,
  recording LTR/RTL spans over measured grapheme columns without claiming the
  full Unicode bidi algorithm, visual reordering, paragraph layout, or native
  HarfBuzz itemization.
  Step 407 adds deterministic line metrics and line-box metadata through
  `TextLineMetrics`, `TextMeasurement::line_metrics`,
  `TextWrapLine::metrics`, `text_line_metrics_for_shape_run(...)`, and
  `src/ui/text_line_metrics.cpp`, exposing baseline/ascent/descent/leading
  values to measurement and wrapped line records without claiming
  platform-derived font metrics, paragraph line boxes, or native shaping
  itemization.
  Step 408 adds an explicit paragraph layout cache boundary through
  `TextParagraphLayout`, `TextParagraphLayoutResult`,
  `TextParagraphLayoutCache`, `layout_text_paragraph(...)`, and
  `src/ui/text_paragraph_layout.cpp`, caching measurement plus wrapped layout
  by text/font/scale/max-width only when callers opt into the cache object and
  leaving richer paragraph shaping, eviction policy, and platform text metrics
  for later work.
  Step 409 normalizes CRLF hard-wrap records by keeping `\r\n` as one measured
  grapheme column through `text_measurement_is_crlf_pair(...)`, treating both
  `\n` and `\r` columns as hard breaks in `text_wrap_column_is_hard_break(...)`,
  and keeping wrapped glyph paint metadata from painting either CRLF glyph.
  Full Unicode line-break classes and paragraph shaping remain later work.
  Step 410 closes the text measurement/wrapping band through
  `tests/api_parity/phase_d_text_measurement_wrapping_audit_test.cpp`,
  freezing the Step 403-409 grapheme, wrap, hard break, bidi planning, line
  metrics, paragraph cache, and CRLF hard-wrap evidence before selection and
  caret behavior starts.
- [x] Steps 411-418: Complete selection and caret behavior: mouse drag,
  double/triple click, word/line selection, scroll-to-caret, preferred column,
  selection painting, and clipboard integration.
  Step 411 starts this band with explicit text selection drag records:
  `TextSelectionDragDirection`, `TextSelectionDrag`,
  `text_selection_drag_from_offsets(...)`, and
  `text_selection_drag_from_points(...)` live in the focused text hit-testing
  leaf, and runtime pointer selection now reuses the same helper instead of
  open-coding anchor/head selection ranges.
  Step 412 adds word-selection range helpers through
  `TextModel::word_selection_range_at(...)`, reusing the existing grapheme and
  word-boundary navigation path to expose zero-allocation double-click-ready
  ranges while separators and end offsets stay collapsed. Runtime
  double/triple-click gesture synthesis remains later in this band.
  Step 413 adds line-selection range helpers through
  `TextModel::line_selection_range_at(...)`, reusing existing line navigation
  while trimming CRLF line endings from selected ranges so triple-click-ready
  model ranges stay explicit and zero-allocation.
  Step 414 adds multi-click selection granularity plumbing:
  `PointerButton::click_count` defaults to single-click behavior, while
  `TextSelectionGranularity` and
  `text_selection_granularity_for_click_count(...)` map single, double, and
  triple-or-higher clicks to caret, word, and line selection without runtime
  allocation.
  Step 415 connects double-click runtime word selection: text input pointer
  down consumes `PointerButton::click_count == 2`, applies
  `TextModel::word_selection_range_at(...)`, and keeps pointer release from
  collapsing that word selection back into the ordinary drag path.
  Step 416 connects triple-click runtime line selection through the same
  granularity path, applying `TextModel::line_selection_range_at(...)` for
  `click_count >= 3` and preserving the selected line on pointer release.
  Step 417 deepens selection painting by moving text selection/caret geometry
  into focused internal `src/ui/text_selection_paint_geometry.cpp` /
  `.hpp` helpers. `TextElement::paint(...)` now emits per-line selection paint
  commands for multiline ranges and positions the caret from measured hard-wrap
  line metrics instead of first-line byte-offset geometry, while
  `tests/ui/element_test.cpp` and `tests/architecture/ui_source_structure_test.cpp`
  guard the behavior and module boundary.
  Step 418 closes the selection/caret band through preferred-column vertical
  navigation in `TextModel`, explicit `ScrollModel::scroll_rect_into_view(...)`
  and `TextElement::scroll_caret_into_view(...)` scroll-to-caret primitives,
  shared `text_caret_rect(...)` geometry for paint and IME candidate placement,
  runtime route scrolling for `ScrollableListElement`, existing focused text
  copy/cut/paste clipboard coverage, and
  `tests/api_parity/phase_d_selection_caret_audit_test.cpp`. Steps 419-426 close
  edit history with grouped typing, IME composition commit grouping,
  undo-manager status integration, redo invalidation diagnostics, and
  edit transaction diagnostics; richer IME platform behavior, rich text, text
  examples, and final Phase D verification remain open in Steps 427-458.
- [x] Steps 419-426: Deepen edit history: grouped typing, IME grouped commits,
  undo manager integration points, redo invalidation, and edit transaction
  diagnostics.
  Step 419 starts this band with adjacent typing history coalescing in
  `TextModel`: `TextInsertHistoryPolicy::merge_adjacent_typing` merges
  uninterrupted typing into one undo/redo record, `separate_edit` keeps paste
  independent of typing, and navigation, selection, delete, undo/redo, and
  composition state changes explicitly break the grouping.
  Step 420 adds a focused IME composition history group in `TextModel`:
  delete-surrounding mutations made during an active composition share the
  composition-start snapshot, and commit/cancel records one
  `TextInsertHistoryPolicy::composition_commit` undo transaction without moving
  this model-level behavior into platform event files.
  Step 421 adds undo-manager integration points without a heavyweight manager:
  `TextEditHistoryStatus`, `TextModel::edit_history_status()`,
  `TextModel::edit_history_clean()`, and `TextModel::mark_edit_history_clean()`
  expose can-undo/can-redo, undo/redo depths, clean state, and revision; marking
  clean also closes the active typing merge group so a saved state remains an
  undo-visible boundary.
  Step 422 adds redo invalidation diagnostics through
  `TextEditHistoryRedoInvalidationReason`,
  `TextEditHistoryRedoInvalidation`,
  `TextEditHistoryStatus::last_redo_invalidation`, and
  `TextModel::invalidate_redo_history(...)`, so branch edits report why and how
  much redo history was discarded without exposing or copying the stacks.
  Step 423 adds last-transaction diagnostics through
  `TextEditHistoryTransactionKind`,
  `TextEditHistoryTransactionDiagnostic`,
  `TextEditHistoryStatus::last_transaction`, and
  `TextModel::record_edit_history_transaction(...)`, so committed records,
  adjacent typing merges, undo, redo, and clean marks expose depth deltas and
  revision without copying edit snapshots.
  Step 424 closes the edit-history band through
  `tests/api_parity/phase_d_edit_history_audit_test.cpp`, freezing the grouped
  typing, composition commit grouping, undo-manager status, redo invalidation,
  and edit transaction diagnostics evidence before active-target IME platform
  work starts.
- [x] Steps 427-434: Complete IME on active targets: Win32 TSF/IMM depth,
  Wayland text-input v3 surrounding text, delete-surrounding, content hints,
  serial policy, preedit styling, and candidate placement.
  Step 427 starts this band by extending `ImeTextInputPlacement` with explicit
  surrounding text, selection anchor, and content hint/purpose metadata.
  Runtime focused-text placement now fills those fields from the focused
  `TextModel`, Wayland text-input v3 submits them through
  `set_surrounding_text` / `set_content_type`, and Win32 IMM keeps the same
  placement state surface while preserving candidate/composition rectangle
  placement.
  Step 428 propagates Wayland text-input v3 `done(serial)` through the focused
  event path: `ImeComposition` and `ImeDeleteSurroundingText` carry the serial,
  `wayland_text_input_events.cpp` forwards it through the Wayland window bridge,
  and `tests/platform/wayland_keyboard_test.cpp` verifies preedit,
  delete-surrounding, and commit serials. Richer Wayland policy, preedit
  styling, candidate placement, and Win32 TSF depth remain open in this band.
  Step 429 preserves Wayland `preedit_string` cursor metadata as scalar
  `ImeComposition::preedit_cursor_begin` / `preedit_cursor_end` fields,
  forwarding `cursor_begin` / `cursor_end` through `WaylandTextInput`,
  `wayland_window_text_input_preedit(...)`, and the Wayland keyboard fixture
  without adding preedit styling or candidate-placement policy.
  Step 430 adds fixed-capacity preedit style metadata through
  `ImePreeditStyleSpan`, `kImePreeditStyleSpanCapacity`,
  `append_ime_preedit_style(...)`, and
  `append_ime_default_preedit_style(...)` in the core text event boundary.
  Wayland preedit updates attach one default underline span without vector
  allocation or `TextModel` behavior changes; platform-specific styling and
  candidate-placement policy remain open.
  Step 431 makes candidate placement explicit by adding
  `ImeTextInputPlacement::candidate_rect`. Runtime focused-text placement fills
  it from `ImeCandidateRect`, Wayland text-input v3 prefers it for
  `set_cursor_rectangle`, and Win32 IMM applies it to `CANDIDATEFORM` while
  keeping `rect` for `COMPOSITIONFORM`. Production candidate UI policy remains
  later work.
  Step 432 adds Win32 IMM composition/result string ingestion: `WM_IME_COMPOSITION`
  dispatches from the lifecycle message proc into `win32_window_ime.cpp`,
  `GCS_COMPSTR` is converted to `ImeCompositionPhase::update`,
  `GCS_RESULTSTR` is converted to `ImeCompositionPhase::commit`, and
  `WM_IME_ENDCOMPOSITION` emits a cancel event. TSF integration and richer
  platform-specific composition styling remain later work.
  Step 433 adds Wayland text-input v3 stale serial policy: `WaylandTextInput`
  tracks the last accepted `done(serial)`, drops non-increasing serials, clears
  pending preedit/delete/commit state when stale or inactive, and
  `wayland_keyboard_test` verifies a stale commit is ignored before later
  increasing serials are accepted.
  Step 434 closes the active-target IME platform band through
  `tests/api_parity/phase_d_ime_platform_audit_test.cpp`, freezing the
  surrounding text/content hint, serial propagation/stale policy, preedit
  cursor/style, candidate-placement metadata, Win32 IMM composition/result
  ingestion, and remaining TSF/candidate UI gaps before Phase D moves to rich
  text runs.
- [x] Steps 435-442: Add rich text runs: spans, links, inline images, syntax
  color-like attributes, underline/strikethrough, background, and hit testing.
  Step 435 starts this band with the focused rich-text run core:
  `include/cgpui/ui/text_rich_text.hpp` and `src/ui/text_rich_text.cpp` define
  `RichTextSpan`, `RichTextRun`, `RichTextAttributes`, decoration metadata,
  numeric link ids, and `build_rich_text_runs(...)` overloads that normalize,
  clip, merge, and coalesce caller-provided spans into explicit byte ranges.
  The caller-owned output/scratch overload keeps allocation visible and
  reusable, while `tests/ui/rich_text_run_test.cpp` freezes plain-text fallback,
  overlapping attribute merge, out-of-range span clipping, empty span dropping,
  and adjacent equal-run coalescing.
  Step 436 adds the first link/run interaction surface:
  `RichTextRunHit`, `RichTextLinkHit`,
  `rich_text_run_at_byte_offset(...)`, and
  `rich_text_link_at_byte_offset(...)` expose half-open byte-range hit metadata
  for normalized runs without renderer state or string link targets.
  `rich_text_run_test` verifies boundary selection, missing-link misses, link
  id propagation, and end-exclusive behavior. Inline images, rich-text painting,
  point-based hit testing, and click activation wiring remain later steps in
  this band.
  Step 437 closes the point-based hit metadata slice with
  `include/cgpui/ui/text_rich_text_hit_testing.hpp` and
  `src/ui/text_rich_text_hit_testing.cpp`: `RichTextRunPointHit`,
  `RichTextLinkPointHit`, `rich_text_run_at_point(...)`, and
  `rich_text_link_at_point(...)` compose the existing text hit-test byte offset
  with normalized run/link metadata. This keeps link detection out of runtime
  and renderer state and still leaves click activation, inline images,
  syntax-theme integration, and rich-text paint integration for later steps.
  Step 438 adds focused inline image metadata via
  `include/cgpui/ui/text_rich_text_inline_image.hpp` and
  `src/ui/text_rich_text_inline_image.cpp`: `RichTextInlineImageSpan`,
  `RichTextInlineImageRun`, and `build_rich_text_inline_image_runs(...)`
  normalize numeric `ImageAssetId` anchors with byte-span clipping,
  zero-length insertion anchors, deterministic sorting, logical size, and
  baseline offset. Image loading, rich-text paint integration, click
  activation, and syntax-theme integration remain later steps.
  Step 439 adds syntax-theme metadata via
  `include/cgpui/ui/text_rich_text_syntax.hpp` and
  `src/ui/text_rich_text_syntax.cpp`: `RichTextSyntaxRole`,
  `RichTextSyntaxToken`, `RichTextSyntaxTheme`,
  `rich_text_syntax_attributes_for_role(...)`, and
  `build_rich_text_syntax_spans(...)` convert caller-owned syntax token ranges
  into deterministic `RichTextSpan` records without maps, parser ownership,
  runtime state, or renderer state. Syntax parsing, editor token integration,
  rich-text paint integration, click activation, and inline image painting
  remain later steps.
  Step 440 adds rich-text paint metadata integration without changing renderer
  drawing semantics: `PaintList::fill_rich_text(...)` lives in focused
  `src/ui/paint_rich_text.cpp`, `TextPaint` and `TextDraw` now carry
  `rich_text_runs` plus `rich_text_inline_images`, and
  `src/ui/render_view_commands.cpp` preserves those records into the render
  frame. `tests/ui/render_view_test.cpp` verifies metadata retention, while
  `tests/architecture/ui_source_structure_test.cpp` keeps the rich-text paint
  entry out of broad paint/runtime files. Actual multi-color glyph painting,
  inline image drawing/loading, click activation, syntax parsing, and editor
  token source integration remain later work.
  Step 441 adds focused link activation metadata through
  `include/cgpui/ui/text_rich_text_activation.hpp` and
  `src/ui/text_rich_text_activation.cpp`: `RichTextLinkActivation`,
  `rich_text_pointer_button_can_activate_link(...)`, and
  `rich_text_link_activation_at_point(...)` convert primary single-button
  release hits into numeric link activation records by composing the existing
  point-hit helper. This keeps activation detection out of runtime/renderer
  state and string link targets. Runtime element dispatch, actual link command
  handling, inline image drawing/loading, syntax parsing, and editor token
  source integration remain later work.
  Step 442 closes the rich-text metadata band through
  `tests/api_parity/phase_d_rich_text_audit_test.cpp`, freezing the evidence
  for rich-text run normalization, byte and point hit metadata, inline image
  metadata, syntax-theme metadata, paint metadata transport, link activation
  metadata, focused source ownership, and the zero-cost boundary that keeps
  runtime dispatch, renderer drawing, parser ownership, editor token sourcing,
  and string link targets out of the metadata layer. Runtime rich-text element
  dispatch, actual link command handling, inline image drawing/loading, syntax
  parsing/editor token source integration, and actual multi-color glyph
  painting remain later Phase D work.
- [x] Steps 443-450: Add text input parity examples and API compatibility
  tests for the official input and text wrapper examples.
  Phase D Step 443 text input public examples start this slice. Step 443 starts the text-input public examples band with
  `examples/api_parity/public_text_input_examples/main.cpp` and
  `api_parity_public_text_input_examples`, demonstrating TextModel public editing,
  selection ranges, edit-history status, text-input builder use, IME placement and composition metadata, delete-surrounding metadata, command
  palette entries, and key bindings through `cgpui/prelude.hpp` only.
  `tests/api_parity/phase_d_text_input_public_examples_test.cpp` freezes the
  example and keeps it away from private runtime APIs. Text wrapper examples,
  richer official input workflows, rich-text examples, and final Phase D
  verification remain later steps.
  Phase D Step 444 text wrapper public examples extend this slice. Step 444 extends the text-input examples band with
  `examples/api_parity/public_text_wrapper_examples/main.cpp` and
  `api_parity_public_text_wrapper_examples`, demonstrating label and TextElement wrappers,
  TextModel-backed public text elements, measurement/wrapping/glyph paint metadata,
  TextMeasurementCache use, layout sizing, and accessibility text through
  `cgpui/prelude.hpp` only.
  `tests/api_parity/phase_d_text_wrapper_public_examples_test.cpp` freezes the
  example and keeps it away from private runtime APIs. Richer official input
  workflows, rich-text examples, and final Phase D verification remain later
  steps.
  Phase D Step 445 text input workflow public example extends this slice.
  Step 445 extends the text-input examples band with
  `examples/api_parity/public_text_input_workflow/main.cpp` and
  `api_parity_public_text_input_workflow`, demonstrating undo/redo and redo invalidation,
  edit-history clean markers, line/word navigation and composition cancellation,
  selection text, backspace/delete-forward, TextEditAction dispatch, and history
  diagnostics through `cgpui/prelude.hpp` only.
  `tests/api_parity/phase_d_text_input_workflow_examples_test.cpp` freezes the
  example and keeps it away from private runtime APIs. Rich-text examples, final
  examples closeout, and final Phase D verification remain later steps.
  Phase D Step 446 rich text public example extends this slice. Step 446 extends the text-input examples band with
  `examples/api_parity/public_rich_text_examples/main.cpp` and
  `api_parity_public_rich_text_examples`, demonstrating rich-text run, syntax, inline-image, hit, activation, and paint metadata
  through `cgpui/prelude.hpp` only.
  `tests/api_parity/phase_d_rich_text_public_examples_test.cpp` freezes the
  example and keeps it away from private runtime APIs. renderer glyph coloring and inline image drawing remain later work,
  alongside final examples closeout and final Phase D verification.
  Phase D text examples closeout lands in
  `tests/api_parity/phase_d_text_examples_closeout_test.cpp`. Step 450 closes the text examples band by freezing the
  public-only prelude examples:
  `api_parity_public_text_input_examples`,
  `api_parity_public_text_wrapper_examples`,
  `api_parity_public_text_input_workflow`, and
  `api_parity_public_rich_text_examples`. renderer glyph coloring and inline image drawing remain later work.
- [x] Steps 451-458: Run full Windows/WSL verification, update text rows in
  the parity ledger, and document which Unicode/IME behaviors are complete.
  Phase D final closeout lands in
  `tests/api_parity/phase_d_final_closeout_test.cpp`. Windows full debug suite
  passed 139/139, and WSL Arch Linux full debug suite passed 136/136 using
  D-drive WSL build/cache output plus `/dev/shm/cgpui` transient temp. The
  closeout keeps the completed text/IME/rich-text/example evidence distinct
  from remaining production gaps: DirectWrite font-file extraction, real color
  glyph rendering, native ZWJ ligature shaping depth, full Unicode script data,
  full Unicode bidirectional shaping/reordering, paragraph shaping, cache
  eviction policy, platform-derived font metrics, Unicode line-break classes,
  FreeType metrics and richer per-face coverage, platform-specific preedit
  styling, production candidate UI policy, runtime rich-text element dispatch,
  actual link command handling, inline image drawing/loading, syntax
  parsing/editor token source integration, and actual multi-color glyph
  painting remain later work. Phase E Step 459 Vulkan glyph atlas production
  follows this closeout.

## Phase E: Steps 459-538 - Vulkan Renderer Production Path

**Goal:** Convert renderer reports and records into real GPU resources and
draw calls for the Windows/Linux renderer.

**Primary modules:**
- `include/cgpui/renderer/*`
- `src/renderer/vulkan/*`
- `tests/renderer/*`
- `tests/examples/*`

- [x] Phase E Step 459 Vulkan glyph atlas production starts the renderer
  production path with focused atlas resource planning rather than shader work.
  `include/cgpui/renderer/glyph_atlas_production.hpp` and
  `src/renderer/vulkan/vulkan_glyph_atlas_production.cpp` add
  `GlyphAtlasProductionResourceState` plus
  `vulkan_plan_glyph_atlas_production_resources(...)` for alpha8 atlas page image readiness, memory allocation and bind readiness, image-view and sampler readiness, and dirty upload command path readiness. descriptor set binding remains Step 460 so the public leaf stays handle-free and the Vulkan
  state integration can land in focused private renderer files.
- [x] Phase E Step 460 creates real Vulkan glyph atlas descriptor resources in
  focused private modules. `VulkanRendererState` owns
  `VulkanGlyphAtlasResources` as handle-bearing atlas
  state through `vulkan_glyph_atlas_resources_internal.hpp`; descriptor layout,
  pool, set allocation, and `vkUpdateDescriptorSets` live in
  `vulkan_glyph_atlas_descriptors.cpp`; R8_UNORM image, device-local memory,
  image-view, and shared sampler creation live in
  `vulkan_glyph_atlas_images.cpp`; and resource reconciliation/destruction live
  in `vulkan_glyph_atlas_resources.cpp`. The handle-free Step 459 plan is
  consumed after the in-flight fence completes, and a real text-frame smoke
  exercises descriptor-bound atlas page creation. Dirty staging buffers,
  layout transitions, and buffer-to-image copies remain Step 461.
- [x] Phase E Step 461 records real dirty glyph-atlas uploads. Focused private
  upload state owns host-visible/coherent staging buffers and copy regions;
  `vulkan_glyph_atlas_staging.cpp` repacks each glyph at a 4-byte-aligned buffer
  offset, maps and copies only dirty alpha payloads, and retires staging after
  the in-flight fence. `vulkan_glyph_atlas_upload_recording.cpp` records
  UNDEFINED or SHADER_READ_ONLY to TRANSFER_DST barriers,
  `vkCmdCopyBufferToImage`, and the transition back to
  SHADER_READ_ONLY_OPTIMAL. `VulkanGlyphAtlasUploadResources` stays private,
  frame preparation lives in `vulkan_glyph_atlas_frame.cpp`, and planner/image
  layout state commits only after `vkQueueSubmit` succeeds.
- [x] Phase E Step 462 makes glyph-atlas submission incremental across frames
  and records only the acquired command buffer. Presentation now follows
  wait/prepare/acquire/record/reset-fence/submit order instead of rebuilding all
  swapchain command buffers. `recover_after_failed_record(...)` blocks
  presentation and rebuilds synchronization when recording fails after acquire.
  The Win32 Vulkan lifetime smoke preserves frame-outlives-renderer coverage and
  adds an `ab -> ab -> abc` sequence for initial upload, no-dirty reuse, and
  shader-readable incremental upload.
- [x] Phase E Step 463 proves multi-page glyph atlas allocation and
  cross-page uploads with 9 synthetic 128x128 glyphs spanning three atlas pages.
  The private `vulkan_glyph_atlas_descriptor_capacity` constant is shared by pool
  creation and a resource-update preflight, so an over-capacity production plan
  is rejected before existing pages are destroyed. Each page keeps its own
  descriptor set, upload batch, staging buffer, and buffer-to-image copies; the
  Win32 Vulkan smoke submits the same three-page workload. Step 464 continues
  the remaining atlas integration band.
- [x] Phase E Step 464 binds planned text page usage to renderer-owned
  descriptor sets through private `VulkanGlyphAtlasDrawBinding` records.
  `vulkan_resolve_glyph_atlas_draw_bindings(...)` rejects missing or stale page
  descriptors, renderer state owns the resolved bindings, and the live command buffer
  validates them before entering the render pass. This preserves a focused
  handle-bearing Vulkan boundary without starting the Step 467 text shader
  pipeline. Step 465 continues atlas draw-data integration.
- [x] Phase E Step 465 preserves renderer-ready glyph draw data in private
  `VulkanGlyphAtlasDrawData`. Flat textured glyph quads are paired with
  `first_quad_index` ranges for contiguous page runs, descriptor bindings copy
  those ranges, and live command recording validates range bounds and page
  identity before the render pass. `vulkan_glyph_atlas_draw_data.cpp` owns this
  planning so descriptor resolution remains focused. Step 466 closes the glyph
  atlas integration band before text shader work.
- [x] Phase E Step 466 closes the production glyph atlas integration band
  through `tests/api_parity/phase_e_glyph_atlas_integration_closeout_test.cpp`.
  This Phase E Step 466 glyph atlas integration closeout audit freezes
  Steps 459-465 evidence for alpha8 page resources,
  descriptor capacity, dirty uploads, acquired command buffer recording, three
  atlas pages, private `VulkanGlyphAtlasDrawBinding` and
  `VulkanGlyphAtlasDrawData` ownership, and contiguous page runs.
  Step 467 text shader pipeline follows; this closeout does not claim shader,
  vertex-buffer, pipeline, or textured draw-call implementation.
- [x] Phase E Step 467 defines the private text-pipeline vertex ABI and fixed
  state in `vulkan_text_pipeline_internal.hpp` and
  `vulkan_text_pipeline_state.cpp`. `VulkanTextVertex` exposes position,
  atlas-UV, and color attributes; the helpers freeze triangle-list input, no
  culling or depth, one-sample rasterization, dynamic viewport/scissor, and
  straight alpha blending without adding handles to public renderer headers or
  broad command recording. Step 468 adds validated embedded shader modules.
- [x] Phase E Step 468 adds reviewable vertex/fragment GLSL plus validated
  embedded SPIR-V in `vulkan_text_shader_binaries.cpp`.
  `vulkan_text_vertex_shader_spirv` and the fragment companion feed focused
  `vulkan_create_text_shader_modules` / destroy / stage helpers; optimized
  binaries pass `spirv-val` on Windows and WSL without making GLSL compilation
  a host build requirement. Step 469 creates pipeline-layout and graphics-
  pipeline handle ownership.
- [x] Phase E Step 469 adds swapchain-owned text graphics-pipeline resources in
  focused `vulkan_text_pipeline_resources_internal.hpp` and
  `vulkan_text_pipeline_resources.cpp` boundaries.
  `VulkanTextPipelineResources` persists the render-pass-compatible layout and
  pipeline handles, `vulkan_create_text_pipeline_resources` consumes the glyph-
  atlas descriptor-set layout plus an 8-byte vertex push constant, and shader
  modules stay transient during creation. Create/install/resize/destroy follow
  the swapchain lifetime. Step 470 adds text vertex-buffer upload resources.
- [x] Phase E Step 470 adds deterministic text-vertex expansion and private
  upload resources in `vulkan_text_vertex_buffer_internal.hpp` and
  `vulkan_text_vertex_buffer.cpp`. Each `TexturedGlyphQuad` becomes six
  triangle-list vertices, and `VulkanTextVertexBufferResources` owns a fence-
  safe host-visible/coherent vertex buffer rebuilt by
  `vulkan_upload_text_vertex_buffer` during frame preparation. Page-run order
  remains intact so Step 471 can bind descriptors and record textured draws.
- [x] Phase E Step 471 records real descriptor-bound textured glyph draws in
  focused `vulkan_text_draw_recording_internal.hpp` and
  `vulkan_text_draw_recording.cpp`. Draw ranges are validated and planned before
  the render pass; `vulkan_record_text_draws` then binds the text pipeline,
  full-frame viewport/scissor, uploaded vertex buffer, framebuffer-size push
  constants, and each page descriptor before one `vkCmdDraw` per contiguous
  page run. Step 472 makes subpixel positioning policy explicit.
- [x] Phase E Step 472 adds explicit private text positioning policy in
  `vulkan_text_positioning_internal.hpp` and
  `vulkan_text_positioning.cpp`. `VulkanTextPositioningPolicy` defaults to
  `preserve_subpixel`; the deterministic snap mode rounds outer device-quad
  edges before vertex expansion, preserves atlas UVs, and is recorded with the
  uploaded vertex-buffer resources. Step 473 defines gamma/alpha handling.
- [x] Phase E Step 473 adds focused glyph coverage and alpha policy in
  `vulkan_text_coverage_internal.hpp` and `vulkan_text_coverage.cpp`.
  `VulkanTextCoveragePolicy` keeps linear `R8_UNORM` coverage by default,
  supports deterministic power-transfer reference behavior, and defines
  `straight_color_coverage_alpha`: RGB remains straight while resolved coverage
  multiplies output alpha. The fragment shader clamps coverage, applies the
  explicit transfer, and ships as validated embedded SPIR-V. Step 474 closes
  text-pipeline integration.
- [x] Phase E Step 474 closes text pipeline integration with audit-only
  `tests/api_parity/phase_e_text_pipeline_integration_closeout_test.cpp`.
  This Phase E Step 474 text pipeline integration closeout freezes Steps 467-473,
  including descriptor-bound textured glyph draws, `preserve_subpixel`, and
  `straight_color_coverage_alpha`, then hands Phase E to
  Step 475 rounded rectangle geometry without adding renderer behavior.
- [x] Phase E Step 475 adds focused rounded rectangle geometry in
  `vulkan_rounded_rect_geometry_internal.hpp` and
  `vulkan_rounded_rect_geometry.cpp`. `VulkanRoundedRectGeometry` builds one
  set of contiguous vertex/index buffers plus stable draw ranges for all valid
  `RoundedRectDraw` records, pre-reserving capacity once and skipping empty
  rectangles. Step 476 owns Vulkan vertex/index buffer uploads.
- [x] Phase E Step 476 adds `VulkanRoundedRectBufferResources` and
  `vulkan_upload_rounded_rect_buffers` in focused resource/frame modules.
  Fence-safe frame preparation uploads paired host-visible/coherent vertex and
  index buffers, retains Step 475 draw ranges, and destroys them through
  `VulkanRendererState`. Step 477 owns the rounded rectangle shader pipeline.
- [x] Phase E Step 477 adds `VulkanRoundedRectPipelineResources` with a
  dedicated position/color vertex ABI, 8-byte framebuffer push constants,
  straight-alpha blending, transient shader modules, and swapchain-owned
  pipeline lifetime. Reviewable GLSL ships as embedded rounded rectangle SPIR-V
  validated for Vulkan 1.0. Step 478 owns indexed rounded rectangle draw
  recording.
- [x] Phase E Step 478 adds zero-allocation rounded rectangle range validation
  and `vulkan_record_rounded_rect_draws` in a focused private module. Frame
  recording binds the dedicated pipeline, vertex/index buffers, viewport,
  scissor, and framebuffer push constants before issuing `vkCmdDrawIndexed` for
  each retained draw range. Existing solid clear recording also moves to a
  focused module so the frame entry remains thin. Step 479 owns the rounded
  rectangle anti-aliasing strategy.
- [x] Phase E Step 479 adds `VulkanRoundedRectAntialiasingPolicy` with a default
  one-device-pixel coverage fringe. Geometry now emits full-coverage inner and
  zero-coverage outer rings, the vertex ABI carries coverage, and validated
  embedded shaders interpolate coverage into straight alpha without MSAA or
  descriptors. Step 480 owns border radius clipping and normalization.
- [x] Phase E Step 480 adds `VulkanRoundedRectRadiiResolution`. Negative radii
  clamp to zero and one CSS-style normalization scale guarantees
  adjacent corner sums fit the rectangle width and height before inner and coverage
  fringe contours are generated. Step 481 owns border stroke geometry.
- [ ] Steps 481-482: Complete rounded-rect rendering with border stroke and
  fill variants.
- [ ] Steps 483-490: Implement clip stack, scissor, stencil or shader clip
  strategy, nested opacity, transform composition, and z/layer ordering in
  actual command recording.
- [ ] Steps 491-498: Build image texture resources, upload staging, sampler
  modes, tint/opacity support, cache lifetime, and invalidation.
- [ ] Steps 499-506: Add SVG path rendering strategy or SVG rasterization
  boundary, including cache, scaling, recolor/tint, and examples.
- [ ] Steps 507-514: Add batching and frame scheduling: vertex/index buffers,
  command reuse, pipeline switches, resource barriers, swapchain recovery, and
  present pacing.
- [ ] Steps 515-522: Add renderer diagnostics that compare planned work with
  submitted GPU work, including upload bytes, draw counts, dropped resources,
  and frame timing.
- [ ] Steps 523-530: Add pixel/screenshot tests for text, rounded rects,
  images, clips, transforms, opacity, and resizing on Windows and Wayland.
- [ ] Steps 531-538: Run full Windows/WSL verification and mark the Vulkan
  renderer production path complete for required primitives.

## Phase F: Steps 539-618 - Win32 And Wayland Platform Production Depth

**Goal:** Make active desktop platforms behave like real application backends,
not protocol/test skeletons.

**Primary modules:**
- `include/cgpui/platform/*`
- `src/platform/win32/*`
- `src/platform/linux/*`
- `tests/platform/*`
- `tests/examples/*`

- [ ] Steps 539-546: Complete window lifecycle: creation, activation, focus,
  resize, scale changes, close policy, fullscreen, minimize/maximize, window
  positioning, transparent/decorated windows, and child-window ownership.
- [ ] Steps 547-554: Complete Win32 input: pointer, wheel, high precision
  scroll, keyboard, dead keys, text input, cursor theme/system cursors,
  capture, drag, and DPI changes.
- [ ] Steps 555-562: Complete Wayland input: seat capability changes,
  keyboard layout/modifiers, pointer enter/leave/motion/buttons/axis,
  fractional scale, configure lifecycle, cursor theme loading, and wakeups.
- [ ] Steps 563-570: Complete clipboard: Win32 Unicode/text/files where
  applicable, Wayland selection ownership/write/read, MIME negotiation,
  incremental transfer, failure handling, and diagnostics.
- [ ] Steps 571-578: Complete drag/drop: Win32 OLE drop target/source,
  Wayland data-device accept/action/finish negotiation, text/files/URI-list,
  non-local URI policy, and cancellation.
- [ ] Steps 579-586: Complete native menus and accelerators: menu tree,
  check/radio/enabled state, dynamic updates, accelerator display, dispatch,
  and platform diagnostics.
- [ ] Steps 587-594: Complete dialogs and platform services: open/save file,
  directory picker, message dialogs, open URL, quit/reopen behavior, and
  unsupported-result policy.
- [ ] Steps 595-602: Complete multi-window event loops: independent renderers,
  routing, activation/focus transfer, lifecycle cleanup, per-window themes,
  per-window accessibility, and child-window tests.
- [ ] Steps 603-610: Add production platform diagnostics and stress tests for
  window churn, clipboard ownership, DnD cancellation, IME, scale changes, and
  timer/task wakeups.
- [ ] Steps 611-618: Run full Windows/WSL verification and mark active
  platform production behavior complete.

## Phase G: Steps 619-678 - Accessibility, Async Runtime, Assets, Animation, And Test Macro Parity

**Goal:** Finish the cross-cutting systems that make GPUI usable for complex
apps rather than only demo windows.

**Primary modules:**
- `include/cgpui/accessibility/*`
- `include/cgpui/ui/runtime_*`
- `include/cgpui/ui/animation_*`
- `include/cgpui/assets/*`
- `src/ui/*`
- `src/platform/*accessibility*`
- `tests/accessibility/*`
- `tests/async/*`
- `tests/assets/*`

- [ ] Steps 619-626: Implement Win32 UIA provider objects, tree navigation,
  patterns, live events, focus/value/text changes, and provider lifetime.
- [ ] Steps 627-634: Implement Linux AT-SPI D-Bus object exposure, tree
  navigation, roles/states, text/value events, focus events, and bus lifecycle.
- [ ] Steps 635-642: Complete async runtime: task pool, priorities, structured
  groups, cancellation propagation, async I/O hooks, timer integration, and
  cross-thread entity access rules.
- [ ] Steps 643-650: Complete animation: transitions, element lifecycle
  animations, spring/tween variants, cancellation, frame pacing, style
  interpolation, and the official animation/opacity examples.
- [ ] Steps 651-658: Complete assets: file-backed loading, PNG/JPEG/GIF/SVG
  decode boundaries, cache keys, reload invalidation, async loading, and
  image/GIF examples.
- [ ] Steps 659-666: Add GPUI-style test macro/context equivalents in C++:
  app setup, window setup, simulated input, timers, async, rendering, and
  platform service fakes.
- [ ] Steps 667-672: Add packaging and CI coverage for Windows and Linux:
  debug/release builds, examples, smoke tests, architecture/header tests, and
  reproducible dependency setup.
- [ ] Steps 673-678: Run full Windows/WSL verification and declare
  Windows/Linux "requested target parity candidate" only if the parity ledger
  has no required Windows/Linux gaps.

## Phase H: Steps 679-758 - macOS Cocoa + Metal Parity

**Goal:** Bring macOS to the same public API and behavior after Windows/Linux
stabilize, without forking public authoring semantics.

**Mac start gate:**
- [ ] Windows full debug and WSL full debug have passed after Phase G.
- [ ] Public authoring APIs are stable enough that a Mac port will not chase
  large context/element/runtime churn.
- [ ] Renderer command boundaries are stable enough to map to Metal.
- [ ] A Mac host with Xcode and command-line tools is available for verification.

**Primary modules:**
- `src/platform/macos/*`
- `src/renderer/metal/*`
- `include/cgpui/platform/*`
- `include/cgpui/renderer/*`
- `tests/platform/macos/*`
- `tests/renderer/metal/*`
- `examples/*`

- [ ] Steps 679-686: Add Cocoa application and NSWindow lifecycle:
  app delegate, run loop, window creation, close policy, activation, scaling,
  resize, cursor, and wakeups.
- [ ] Steps 687-694: Add CAMetalLayer and Metal renderer bootstrap:
  device, command queue, drawable lifecycle, resize, frame pacing, and
  diagnostics.
- [ ] Steps 695-702: Port renderer primitives to Metal: solid rect,
  rounded rect, text glyph atlas, images/SVG, clip stack, opacity, transform,
  batching, and frame statistics.
- [ ] Steps 703-710: Add macOS input: mouse, trackpad scrolling, keyboard,
  modifiers, text input, focus, cursor, capture, drag gestures, and scale.
- [ ] Steps 711-718: Add macOS text and fonts: CoreText font discovery,
  shaping/rasterization integration, IME candidate placement, marked text,
  delete-surrounding equivalent behavior, and text services diagnostics.
- [ ] Steps 719-726: Add macOS clipboard and drag/drop:
  NSPasteboard text/files/images, drag source/target, operation negotiation,
  cancellation, and diagnostics.
- [ ] Steps 727-734: Add macOS menus, accelerators, file dialogs, message
  dialogs, open URL, quit/reopen, window chrome, and platform services.
- [ ] Steps 735-742: Add macOS accessibility: NSAccessibility tree, roles,
  values, focus, text/value events, live updates, and provider lifetime.
- [ ] Steps 743-750: Port all public examples and smoke tests to macOS using
  the same public API as Windows/Linux.
- [ ] Steps 751-758: Run macOS full debug plus Windows/WSL regression suites,
  then mark Mac parity complete in the ledger.

## Phase I: Steps 759-798 - Optional X11 Backend For Strict Upstream Platform Matrix

**Goal:** Only if the user wants literal upstream Linux backend parity, add X11
after Wayland is production-grade. Do not let X11 slow the active Wayland path.

**Activation rule:** Leave this phase unchecked and inactive unless the user
explicitly accepts X11 as part of the full replication target.

- [ ] Steps 759-764: Add X11/XCB platform boundary, source layout, build flags,
  architecture tests, and backend selection without changing public APIs.
- [ ] Steps 765-770: Implement X11 window lifecycle, Vulkan surface creation,
  resize/scale, focus, close, and event loop wakeups.
- [ ] Steps 771-776: Implement X11 input: pointer, wheel, keyboard, text,
  cursor, capture, and keymap modifiers.
- [ ] Steps 777-782: Implement X11 clipboard and drag/drop using selection
  ownership, MIME conversion, file/text payloads, and diagnostics.
- [ ] Steps 783-788: Implement X11 menus/dialog service policy, accessibility
  boundary, platform diagnostics, and smoke tests.
- [ ] Steps 789-794: Run Linux matrix verification for Wayland and X11 without
  regressing Windows or macOS.
- [ ] Steps 795-798: Update the parity ledger to show strict Linux backend
  parity complete.

## Phase J: Steps 799-840+ - Final Upstream Audit Closure And Release Hardening

**Goal:** Close every required row in the parity ledger and make the result
usable as a C++23 GPUI replacement.

- [ ] Steps 799-804: Re-run upstream extractor against the pinned revision and
  fail if any required API row lacks a C++ equivalent or accepted adaptation.
- [ ] Steps 805-810: Compile every official example equivalent on every active
  platform and run smoke/pixel/input tests where possible.
- [ ] Steps 811-816: Add performance baselines: startup, first frame, resize,
  text layout, list scrolling, image loading, async wakeups, and frame pacing.
- [ ] Steps 817-822: Add stress tests: window churn, many entities, large
  uniform lists, IME composition, clipboard ownership changes, DnD cancel,
  asset reload, and task cancellation.
- [ ] Steps 823-828: Complete documentation: getting started, architecture,
  platform dependencies, examples, migration notes from GPUI concepts to
  C++23 concepts, and non-goal list.
- [ ] Steps 829-834: Package Windows/Linux/macOS builds, CI jobs, dependency
  bootstrap scripts, and release artifacts.
- [ ] Steps 835-840: Run final full matrix verification and lock the parity
  ledger. Any remaining unchecked required row blocks the "fully replicated"
  claim.
- [ ] Steps 841+: Reserve for upstream drift. If upstream GPUI changes after
  the pinned revision, add a new parity delta plan instead of mutating the
  completed roadmap.

## Immediate Next Slice

Step 361 starts SVG/image element front-end APIs after the Step 360
window/examples closeout. Step 360 closes the window/examples widget band
through `tests/api_parity/phase_c_window_examples_closeout_test.cpp`, guarding
the Steps 355-359 public example evidence and the prelude-only boundary before
the SVG/image band starts. Phase C Step 361 SVG/image front-end source APIs
adds the focused `ImageSource` layer and hands off to Phase C Step 362 SVG/image
asset registration. Phase C Step 362 SVG/image asset registration adds the
focused `ImageAssetRegistry` layer over `ImageSource` and hands off to Phase C
Step 363 SVG/image public example coverage. Phase C Step 363 SVG/image public
example coverage adds the `public_svg_image_sources` example and hands off to
Phase C Step 364 SVG/image band closeout. Phase C Step 364 SVG/image band
closeout closes the front-end SVG/image authoring band with
`tests/api_parity/phase_c_svg_image_closeout_test.cpp` and hands off to Phase C
Step 367 widget family structure tests. Step 367 adds the explicit widget
family boundary table in `tests/architecture/widget_source_structure_test.cpp`,
guards it with `tests/api_parity/phase_c_widget_family_structure_test.cpp`, and
hands off to Phase C Step 373 final element/style/widget ledger audit.
The Phase B public authoring boundary remains the source of truth for this
next slice.
Step 319 landed the child-list foundation on `master` at `14aaff0`; Step 320
landed the flex vocabulary helpers on `master` at
`5040365`; Step 321 landed the sizing/color/border helper aliases on `master`
at `694d64e`; Step 322 landed overflow/opacity/position helper aliases on
`master` at `f4f2fc2`; Step 323 landed text-style aliases on `master` at
`78f2f05`; Step 324 landed focused shadow vocabulary/storage on `master` at
`ef306c0`; Step 325 landed focused min/max layout constraints on `master` at
`a1c945e`; Step 326 landed focused percentage-like sizing on `master` at
`7b748b7`; Step 327 landed focused margin/padding shorthand vocabulary and gap
composition coverage on `master` at `32ec5e1`; Step 328 landed focused
absolute/fixed positioning with `Position::fixed`, `ElementBuilder::fixed()`,
and out-of-flow positioned child layout on `master` at `38574a2`; Step 329
landed focused direct overlay-layer ordering through `z_order()` for
direct-container child paint, hit-test, and event dispatch on `master` at
`6c1bfe4`; and Step 330 landed focused nested scroll clipping through
effective paint clip intersection on `master` at `266bc9f`. Step 331 landed
focused active-state style cascade support through `StyleState::active`,
`StyleStateFlags::active`, `ElementBuilder::active_style(...)`,
`ButtonBuilder::active_style(...)`, and hover/focus/active/disabled overlay
ordering on `master` at `c621fe8`. Step 332 landed focused class-style reuse
depth through
`StyleClassRule`, `StyleCascade::set_class_rule(...)`, out-of-line
`src/ui/style_cascade.cpp` ownership, and depth-first reused class resolution
with cycle protection on `master` at `630bb3d`. Step 333 landed focused theme
token fallback through `StyleThemeTokens`, theme-aware
`resolved_style(...)` overloads, and missing-token fallback to existing
concrete style values on `master` at `e1d5467`. Step 334 landed focused
inherited text style through `StyleAuthoredTextFields`,
`src/ui/text_style_inheritance.cpp`, effective text styles on text nodes, and
container/wrapper forwarding for foreground color, font family, and font size
on `master` at `a472317`.
Step 335 landed focused dynamic style invalidation on `master` at `e88bd78`
through
`WindowRuntime::request_style_state_invalidation(...)`, hover/focus element
transition hooks, and `src/ui/runtime_style_invalidation.cpp` ownership.
Step 336 closes the Phase C style-cascade depth band through
`tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp`, guarding the
Steps 331-335 evidence and the explicit Step 337 handoff. Step 337 opens the
focusable/interactable semantics band with pointer-active input behavior while
leaving broad style cascade behavior and broad resolved-style layout/paint
rewrites closed. Step 338 adds tab-order/focus-ring metadata through
`FocusMetadata`, `ElementBuilder::tab_index(...)`, accessibility metadata, and
`src/ui/runtime_focus_order.cpp`. Step 339 adds focused click/drag gesture
synthesis through `ElementGestureKind::click`, runtime gesture state, and
`src/ui/runtime_gesture_synthesis.cpp`. Step 340 adds keyboard activation
semantics through focused Enter/Space synthesized clicks and raw-key-handler
first refusal. Step 341 adds disabled interaction semantics through
`WindowRuntime::refresh_disabled_interaction_state()`, focused
`src/ui/runtime_disabled_interaction.cpp` ownership, and stale interaction
state cleanup for disabled or missing elements. Step 342 closes the focusable/interactable band through
`tests/api_parity/phase_c_focusable_interactable_audit_test.cpp`, guarding the
Steps 337-341 evidence and the explicit Step 343 built-in widget expansion
handoff. Step 343 starts built-in widget expansion with focused button, label,
and text-input builder module ownership: `ButtonBuilder::label(...)` composes
button labels without caller-side `label(...).build()` boilerplate, widget
builder implementation bodies move under `src/ui/widgets/*`, `xmake.lua`
compiles that module, and focused behavior/structure coverage lives in
`tests/ui/builtin_widget_test.cpp` and
`tests/architecture/widget_source_structure_test.cpp`; it landed on `master`
at `10907e2 feat: add built-in widget module boundary`. Step 344 adds
checkbox/radio/switch widgets through `ToggleBuilder`, `checkbox(...)`,
`radio(...)`, `toggle_switch(...)`, `ToggleControlElement`, focused
`src/ui/widgets/toggle_builder.cpp`, `src/ui/element_choice_nodes.cpp`,
`src/ui/element_choice_layout.cpp`, and `src/ui/element_choice_paint.cpp`,
plus accessibility role/value metadata for checked/selected/on state. Step 345
adds a focused slider widget through `SliderBuilder`, `slider(...)`,
`SliderElement`, focused `src/ui/widgets/slider_builder.cpp`,
`src/ui/element_slider_nodes.cpp`, `src/ui/element_slider_layout.cpp`, and
`src/ui/element_slider_paint.cpp`, plus accessibility role/value metadata for
the current slider value. Step 346 adds list item and menu item widgets through
`ItemBuilder`, `list_item(...)`, `menu_item(...)`, `ItemElement`, focused
`src/ui/widgets/item_builder.cpp`, `src/ui/element_item_nodes.cpp`,
`src/ui/element_item_layout.cpp`, and `src/ui/element_item_paint.cpp`, plus
accessibility role/value metadata for selected list items and menu actions.
Step 347 adds icon/image widgets through `ImageBuilder`, `image(...)`,
`icon(...)`, `ImageElement`, focused `src/ui/widgets/image_builder.cpp`,
`src/ui/element_image_nodes.cpp`, `src/ui/element_image_layout.cpp`, and
`src/ui/element_image_paint.cpp`, plus source-rect, alternate-text, and icon
tint metadata over the existing image asset descriptor and paint command path.
Step 348 closes the built-in widget band with focused container primitive
free-function authoring through `include/cgpui/ui/container_builder.hpp`,
`src/ui/widgets/container_builder.cpp`, and `div()`, `h_flex()`, `v_flex()`,
`h_stack()`, and `v_stack()` public vocabulary. Step 349 starts uniform list parity with `UniformListVisibleRange`, `UniformListItemIdentity`,
`UniformListLayoutSnapshot`, `calculate_uniform_list_visible_range(...)`, and
`ScrollableListElement::layout_snapshot()` over the existing
`scrollable_list` path. Step 350 adds keyed scroll anchoring through
`UniformListScrollAnchor`, `capture_uniform_list_scroll_anchor(...)`,
`apply_uniform_list_scroll_anchor(...)`, and `ScrollableListElement::layout()`
anchor application. Step 351 adds keyed item measurement caching through
`UniformListItemMeasurement`, `UniformListItemMeasurementResult`,
`UniformListItemMeasurementCache`, `measure_uniform_list_items(...)`,
`ScrollableListElement::measurement_cache()`, focused
`src/ui/uniform_list_measurement.cpp` ownership, and snapshot-level cache
hit/miss observability. Step 352 adds `UniformListRecyclingWindow`,
`calculate_uniform_list_recycling_window(...)`,
`UniformListLayoutSnapshot::recycling_window`, `UniformListItemIdentity::recycled`,
focused `src/ui/uniform_list_recycling.cpp` ownership, and retained-window
painting. Step 353 adds `UniformListSelectionSource`,
`UniformListSelectionDirection`, `UniformListSelection`,
`UniformListSelectionState`, `select_uniform_list_item_at_point(...)`,
`move_uniform_list_selection(...)`, `ScrollableListElement::selection()`,
focused `src/ui/uniform_list_selection.cpp` and
`src/ui/element_scroll_events.cpp` ownership, and snapshot selected flags.
Step 354 closes the uniform-list band through
`tests/api_parity/phase_c_uniform_list_audit_test.cpp`, guarding the Steps
349-353 evidence and the explicit Step 355 handoff. Step 355 starts the window/examples widget band through
`examples/api_parity/public_window_examples/main.cpp`,
`api_parity_public_window_examples`, and
`tests/api_parity/phase_c_window_examples_public_api_test.cpp` for menu demos,
shadow, window positioning, window shadow, and input examples using public
APIs. Step 356 adds `examples/api_parity/public_window_examples_workflow/main.cpp`,
`api_parity_public_window_examples_workflow`, and
`tests/api_parity/phase_c_window_examples_workflow_test.cpp` for a public
test-context workflow covering menu installation, window activation/focus, key
binding simulation, pointer dispatch, text input, and shadow/fixed positioning
examples. Step 357 adds `examples/api_parity/public_window_examples_widget_catalog/main.cpp`,
`api_parity_public_window_examples_widget_catalog`, and
`tests/api_parity/phase_c_window_examples_widget_catalog_test.cpp` for a
prelude-only widget catalog covering checkbox/radio/switch, slider, list/menu,
image/icon, and container widgets. Step 358 should continue the window/examples widget band. Step 358 adds `examples/api_parity/public_window_examples_interaction_states/main.cpp`,
`api_parity_public_window_examples_interaction_states`, and
`tests/api_parity/phase_c_window_examples_interaction_states_test.cpp` for
hover/focus/active/disabled interaction states, focus ring and tab-index examples,
keyed controls, and click handlers. Step 359 adds `examples/api_parity/public_window_examples_service_matrix/main.cpp`,
`api_parity_public_window_examples_service_matrix`, and
`tests/api_parity/phase_c_window_examples_service_matrix_test.cpp` for
menu accelerators and command palette service matrix plus window options, shadow, fixed positioning, and text input service examples.
Step 360 closes the window/examples widget band through
`tests/api_parity/phase_c_window_examples_closeout_test.cpp`. Keep the Phase B closeout
exclusions out of the Step 355/356/357/358/359/360 band:
`ClipboardItem` payload parity, upstream `gpui::test` macro equivalents,
action macro payloads, task priorities, structured task groups, private runtime headers, or direct `WindowRuntime` use.

## Self-Review

- Spec coverage: covers Windows/Linux Vulkan, Wayland-first Linux, deferred
  Cocoa + Metal macOS, optional X11, upstream API audit, examples, renderer,
  platform, text, accessibility, async, assets, animation, tests, docs, and
  final release gates.
- Placeholder scan: no unresolved placeholder tokens are used. Deferred work
  is represented as explicit phase gates.
- Type consistency: all CGPUI paths follow the current modular leaf/aggregate
  layout policy from `AGENTS.md`; future exact file names should be finalized
  in each phase-specific implementation plan before code changes.
- Risk control: full replication is tied to a pinned upstream revision, so the
  project can finish a concrete target while still allowing later upstream
  drift delta plans.
