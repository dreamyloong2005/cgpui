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
- [ ] WSL Arch Linux full debug passes after every shared, renderer, Wayland,
  or platform-facing merge.
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
- [ ] The parity ledger has no required gaps. Any non-goal, such as active X11
  exclusion, is explicitly accepted by the user.

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
- [ ] Run WSL tests when the slice touches shared UI, renderer, platform,
  Wayland, build, or headers.
- [ ] Run `git diff --check`.
- [ ] Fast-forward merge to `master`.
- [ ] Run post-merge Windows full debug and required WSL full debug.
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

- [ ] Steps 319-324: Add the upstream-style `div` element vocabulary:
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
- [ ] Steps 355-360: Implement window/examples widgets for menu demos, shadow,
  window positioning, window shadow, and input examples using public APIs.
  Phase C Step 355 window/examples widgets starts this band by adding
  `examples/api_parity/public_window_examples/main.cpp`,
  `api_parity_public_window_examples`, and
  `tests/api_parity/phase_c_window_examples_public_api_test.cpp`. The example
  covers menu demos, shadow, window positioning, window shadow, and input
  examples using public APIs and remains prelude-only. Step 356 should deepen
  the window/examples widget band without introducing private runtime headers.
- [ ] Steps 361-366: Add SVG/image element front-end APIs that feed the asset
  pipeline without adding renderer details to public element headers.
- [ ] Steps 367-372: Add structure tests requiring every widget family to have
  a public leaf header, a focused source file, and focused behavior tests.
- [ ] Steps 373-378: Run full Windows/WSL verification and update the ledger
  so element/style/widget rows are either complete or explicitly deferred.

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

- [ ] Steps 379-386: Replace fallback-only shaping with HarfBuzz-backed
  shaping on Windows/Linux while preserving deterministic test fallbacks.
- [ ] Steps 387-394: Add real font discovery and fallback: DirectWrite on
  Windows, fontconfig/FreeType on Linux, and later CoreText on macOS.
- [ ] Steps 395-402: Add per-script and per-codepoint fallback splitting,
  font coverage checks, emoji/color glyph planning, and missing-glyph
  diagnostics.
- [ ] Steps 403-410: Complete text measurement and wrapping: grapheme columns,
  soft wraps, hard wraps, bidirectional text planning, line boxes, baseline,
  ascent/descent, and paragraph caches.
- [ ] Steps 411-418: Complete selection and caret behavior: mouse drag,
  double/triple click, word/line selection, scroll-to-caret, preferred column,
  selection painting, and clipboard integration.
- [ ] Steps 419-426: Deepen edit history: grouped typing, IME grouped commits,
  undo manager integration points, redo invalidation, and edit transaction
  diagnostics.
- [ ] Steps 427-434: Complete IME on active targets: Win32 TSF/IMM depth,
  Wayland text-input v3 surrounding text, delete-surrounding, content hints,
  serial policy, preedit styling, and candidate placement.
- [ ] Steps 435-442: Add rich text runs: spans, links, inline images, syntax
  color-like attributes, underline/strikethrough, background, and hit testing.
- [ ] Steps 443-450: Add text input parity examples and API compatibility
  tests for the official input and text wrapper examples.
- [ ] Steps 451-458: Run full Windows/WSL verification, update text rows in
  the parity ledger, and document which Unicode/IME behaviors are complete.

## Phase E: Steps 459-538 - Vulkan Renderer Production Path

**Goal:** Convert renderer reports and records into real GPU resources and
draw calls for the Windows/Linux renderer.

**Primary modules:**
- `include/cgpui/renderer/*`
- `src/renderer/vulkan/*`
- `tests/renderer/*`
- `tests/examples/*`

- [ ] Steps 459-466: Create real Vulkan glyph atlas images, memory allocation,
  image views, samplers, descriptor sets, and dirty upload command paths.
- [ ] Steps 467-474: Add text shader pipeline, descriptor layout, textured
  glyph draw calls, subpixel positioning policy, and gamma/alpha handling.
- [ ] Steps 475-482: Promote rounded-rect records to real geometry buffers,
  anti-aliasing strategy, border radius clipping, border stroke, and fill
  variants.
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

Step 356 should deepen the window/examples widget band from the Step 355
public example entry. Step 354 closes the uniform-list band through
`tests/api_parity/phase_c_uniform_list_audit_test.cpp`, guarding the focused
layout, style, focusable/interactable, built-in widget, Step 349 stable
identity/range, Step 350 scroll anchoring, Step 351 item measurement cache,
Step 352 recycling, and Step 353 keyboard/pointer selection evidence from
Steps 319-353. The Phase B public authoring boundary remains the source of
truth for this next slice.
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
APIs. Step 356 should deepen the window/examples widget band. Keep the Phase B closeout
exclusions out of the Step 355 start:
`ClipboardItem` payload parity, upstream `gpui::test` macro equivalents,
action macro payloads, task priorities, or structured task groups.

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
