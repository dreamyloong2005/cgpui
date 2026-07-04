# CGPUI GPUI-Core 128-Step Plan

## Goal

Track small, verified implementation slices toward a Windows/Linux
GPUI-core-like API. Windows and Linux stay on Vulkan, Linux stays on Wayland,
and macOS/Cocoa + Metal remains an explicit readiness boundary until the
Windows/Linux core API is stable enough for parity work.

## Current Baseline

- `master` starts at `779e62c feat: add pointer capture owner`.
- Windows and Linux use Vulkan.
- Linux platform target is Wayland.
- Step 88 adds the explicit macOS/Cocoa + Metal target boundary without making
  macOS parity part of the active Windows/Linux implementation track.

## Definition Of Done For This 20-Step Goal

- All 20 steps below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 40-Step Goal

- Steps 1-20 remain implemented and verified on `master`.
- Steps 21-40 below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 60-Step Goal

- Steps 1-40 remain implemented and verified on `master`.
- Steps 41-60 below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 128-Step Goal

- Steps 1-88 remain implemented and verified on `master`.
- Steps 89-128 below are implemented, committed, merged to `master`, and verified.
- Each implementation step follows the existing RED/GREEN pattern with a
  targeted test that fails for the expected missing API or behavior before the
  implementation lands.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI,
  platform, renderer, or build surfaces.
- macOS-specific code remains source-isolated behind macOS build guards until
  a dedicated Mac parity run begins.
- `git status --short --branch` on `master` has no tracked/staged changes from
  the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 168-Step Follow-On Goal

- Steps 1-128 remain implemented, merged to `master`, and verified on Windows
  and WSL Arch Linux.
- Steps 129-168 below are implemented only after the 128-step goal is complete;
  until then they are a planned follow-on queue, not the active execution focus.
- The follow-on work moves from "close to GPUI core API" toward a practical
  Windows/Linux GPUI-core experience: context/entity ergonomics, keyed
  reconciliation, reusable widgets, text/font rendering depth, async/timer
  integration, diagnostics, and platform-backed Win32/Wayland behavior.
- Windows and Linux continue to use Vulkan; Linux continues to target Wayland.
- macOS/Cocoa + Metal remains a readiness boundary and must not become a parity
  requirement inside this follow-on Windows/Linux track.
- Each implementation step follows the same RED/GREEN, feature-worktree,
  merge, Windows verification, and WSL verification discipline used for Steps
  89-128.

## Definition Of Done For The 178-Step Depth Goal

- Steps 1-168 remain implemented, merged to `master`, and verified on Windows
  and WSL Arch Linux.
- Steps 169-178 are implemented as a Windows/Linux depth pass, not a macOS
  parity pivot. Windows continues to use Win32 + Vulkan, Linux continues to
  use Wayland + Vulkan, and macOS/Cocoa + Metal remains deferred.
- Vulkan text moves beyond metadata toward deterministic glyph bitmap
  rasterization, atlas allocation/upload records, textured glyph quads, and
  renderer reports that distinguish real text draw preparation from
  placeholder metadata.
- Wayland clipboard, drag/drop, and text-input move from skeleton state toward
  MIME payload extraction and protocol state-machine behavior that can be
  exercised by the test compositor.
- Windows UIA and Linux AT-SPI adapter skeletons consume the existing
  `AccessibilityTreeSnapshot` without forking public UI semantics.
- Multi-window work turns existing runtime records into native additional
  window creation scaffolding while preserving the single-window root path.
- Each implementation step follows RED/GREEN, feature-worktree isolation,
  merge, Windows verification, and WSL verification.

## Definition Of Done For The 218-Step Production-Depth Goal

- Steps 1-178 remain implemented, merged to `master`, and verified on Windows
  and WSL Arch Linux.
- Steps 179-218 are implemented as the next Windows/Linux production-depth
  pass, not a macOS parity pivot. Windows continues to use Win32 + Vulkan,
  Linux continues to use Wayland + Vulkan, and macOS/Cocoa + Metal remains a
  separate track.
- Vulkan renderer work promotes text and shape paths from deterministic
  metadata/reporting toward atlas image planning, texture resource lifetime
  records, dirty uploads, sampler-pipeline readiness, rounded-rect geometry,
  text selection/caret support, clip/opacity/transform stack reporting, and
  frame-level renderer reports.
- Text, font, and IME work deepens fallback resolution, platform font
  discovery records, grapheme/word navigation, undo/redo, delete-surrounding
  editing, multiline navigation, measurement caching, pointer selection, and
  soft-wrap records.
- Win32 and Wayland platform work promotes clipboard, drag/drop, cursor,
  configure lifecycle, OLE drag/drop, menus, file dialogs, window chrome,
  command palette metadata, and platform diagnostics without forking public UI
  semantics.
- Accessibility, multi-window, theme, animation, assets, and async work
  upgrades skeletons into richer provider facades, child-window renderer/event
  ownership, runtime theme switching, deterministic animations, image assets,
  and a cancellable threaded executor.
- Each implementation step follows RED/GREEN, feature-worktree isolation,
  merge, Windows verification, and WSL verification.

## Steps 89-128 Execution Roadmap

Detailed execution plan:
`docs/superpowers/plans/2026-06-30-gpui-core-steps-89-128-execution-plan.md`.

- Band A, Steps 89-98: authoring ergonomics and render entry. This lands the
  GPUI-like public surface for elements, style helpers, event handler
  shortcuts, style states, `View::render(ViewContext&)`, rendered tree
  installation, and render invalidation observability.
- Band B, Steps 99-108: model, app, window, and view lifecycle. This introduces
  public model aliases, context model helpers, weak handles, observations,
  model-driven invalidation, `AppContext`, `WindowOptions`, root-view
  lifecycle storage, a view registry, and child-view placeholders.
- Band C, Steps 109-118: event propagation, focus traversal, scroll routing,
  hidden-overflow hit testing, and deeper layout controls. This makes route
  ancestry, bubbling, Tab traversal, scroll state binding, flex alignment,
  grow/shrink, absolute positioning, and layer/elevation behavior explicit.
- Band D, Steps 119-128: render command depth, text metadata, platform cursor
  and clipboard backends, IME geometry, and the public-prelude demo rewrite.
  This is the Windows/Linux backend-hardening band while macOS remains a
  readiness boundary.

## Steps 129-168 Follow-On Roadmap

Detailed follow-on plan:
`docs/superpowers/plans/2026-06-30-gpui-core-steps-129-168-forward-plan.md`.
That plan now includes branch slugs, first RED test intent, targeted commands,
checkpoint exits, and Windows/WSL verification matrix for all 40 follow-on
steps. The pre-back-40 handoff is complete: Steps 89-128 are merged to
`master` and post-merge verified on Windows and WSL Arch Linux.

## Steps 169-178 Depth Roadmap

Detailed depth plan:
`docs/superpowers/plans/2026-07-02-gpui-core-depth-steps-169-178-plan.md`.
The active track stays Windows/Linux first. The ten-step depth pass prioritizes
Vulkan text rendering data flow, Wayland data/protocol handling, native
accessibility adapter boundaries, and native multi-window creation before a
separate macOS/Cocoa + Metal parity track begins.

## Steps 179-218 Production-Depth Roadmap

Detailed production-depth plan:
`docs/superpowers/plans/2026-07-02-gpui-core-depth-steps-179-218-plan.md`.
The active track remains Windows/Linux first. The forty-step pass is organized
as Vulkan text/renderer depth, text/font/editing depth, Win32/Wayland native
platform depth, and accessibility/multi-window/theme/asset/animation/async
depth before a separate macOS/Cocoa + Metal parity track begins.

## Complete GPUI Replication Roadmap

Detailed complete-replication roadmap:
`docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md`.
The current baseline is post-Step-218 plus the merged structural optimization
pass. Phase A, Steps 219-258, is implemented on
`codex/gpui-upstream-parity-ledger`: the upstream GPUI revision is pinned, a
complete-replication parity ledger and JSON export exist, the extractor is in
place, and the first API parity gate plus hello-world parity example are
registered. Windows/Linux remain first. macOS/Cocoa + Metal starts after
Windows/Linux public APIs and renderer/platform boundaries are stable. X11
remains out of the active track unless the user explicitly chooses strict
upstream Linux backend parity.

Step 258, upstream GPUI parity ledger and first API parity gate, is merged on
`master` at `2a21b68 docs: add gpui upstream parity ledger` and post-merge
verified on Windows and WSL Arch Linux. The branch pins upstream Zed/GPUI at
`5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`, records crate versions
`gpui = 0.2.2` and `gpui_platform = 0.1.0`, adds
`docs/gpui-complete-parity-ledger.md`,
`docs/gpui-complete-parity-ledger.json`,
`tools/gpui_parity/extract_upstream_symbols.py`,
`tests/api_parity/gpui_parity_ledger_test.cpp`, and
`examples/api_parity/hello_world/main.cpp`. Verification passed: Windows
extractor and JSON validation, WSL extractor and JSON validation, Windows
focused parity test 1/1, WSL focused parity test 1/1, Windows hello-world
parity build, WSL hello-world parity build, feature-worktree Windows full
debug 42/42, feature-worktree WSL full debug 39/39, `git diff --check` exit 0
with only expected LF-to-CRLF normalization warnings, post-merge Windows full
debug 42/42, and post-merge WSL full debug 39/39. Phase B, Steps 259-318, is
the next implementation phase: public Application, Context, Entity, Action,
key dispatch, and test-context API parity.

Step 259, public `Application` facade, is merged on `master` at
`e824643 feat: add application facade`. RED failed as expected on missing
`cgpui::Application`. GREEN adds focused app-module ownership through
`include/cgpui/app/application.hpp`, `include/cgpui/app/app.hpp`, and
`src/app/application.cpp`, plus a new `cgpui_app` target. The facade is
move-only, owns a `PlatformApplication`, exposes `Application::create()`,
keeps `platform_application()` access for low-level interop, and forwards
`run(...)` to the existing `run_app(...)` path without removing
`create_platform_application()` or `run_app(...)`. The public aggregate
`include/cgpui/cgpui.hpp` now includes `cgpui/app/app.hpp`, and the
hello-world API parity example uses `Application::create()` plus `app->run`.
The WSL focused failure during verification was traced to a test use-after-free:
`application_facade_test` read a renderer pointer after `run_app` had destroyed
its owned renderer vector. The test now records begin-frame count through an
external reference, matching the established `app_runner_test` pattern.
Feature-worktree verification passed Windows focused 6/6, Windows
`api_parity_hello_world` build, WSL focused 6/6, WSL
`api_parity_hello_world` build, and `git diff --check` with only expected
LF-to-CRLF normalization warnings. Feature-worktree full debug passed Windows
44/44 and WSL Arch Linux 41/41. Post-merge verification passed Windows full
debug 44/44 and WSL Arch Linux full debug 41/41.

Step 260, public `App` and `Window` context facades, is merged on `master` at
`d3a501c feat: add app window context facades`. RED failed as expected on
missing `cgpui::App`, `cgpui::Window`, `AppContext::app()`,
`WindowRuntimeContext::app()`, and `WindowRuntimeContext::current_window()`.
GREEN added app-module ownership through `include/cgpui/app/app_facade.hpp`,
`include/cgpui/app/window.hpp`, `src/app/app_facade.cpp`, and
`src/app/app_context_facade.cpp`; after Step 279, `Window` implementation
lives in `src/ui/window.cpp` so UI headers link through `cgpui_ui`.
`include/cgpui/app/app.hpp` remains a thin aggregate.
The slice preserves low-level runtime entrypoints and exposes narrow facade
queries for `open_window`, `root_window`, `window(runtime_id)`, descriptor,
viewport, scale, focus, and invalidation forwarding. Windows and WSL focused
verification passed 8/8 each. Feature-worktree full debug passed Windows
46/46 and WSL Arch Linux 43/43. Post-merge verification passed Windows full
debug 46/46 and WSL Arch Linux full debug 43/43.

Step 261, public `Context<T>` / `Render` / `IntoElement` spelling depth, is
merged on `master` at `56553e0 feat: add context render spelling` and
post-merge verified on Windows and WSL Arch Linux. RED failed as expected on
missing `include/cgpui/ui/render.hpp`. GREEN adds the focused render leaf
header, keeps `View` ownership in `include/cgpui/ui/view.hpp`, updates the
hello-world parity example to use `Context<HelloWorldView>`, `IntoElement`,
and `Render<HelloWorldView>`, and records the ledger evidence. Feature-worktree
verification passed Windows focused 5/5, WSL focused 5/5, Windows full debug
47/47, and WSL Arch Linux full debug 44/44. Post-merge verification passed
Windows full debug 47/47 and WSL Arch Linux full debug 44/44. Step 262,
deeper public `Context<T>` app/window/entity access patterns, is the next
Phase B slice.

Step 262, deeper public `Context<T>` app/window/entity access patterns, is
merged on `master` at `ef9dff0 feat: add context capability helpers`. RED
failed as expected on missing `Context<T>::window()`, `Context<T>::entity(...)`,
and `Context<T>::weak_entity(...)` authoring helpers. GREEN adds `window()` as
a public `Window` facade alias for the current runtime window, renames the
low-level public platform-window field to `platform_window`, and adds template
entity/weak-entity helpers over the existing runtime entity store without
claiming full entity lifecycle parity yet. Feature-worktree focused
verification passed Windows 7/7 and WSL Arch Linux 11/11. Feature-worktree
full debug passed Windows 48/48 and WSL Arch Linux 45/45. Post-merge
verification passed Windows 48/48 and WSL Arch Linux 45/45.

Step 263, public typed view handle spelling, is merged on `master` at
`65d75ea feat: add typed view handles` and post-merge verified on Windows and
WSL Arch Linux. RED failed as expected on missing `cgpui::ViewHandle<T>`,
`cgpui::WeakViewHandle<T>`, `Context<T>::view<T>()`, and
`Context<T>::read_view(...)`. GREEN adds a focused
`include/cgpui/ui/view_handle.hpp` public leaf, moves the existing untyped
`WeakView` there, adds typed handle wrappers over `ViewId`, and adds current
view, weak-view, typed upgrade, and read-only lookup helpers in
`WindowRuntimeContext` templates. This is author-facing handle spelling over
the existing registry, not full view lifecycle parity yet. Feature-worktree
verification passed Windows focused 8/8, Windows full debug 49/49, and WSL
Arch Linux full debug 46/46. Post-merge verification passed Windows full debug
49/49 and WSL Arch Linux full debug 46/46. Step 264 is the next Phase B slice.

Step 264, public prelude authoring surface gate, is merged on `master` at
`e4ed147 feat: add public prelude authoring gate` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`cgpui/prelude.hpp`. GREEN added that public aggregate, keeps
`include/cgpui/cgpui.hpp` as a thin compatibility wrapper, and adds
`tests/api_parity/public_authoring_surface_test.cpp` so an author-facing view
can compile using only the public prelude for `Application`, `App`, `Window`,
`Context<T>`, `Render`, `IntoElement`, and typed view/entity handles. This
closes the Steps 259-264 public-name band before entity lifecycle work starts.
Feature-worktree verification passed Windows full debug 50/50 and WSL Arch
Linux full debug 47/47. Post-merge verification passed Windows full debug
50/50 and WSL Arch Linux full debug 47/47. Step 265 is the next Phase B slice.

Step 265, public entity lifecycle creation handles, is merged on `master` at
`f77f30a feat: add entity lifecycle creation handles` and post-merge verified
on Windows and WSL Arch Linux. RED failed as expected on missing
`Context<T>::new_entity<T>(...)`. GREEN adds the public creation spelling in
`WindowRuntimeContext` templates, returning `EntityHandle<T>` over the existing
runtime entity store, and keeps observation, transactions, deletion, and
cross-context rules out of this slice. `insert_entity_handle(...)` is the
handle-returning counterpart to the existing low-level `insert_entity(...)`.
Feature-worktree verification passed Windows full debug 51/51 and WSL Arch
Linux full debug 48/48. Post-merge verification passed Windows full debug
51/51 and WSL Arch Linux full debug 48/48. Step 266 is the next Phase B slice.

Step 266, weak entity handle semantics, is merged on `master` at
`d466a48 feat: add weak entity handle semantics`. RED
failed as expected on missing `WeakEntity<T>::upgrade(...)` and
`WeakEntity<T>::read(...)`. GREEN adds those public convenience methods in the
focused `include/cgpui/core/entity.hpp` leaf, returning
`std::optional<EntityHandle<T>>` and `const T*` soft-failure reads over the
existing `WindowRuntimeContext::upgrade_entity(...)` compatibility path. This
keeps observation, update transactions, deletion, and cross-context rules out
of Step 266. Feature-worktree verification passed Windows focused 10/10, WSL
Arch Linux focused 10/10, Windows full debug 52/52, and WSL Arch Linux full
debug 49/49. Post-merge verification passed Windows full debug 52/52 and WSL
Arch Linux full debug 49/49.

Step 267, public entity observation helpers, is merged on `master` at
`10d5c23 feat: add entity observation helpers`. RED failed as expected on
missing `EntityHandle<T>::observe(...)`,
`EntityHandle<T>::observe_subscription(...)`, and
`Context<T>::observe_entity(...)`. GREEN adds public handle convenience
methods in `include/cgpui/core/entity.hpp` and context forwarding methods in
`include/cgpui/ui/runtime_context.hpp` /
`include/cgpui/ui/runtime_templates.hpp`, adapting the existing
`observe_model(...)` storage to callback with `EntityHandle<T>` spelling.
Runtime coverage in `window_runtime_actions_test` verifies notifications
through `EntityHandle<T>::update(...)`. Feature-worktree verification passed
Windows focused 11/11, WSL Arch Linux focused 11/11, Windows full debug 53/53,
and WSL Arch Linux full debug 50/50. Post-merge verification passed Windows
full debug 53/53 and WSL Arch Linux full debug 50/50. Step 268, entity update
transactions, is the next Phase B slice.

Step 268, entity update transactions, is merged on `master` at
`84c4dfa feat: add entity update transactions`. RED failed as expected on
missing `Context<T>::update_entity(...)` and on
`EntityHandle<T>::update(...)` returning `bool` rather than `std::optional<R>`
for value-returning transactions. GREEN adds `Context<T>::update_entity(...)`
in the focused `runtime_context.hpp` / `runtime_templates.hpp` template
boundary and routes `EntityHandle<T>::update(...)` through it. Void callbacks
still return `bool`; value callbacks return `std::optional<R>`, callbacks may
receive the typed context, and missing entities soft-fail. Feature-worktree
verification passed Windows focused 12/12, WSL Arch Linux focused 12/12,
Windows full debug 54/54, and WSL Arch Linux full debug 51/51. Post-merge
verification passed Windows full debug 54/54 and WSL Arch Linux full debug
51/51. Step 269, entity invalidation semantics, is the next Phase B slice.

Step 269, entity invalidation semantics, is merged on `master` at
`df46612 feat: add entity invalidation helpers`. RED failed as expected on
missing `EntityHandle<T>::invalidate(...)` and
`Context<T>::invalidate_entity(...)`. GREEN adds the explicit invalidation
helpers in the focused public entity/context template boundaries and keeps
entity update transactions on the existing `notify_entity_changed(...)`
return semantics. Existing entities notify observers/subscribed views and
request redraw; missing or empty entities soft-fail. Deletion and
cross-context rules remain Step 270 work. Feature-worktree verification passed
Windows focused 13/13, WSL Arch Linux focused 13/13, Windows full debug 55/55,
and WSL Arch Linux full debug 52/52. Post-merge verification passed Windows
full debug 55/55 and WSL Arch Linux full debug 52/52. Step 270 entity deletion
and cross-context boundaries are the next Phase B slice.

Step 270, entity deletion and cross-context boundaries, is merged on `master`
at `90c05d7 feat: add entity deletion boundaries`. RED failed as expected on
missing `EntityHandle<T>::remove(...)` and
`Context<T>::remove_entity(EntityHandle<T>)`. GREEN adds public deletion
helpers in the focused entity/context template boundary, binds context-created
entity and weak handles to a runtime token, and preserves unbound
`EntityId<T>`-constructed handles for low-level compatibility. Empty, missing,
deleted, and wrong-runtime handles soft-fail without touching a same-numeric-id
entity in another runtime. Feature-worktree verification passed Windows focused
14/14, WSL Arch Linux focused 14/14, Windows full debug 56/56, and WSL Arch
Linux full debug 53/53. Post-merge verification passed Windows full debug
56/56 and WSL Arch Linux full debug 53/53. Step 271, GPUI-like context
capabilities by domain, is the next Phase B slice.

Step 271, app-context capability domain, is merged on `master` at
`29bc73a feat: add app context capability`. RED failed as expected on missing
`WindowRuntimeContext::app_context()`. GREEN adds
`Context<T>::app_context() -> AppContext`, reusing the existing app-domain
facade for app/global/theme/window-opening capability access from authoring
contexts. Runtime coverage verifies the app-context capability shares the same
root window and global state as the current frame context. Feature-worktree
verification passed Windows focused 12/12, WSL Arch Linux focused 12/12,
Windows full debug 57/57, and WSL Arch Linux full debug 54/54. Post-merge
verification passed Windows full debug 57/57 and WSL Arch Linux full debug
54/54. Step 272, the view-context capability domain, is the next Phase B slice.

Step 272, view-context capability domain, is merged on `master` at
`a65675a feat: add view context capability`. RED failed as expected on missing
`cgpui::ViewContextCapability<T>` and `Context<T>::view_context<T>()`. GREEN
adds the focused public leaf `include/cgpui/ui/view_context.hpp`, exposes
`Context<T>::view_context<T>() -> ViewContextCapability<T>`, and layers
`view_id()`, `view()`, `weak_view()`, `upgrade(...)`, `read(...)`, and
`current()` over the existing typed view handle/registry machinery. This slice
does not add new view lifecycle, observation, subscription, or runtime state.
Feature-worktree verification passed Windows focused 13/13, WSL Arch Linux
focused 13/13, Windows full debug 58/58, and WSL Arch Linux full debug 55/55.
Post-merge verification passed Windows full debug 58/58 and WSL Arch Linux
full debug 55/55. Step 273, the window-context capability domain, is the next
Phase B slice.

Step 273, window-context capability domain, is merged on `master` at
`c19939b feat: add window context capability`. RED failed as expected on
missing `cgpui::WindowContextCapability` and
`Context<T>::window_context()`. GREEN adds the focused public leaf
`include/cgpui/ui/window_context.hpp`, the focused implementation
`src/ui/window_context.cpp`, and
`Context<T>::window_context() -> WindowContextCapability`, grouping existing
`Window` facade queries plus render/layout/paint requests without adding new
native lifecycle state. Feature-worktree verification passed Windows focused
15/15, WSL Arch Linux focused 15/15, Windows full debug 59/59, and WSL Arch
Linux full debug 56/56. Post-merge verification passed Windows full debug
59/59 and WSL Arch Linux full debug 56/56. Step 274, the element-context
capability domain, is the next Phase B slice.

Step 274, element-context capability domain, is merged on `master` at
`fea090d feat: add element context capability`. RED failed as expected on
missing `cgpui::ElementContextCapability` and
`Context<T>::element_context(ElementId)`. GREEN adds the focused public leaf
`include/cgpui/ui/element_context.hpp`, the focused implementation
`src/ui/element_context.cpp`, and
`Context<T>::element_context(ElementId) -> ElementContextCapability`,
grouping existing element focus, keyboard focus, pointer capture/release,
cursor, focus-handle, and typed element state helpers without adding runtime
state or weakening entity runtime-token boundaries. Feature-worktree
verification passed Windows focused 16/16, WSL Arch Linux focused 16/16,
Windows full debug 60/60, and WSL Arch Linux full debug 57/57. Post-merge
verification passed Windows full debug 60/60 and WSL Arch Linux full debug
57/57. Step 275, the async-context capability domain, is the next Phase B
slice.

Step 275, async-context capability domain, is implemented and focused-verified
on `codex/phase-b-async-context-capability`. RED failed as expected on
missing `cgpui::AsyncContextCapability` and `Context<T>::async_context()`.
GREEN adds the focused public leaf `include/cgpui/ui/async_context.hpp`, the
focused implementation `src/ui/async_context.cpp`, and
`Context<T>::async_context() -> AsyncContextCapability`, grouping existing
defer, timer, animation, foreground task, background task, and update-batch
scheduling without moving executor state, weakening entity runtime-token
boundaries, or growing broad UI runtime files. Focused verification passed
Windows 13/13 and WSL Arch Linux 13/13. Full debug verification and merge
cleanup remain before marking the slice complete on `master`.

Step 139, keyed element identity and keyed reconciliation beyond parent-local
index matching, is merged on `master` at
`8695bb1 feat: add keyed element identity` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `ElementKey`,
`Element::key()`, `ElementBuilder::key(...)`, and
`ElementTree::reconcile_children(...)`. GREEN adds optional stable element
keys, builder key propagation through event/focus wrappers, and parent-local
batch reconciliation that preserves keyed child ids across
reorder/insert/remove while keeping index-based reconciliation available for
existing unkeyed call sites. The effective distance through Step 168 is 29
remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 140, element lifecycle hooks for mount, update, and unmount
notifications, is merged on `master` at
`a179f5a feat: add element lifecycle hooks` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`ElementLifecycleContext` and lifecycle hook APIs. GREEN adds no-op
`Element::on_mount(...)`, `on_update(...)`, and `on_unmount(...)` hooks plus
mount/update/unmount dispatch from root, index child, keyed child, subtree
removal, and `set_root(...)` replacement paths. The effective distance through
Step 168 is 28 remaining follow-on implementation steps plus the four
follow-on band checkpoint reviews.

Step 141, element state storage keyed by element id for reusable widgets, is
merged on `master` at `10415c6 feat: add element state storage` and
post-merge verified on Windows and WSL Arch Linux. RED failed as expected on
missing `ElementTree::state<T>(...)`, `state_or_init<T>(...)`, and
`emplace_state<T>(...)` APIs. GREEN adds per-node type-indexed state storage,
soft-fail lookup/init/replace helpers, and `WindowRuntime` /
`WindowRuntimeContext` forwarding for runtime-owned element trees. The
effective distance through Step 168 is 27 remaining follow-on implementation
steps plus the four follow-on band checkpoint reviews.

Step 142, style class and theme token primitives for reusable design
vocabulary, is merged on `master` at
`1493c91 feat: add style classes theme tokens` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing `StyleClassId`,
`style_class(...)`, `StyleClasses`, `ThemeTokenId`, `theme_token(...)`, and
`Theme` color/spacing token APIs. GREEN adds inert public style vocabulary
primitives: named class ids, ordered de-duplicated class lists, named theme
token ids, and typed color/spacing token storage with missing-token soft
failure. The effective distance through Step 168 is 26 remaining follow-on
implementation steps plus the four follow-on band checkpoint reviews.

Step 143, style cascade resolution combining base, class, state, and inline
styles, is merged on `master` at
`2ab43a7 feat: add style cascade resolution` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing `StyleCascade`,
class/inline builder APIs, and styled-element resolved-style APIs. GREEN adds
deterministic `StyleCascade` resolution plus `StyledElement` storage for
classes and inline overlays, while leaving runtime layout/paint cascade
installation for a later slice. The effective distance through Step 168 is 25
remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 144, `FocusHandle` primitive with request, release, contains, and focused
queries, is merged on `master` at
`874ef1f feat: add focus handle primitive` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `FocusHandle` and
`focus_handle(...)` APIs. GREEN adds a lightweight element-id focus handle,
runtime/context `focus_handle(...)` factories, focus request/release
forwarding, and query helpers over `ViewInputState` plus a public runtime
input snapshot. The effective distance through Step 168 is 24 remaining
follow-on implementation steps plus the four follow-on band checkpoint reviews.

Step 145, button widget primitive built from public element, focus, style, and
action APIs, is merged on `master` at
`da62f61 feat: add button widget primitive` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `cgpui::button`,
`ButtonElement`, and button/runtime action dispatch behavior. GREEN adds
`ButtonElement`, fluent `ButtonBuilder`, public `button(...)`, style-state
metadata, disabled/focusable/click/action behavior, and button style-box paint
metadata shared with styled elements. The effective distance through Step 168
is 23 remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 146, label widget primitive using text style and text paint commands, is
merged on `master` at `ca63320 feat: add label widget primitive` and
post-merge verified on Windows and WSL Arch Linux. RED failed as expected on
missing `cgpui::label` and `LabelElement` APIs. GREEN adds an owned-text
`LabelElement`, fluent `LabelBuilder`, public `label(...)`, text style
shortcuts, key/disabled builder support, and paint behavior that emits a text
command without caret or selection metadata. The effective distance through
Step 168 is 22 remaining follow-on implementation steps plus the four
follow-on band checkpoint reviews.

Step 147, text input widget primitive integrating focus, text model,
selection, clipboard, and IME geometry, is merged on `master` at
`e3f122b feat: add text input widget primitive` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`cgpui::text_input` and `TextInputElement` APIs. GREEN adds a focusable
`TextInputElement`, fluent `TextInputBuilder`, public `text_input(...)`,
text style/key/disabled builder support, focused text-model fallback through
installed text input elements, and runtime routing for text input, edit
bindings, clipboard operations, and IME geometry without requiring manual
`bind_text_model(...)`. The effective distance through Step 168 is 21
remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 148, scrollable list container with stable item keys and viewport
clipping metadata, is merged on `master` at
`447b74c feat: add scrollable list container` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`cgpui::scrollable_list` and `ScrollableListElement` APIs. GREEN adds
`ScrollableListElement`, fluent `ScrollableListBuilder`, public
`scrollable_list(ScrollState&)`, keyed item installation, scroll-state
viewport/content updates, offset-adjusted child bounds, and viewport clip
metadata around item paint commands. The effective distance through Step 168
is 20 remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 149, font database abstraction and platform font discovery skeleton for
Win32 and Linux, is merged on `master` at
`b415784 feat: add font database skeleton` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `FontDatabase`,
`FontFaceDescriptor`, `FontSource`, `discover_test_fonts(...)`, and
`PlatformApplication::discover_fonts()` APIs. GREEN adds a platform-neutral
font database and deterministic test discovery helper in `ui/text.hpp`, plus
gracefully empty Win32 and Wayland discovery overrides. The effective distance
through Step 168 is 19 remaining follow-on implementation steps plus the four
follow-on band checkpoint reviews.

Step 150, text shaping run abstraction with deterministic fallback metrics
before full shaping, is merged on `master` at
`d5a3c57 feat: add text shaping run abstraction` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing `TextShapeRun`,
`shape_text(...)`, and text element shaping APIs. GREEN adds `TextGlyphRun`,
`TextShapeRun`, UTF-8 codepoint-style fallback glyph grouping,
font-size-derived fallback advances, and `TextElement`/`LabelElement` layout
through shaping runs. The effective distance through Step 168 is 18 remaining
follow-on implementation steps plus the four follow-on band checkpoint
reviews.

Step 151, glyph atlas/cache interface shared by text elements and Vulkan
renderer, is merged on `master` at
`681513a feat: add glyph atlas cache interface` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing `GlyphCache`,
`GlyphAtlasKey`, `GlyphCacheLookup`, and `GlyphAtlasEntry` APIs. GREEN adds
deterministic glyph atlas keys, text-glyph paint metadata, a renderer-facing
cache/lookup/entry interface, and text paint commands that carry shaped glyph
metadata for later Vulkan consumption. The effective distance through Step 168
is 17 remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 152, Vulkan text draw path consumes text paint commands through cached
glyph metadata, is merged on `master` at
`367854b feat: consume text glyphs in vulkan renderer` and post-merge verified
on Windows and WSL Arch Linux. RED failed as expected on missing `TextDraw`,
`RenderFrame::draw_text(...)`, and `vulkan_consume_text_draw(...)` APIs. GREEN
adds a renderer text draw command, forwards UI text paint commands into frames,
and makes the Vulkan path consume glyph metadata through a persistent
`GlyphCache` without adding real glyph raster/upload yet. The effective
distance through Step 168 is 16 remaining follow-on implementation steps plus
the four follow-on band checkpoint reviews.

Step 153, opacity and transform paint metadata with deterministic command
ordering, is merged on `master` at
`8bf2270 feat: add opacity transform paint metadata` and post-merge verified
on Windows and WSL Arch Linux. RED failed as expected on missing
`AffineTransform`, `Style::opacity`, `Style::transform`, `PaintMetadata`, and
renderer command metadata APIs. GREEN adds shared affine transform metadata,
style/overlay opacity and transform fields, paint-list metadata scoping,
render-view forwarding for rect and text commands, and deterministic
parent/child metadata composition. The effective distance through Step 168 is
15 remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 154, renderer command batching by clip, opacity, transform, and primitive
kind, is merged on `master` at
`8f4a39e feat: add renderer command batching diagnostics` and post-merge
verified on Windows and WSL Arch Linux. RED failed as expected on missing
`RendererCommandBatch`, `RendererPrimitiveKind`, and
`vulkan_build_renderer_command_batches(...)` APIs. GREEN adds renderer-facing
batch keys over primitive kind, clip rect, and paint metadata, plus a Vulkan
diagnostic helper that groups adjacent rect/text commands without reordering or
optimizing GPU submission. The effective distance through Step 168 is 14
remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 155, frame timing and paint/layout/render statistics exposed through
diagnostics, is merged on `master` at
`12f5cb3 feat: add frame statistics diagnostics` and post-merge verified on
Windows and WSL Arch Linux. GREEN adds deterministic frame counters for render,
layout, paint, emitted/submitted/skipped commands, primitive counts, clear, and
present while keeping timing fields stable. The effective distance through
Step 168 is 13 remaining follow-on implementation steps plus the four
follow-on band checkpoint reviews.

Step 156, HiDPI scale propagation into layout, text metrics, and renderer
resources, is merged on `master` at `111ba8d feat: propagate hidpi scale` and
post-merge verified on Windows and WSL Arch Linux. GREEN keeps authored sizes
in logical pixels while deriving logical viewport size from framebuffer size
and scale, then threads device-scale text/glyph metadata to the Vulkan glyph
cache. The effective distance through Step 168 is 12 remaining follow-on
implementation steps plus the four follow-on band checkpoint reviews.

Step 157, snapshot tests for paint command streams emitted by the demo and
widgets, is merged on `master` at `cbda2e4 test: add paint command snapshots`
and post-merge verified on Windows and WSL Arch Linux. GREEN adds test-only
paint/render command serializers, widget paint snapshots, render-view submitted
command snapshots, and an inert demo paint-snapshot smoke marker. The effective
distance through Step 168 is 11 remaining follow-on implementation steps plus
the four follow-on band checkpoint reviews.

Step 158, renderer fallback path for unsupported commands with explicit
diagnostics, is merged on `master` at
`86a3e00 feat: add renderer unsupported diagnostics` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`RendererCommandStreamItem`, `RendererCommandReport`,
`RendererUnsupportedCommandDiagnostic`, unsupported primitive kinds, and
`vulkan_build_renderer_command_report(...)`. GREEN adds a renderer-facing
command report that batches supported solid/text commands while recording
explicit unsupported diagnostics for rounded-rect, text-selection, and
text-caret style primitives. The effective distance through Step 168 is 10
remaining follow-on implementation steps plus the final Band H checkpoint
review.

Step 159, multi-window runtime registry with per-window root view and renderer
ownership, is merged on `master` at
`7d515bb feat: add multi-window runtime registry` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`WindowRuntimeId`, `WindowRuntimeRecord`, `AppOpenedWindow::runtime_id`,
`WindowRuntime::window_runtime_records()`,
`WindowRuntime::window_runtime_record(...)`, and
`WindowRuntime::root_window_runtime_id()`. GREEN adds a platform-neutral
runtime registry that keeps the root window record active during the existing
single-window run path and records app-opened windows with independent root
view ids plus explicit window/renderer/root-view ownership metadata. The
effective distance through Step 168 is 9 remaining follow-on implementation
steps plus the final Band H checkpoint review.

Step 160, window activation, focus, minimize, restore, and close lifecycle
events, is merged on `master` at
`13a99ae feat: add window lifecycle events` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `WindowActivated`,
`WindowMinimized`, `WindowRestored`, and lifecycle `EventKind` values. GREEN
adds public lifecycle event structs, variant/event-kind mapping, and root-route
dispatch records plus after-event callback observability for
activate/minimize/restore/close while preserving the existing focused-event
view dispatch and close-request quit behavior. The effective distance through
Step 168 is 8 remaining follow-on implementation steps plus the final Band H
checkpoint review.

Step 161, Win32 IME composition window placement wired to focused text
geometry, is merged on `master` at `0a44fed feat: wire win32 ime placement`
and post-merge verified on Windows and WSL Arch Linux. RED failed as expected
on missing `ImeTextInputPlacement`,
`PlatformWindow::set_ime_text_input_placement(...)`, and
`WindowState::ime_text_input_placement`. GREEN adds a platform-neutral IME text
input placement snapshot, runtime propagation from focused text geometry,
Win32 IMM composition/candidate placement application, and Wayland state
storage for later text-input protocol wiring. The effective distance through
Step 168 is 7 remaining follow-on implementation steps plus the final Band H
checkpoint review.

Step 162, Wayland text-input/IME protocol skeleton wired to focused text
geometry, is merged on `master` at `23eb6e3 feat: add wayland ime skeleton`
and post-merge verified on Windows and WSL Arch Linux. RED failed as expected
on missing `ImeTextInputSupport`, `WindowState::ime_text_input_support`, and a
Wayland text-input skeleton. GREEN adds platform-neutral IME support state, a
Wayland `WaylandTextInput` skeleton that consumes focused text placement while
reporting graceful `unsupported` behavior without a protocol global, and Win32
support metadata for the existing IMM path. The effective distance through
Step 168 is 6 remaining follow-on implementation steps plus the final Band H
checkpoint review.

Step 163, Win32 drag-and-drop text/file event skeleton, is merged on `master`
at `0d90edd feat: add win32 drag drop skeleton` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`DragDropPayload`, `DragDropPayloadKind`, `DragEntered`, `DragUpdated`,
`DragDropped`, `DragExited`, drag `EventKind` values, and Win32 drag/drop
skeleton hooks. GREEN adds public text/file drag payload and event shapes,
runtime event-kind mapping plus hit routing by drag position, and Win32
deterministic `RegisterWindowMessageW` test hooks that translate text and file
payloads without claiming real shell drag/drop integration yet. The effective
distance through Step 168 is 5 remaining follow-on implementation steps plus
the final Band H checkpoint review.

Step 164, Wayland data-device drag-and-drop text/file event skeleton, is
merged on `master` at `e1f485e feat: add wayland data device dnd skeleton`
and post-merge verified on Windows and WSL Arch Linux. RED failed as expected
on missing Wayland data-device drag/drop dispatch and source-readiness
coverage. GREEN binds `wl_data_device_manager`, creates a seat data device,
maps Wayland enter/motion/drop/leave notifications to the public drag/drop
events, and keeps payload extraction as a graceful no-data skeleton. The
effective distance through Step 168 is 4 remaining follow-on implementation
steps plus the final Band H checkpoint review.

Step 165, platform event loop wakeup API for timers, async completions, and
deferred callbacks, is merged on `master` at
`1c7f665 feat: add platform event loop wakeup` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`WindowWakeupRequested` and platform wakeup APIs. GREEN adds
`WindowWakeupRequested`, `PlatformApplication::request_wakeup()`, runtime
wakeup requests from deferred callbacks, one-shot/repeating timers, and task
completions, plus a deterministic wakeup drain order of task completions,
timers, then deferred callbacks. The runtime treats wakeup handling as an
outer drain batch so all queued invalidations flush one redraw at the end.
Win32 posts a private thread wakeup message, Wayland polls a nonblocking pipe,
and the empty backend keeps a no-op default. The effective distance through
Step 168 is 3 remaining follow-on implementation steps plus the final Band H
checkpoint review.

Step 166, accessibility tree skeleton for labels, buttons, text inputs, and
focus state, is merged on `master` at
`57bb3aa feat: add accessibility tree skeleton` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`AccessibilityTreeSnapshot`, `AccessibilityNode`, `AccessibilityRole`,
`AccessibilitySnapshotOptions`, and `accessibility_snapshot(...)` APIs. GREEN
adds platform-neutral accessibility snapshot metadata over the `ElementTree`,
element role/name/text hooks for labels, buttons, text, and text inputs,
runtime/context snapshot helpers that mark the keyboard-focused element, and a
low-coupling platform update placeholder for future Windows UIA / Linux AT-SPI
adapters. The effective distance through Step 168 is 2 remaining follow-on
implementation steps plus the final Band H checkpoint review.

Step 167, Windows/Linux demo smoke tests covering window, input, text,
clipboard, redraw, and close flows, is merged on `master` at
`c83996b test: add windows linux demo smoke flows` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`CGPUI_DEMO_SMOKE_FLOW`, a bounded full-flow demo smoke marker, clipboard/text
runtime coverage in the demo, and platform-specific xmake smoke tests. GREEN
adds a deterministic `CGPUI_DEMO_SMOKE_FLOW` path to `examples/hello_window`
that binds a memory clipboard, exercises text input, focused text mutation,
clipboard paste/copy, redraw, and close after the second frame, plus
`windows_demo_smoke_flow` and `linux_demo_smoke_flow` test entries. The
effective distance through Step 168 is 1 remaining follow-on implementation
slice plus the final Band H checkpoint review.

Step 168, GPUI-core API parity audit document with remaining gaps and Mac
parity handoff boundaries, is merged on `master` at
`c16689e docs: add gpui core api parity audit` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`docs/gpui-core-api-parity.md`; direct binary verification returned exit code
30 for the missing audit document. GREEN adds
`docs/gpui-core-api-parity.md`, separating implemented, partial, missing, and
Mac/Metal-deferred areas with an explicit Windows/Linux completion lens and
an explicit "not full upstream GPUI parity" boundary. The Step 129-168
follow-on goal is now complete on the Windows/Linux track.

- Band E, Steps 129-138: GPUI-like context, entity, global state, action
  scoping, subscriptions, and async/timer primitives.
- Band F, Steps 139-148: keyed reconciliation, element lifecycle hooks,
  style classes/themes, focus handles, and reusable widget primitives.
- Band G, Steps 149-158: text/font rendering maturity, glyph cache surfaces,
  renderer command batching, opacity/transform, and frame diagnostics.
- Band H, Steps 159-168: Windows/Wayland platform completion, multi-window
  lifecycle, accessibility/debug surfaces, packaging checks, and API parity
  documentation.

## Steps

1. [x] EventResult: make view event handlers return consumed/cancelled state.
2. [x] Event dispatch observability: expose last event result to runtime tests and callbacks where useful.
3. [x] Event routing shell: add a root-level dispatch function that can later route to element/view ids.
4. [x] EntityId and EntityStore skeleton: typed ids, insert/get/remove lifecycle.
5. [x] Context entity access: expose safe entity mutation/read helpers through a context object.
6. [x] View identity allocation: allocate stable ids beyond the root view.
7. [x] ElementId and Element base API: minimal polymorphic element contract.
8. [x] Element tree container: store children, parent links, and root element id.
9. [x] Element reconcile pass: preserve ids across simple rebuilds.
10. [x] LayoutInput/LayoutOutput primitives: constraints, size, origin.
11. [x] Layout pass for fixed-size leaf elements.
12. [x] Layout pass for simple vertical stack.
13. [x] Hit testing over laid-out element bounds.
14. [x] Pointer routing to hit-tested element id.
15. [x] Pointer capture routing to owner id.
16. [x] Keyboard routing to focus owner id.
17. [x] Action/Command primitive: named actions and dispatch result.
18. [x] Key binding table: map key events to actions.
19. [x] Basic text model: editable UTF-8 buffer with cursor.
20. [x] Text input routing into focused text model.
21. [x] Style primitives: colors, size, padding, and border fields.
22. [x] Element builder API skeleton for composing styled elements.
23. [x] Styled element paints a solid background rect.
24. [x] Padding participates in styled element layout.
25. [x] Border width/color and border radius primitives.
26. [x] Paint tree generation from element hierarchy.
27. [x] Clip rect and overflow primitives.
28. [x] Z-order and deterministic child paint order.
29. [x] Hover state tracking for routed pointer targets.
30. [x] Cursor shape API and routed cursor state.
31. [x] Scroll state/model primitive.
32. [x] Flex row/column layout basics.
33. [x] Runtime invalidation requests for layout and paint.
34. [x] Update scheduling that requests redraw after model/view changes.
35. [x] View/model subscription relation skeleton.
36. [x] Text selection range model.
37. [x] Text key editing actions for delete, movement, and selection.
38. [x] Clipboard abstraction for Win32 and Wayland.
39. [x] IME composition skeleton for Win32 and Wayland.
40. [x] Real interactive demo using the new API surface.
41. [x] ElementTree root layout helper.
42. [x] ElementTree root hit-test helper.
43. [x] Element tree owned runtime root installation.
44. [x] Runtime lays out installed element trees on redraw.
45. [x] Element event handler hook.
46. [x] Runtime dispatches routed element events before view fallback.
47. [x] Element builder flex row/column helpers.
48. [x] Element builder vertical stack helper.
49. [x] Element builder fixed-size helper.
50. [x] Gap style primitive for stack and flex layout.
51. [x] Margin style primitive and outer layout sizing.
52. [x] Border paint emission from styled elements.
53. [x] Hidden overflow clip metadata in paint commands.
54. [x] Text element skeleton bound to TextModel.
55. [x] Runtime text edit action routing for focused text models.
56. [x] Runtime clipboard paste into focused text models.
57. [x] Runtime clipboard copy from focused text selections.
58. [x] Runtime clipboard cut from focused text selections.
59. [x] ViewContext convenience wrapper for common runtime APIs.
60. [x] Public app runner skeleton for GPUI-like startup.
61. [x] Text element builder helper.
62. [x] Focusable element activation hook.
63. [x] Runtime clicks request focus for focusable elements.
64. [x] Element enabled/disabled state primitive.
65. [x] Disabled elements skip event handling.
66. [x] Element tree preorder traversal helper.
67. [x] Element tree find-by-id convenience helper.
68. [x] ViewContext text model binding helper.
69. [x] ViewContext element tree installation helper.
70. [x] ViewContext action registration helper.
71. [x] ViewContext key binding helper.
72. [x] ViewContext text edit binding helper.
73. [x] ViewContext cursor binding helper.
74. [x] ViewContext focus request/release element helpers.
75. [x] ViewContext pointer capture element helpers.
76. [x] Element builder click handler helper.
77. [x] Element builder focusable helper.
78. [x] Element builder key handler helper.
79. [x] Element builder disabled convenience helper.
80. [x] Element tree enabled descendant traversal query.
81. [x] Runtime skips disabled elements for focus activation.
82. [x] Runtime clears hover cursor when hovered element becomes disabled.
83. [x] Runtime text model lookup helper for focused element.
84. [x] ViewContext focused text model mutation helper.
85. [x] Public prelude header for core authoring APIs.
86. [x] ViewContext event route access helper.
87. [x] ViewContext input state access helper.
88. [x] Mac readiness audit and desktop target mapping.
89. [x] Public `AnyElement` alias and minimal into-element authoring convention.
90. [x] Free authoring factories for `div()`, `h_flex()`, `v_flex()`, `v_stack()`, and `text(...)`.
91. [x] Element builder child overloads for builders, `AnyElement`, and element ownership.
92. [x] Style unit and color helpers: `px`, `rgb`, `rgba`, and edge constructors.
93. [x] Element builder fluent style shortcuts for size, spacing, background, border, radius, and gap.
94. [x] Pointer handler shortcuts for down, up, move, and click authoring paths.
95. [x] Focus, hover, and disabled style-state overlay primitives.
96. [x] GPUI-like `View::render(ViewContext&)` hook skeleton while preserving the current view contract.
97. [x] Runtime render pass installs the root view's rendered element tree.
98. [x] ViewContext render invalidation helper and after-render observability.
99. [x] Public `Model<T>`/`Entity<T>` authoring aliases over typed entity ids.
100. [x] ViewContext model helpers for create, read, update, and remove.
101. [x] Weak entity/view handle primitives with soft-fail upgrade.
102. [x] ViewContext observe/subscribe callback helper for model changes.
103. [x] Model update notification automatically invalidates subscribed views.
104. [x] Public `AppContext` wrapper over the app runner setup phase.
105. [x] `WindowOptions` builder and `AppContext::open_window(...)` helper skeleton.
106. [x] Runtime root view lifecycle storage for app-opened windows.
107. [x] View registry skeleton for multiple view ids beyond the root.
108. [x] Child-view element placeholder that embeds another view's rendered output.
109. [x] Event route carries element and view ancestry metadata.
110. [x] Event propagation phases: target handling then ancestor bubbling before view fallback.
111. [x] Focus traversal over enabled focusable elements with Tab and Shift+Tab actions.
112. [x] Scroll element binding helper backed by `ScrollState`.
113. [x] Wheel and trackpad scroll routing into bound scroll state.
114. [x] Hidden overflow participates in hit testing, not only paint clip metadata.
115. [x] Flex alignment and justification primitives.
116. [x] Flex grow and shrink factors for child layout.
117. [x] Absolute positioning and inset style primitive.
118. [x] Layer/elevation style primitive mapped onto deterministic z order.
119. [x] Rounded-rect paint command that preserves border radius metadata.
120. [x] Vulkan renderer honors clip rect metadata for solid rectangles.
121. [x] Text paint command separates text drawing from placeholder rectangles.
122. [x] Font descriptor and basic font-size style primitives.
123. [x] Text element emits caret and selection paint metadata.
124. [x] Platform cursor application for Win32 and Wayland.
125. [x] Win32 system clipboard backend for text copy, cut, and paste.
126. [x] Wayland system clipboard backend skeleton for text copy, cut, and paste.
127. [x] IME composition/candidate rectangle data from the focused text element.
128. [x] GPUI-like demo rewrite using the public prelude and new authoring API.
129. [x] Public `Context<T>` authoring alias over `ViewContext` for view/model code.
130. [x] Entity handle API with `read`, `update`, and `downgrade` convenience methods.
131. [x] Global app state registry with typed `set_global`, `global`, and `update_global` helpers.
132. [x] Scoped action registry for app, window, view, and focused element actions.
133. [x] Subscription ownership token that disconnects observers on drop/removal.
134. [x] Deferred callback queue for `cx.defer(...)` style post-event work.
135. [x] Timer API for one-shot and repeating callbacks through the runtime loop.
136. [x] Async task handle skeleton with main-thread completion dispatch.
137. [x] Runtime update batching so multiple model/global changes coalesce redraws.
138. [x] Public diagnostics snapshot for entities, subscriptions, invalidations, and frames.
139. [x] Keyed element identity and keyed reconciliation beyond parent-local index matching.
140. [x] Element lifecycle hooks for mount, update, and unmount notifications.
141. [x] Element state storage keyed by element id for reusable widgets.
142. [x] Style class and theme token primitives for reusable design vocabulary.
143. [x] Style cascade resolution combining base, class, state, and inline styles.
144. [x] `FocusHandle` primitive with request, release, contains, and focused queries.
145. [x] Button widget primitive built from public element, focus, style, and action APIs.
146. [x] Label widget primitive using text style and text paint commands.
147. [x] Text input widget primitive integrating focus, text model, selection, clipboard, and IME geometry.
148. [x] Scrollable list container with stable item keys and viewport clipping metadata.
149. [x] Font database abstraction and platform font discovery skeleton for Win32 and Linux.
150. [x] Text shaping run abstraction with deterministic fallback metrics before full shaping.
151. [x] Glyph atlas/cache interface shared by text elements and Vulkan renderer.
152. [x] Vulkan text draw path consumes text paint commands through cached glyph metadata.
153. [x] Opacity and transform paint metadata with deterministic command ordering.
154. [x] Renderer command batching by clip, opacity, transform, and primitive kind.
155. [x] Frame timing and paint/layout/render statistics exposed through diagnostics.
156. [x] HiDPI scale propagation into layout, text metrics, and renderer resources.
157. [x] Snapshot tests for paint command streams emitted by the demo and widgets.
158. [x] Renderer fallback path for unsupported commands with explicit diagnostics.
159. [x] Multi-window runtime registry with per-window root view and renderer ownership.
160. [x] Window activation, focus, minimize, restore, and close lifecycle events.
161. [x] Win32 IME composition window placement wired to focused text geometry.
162. [x] Wayland text-input/IME protocol skeleton wired to focused text geometry.
163. [x] Win32 drag-and-drop text/file event skeleton.
164. [x] Wayland data-device drag-and-drop text/file event skeleton.
165. [x] Platform event loop wakeup API for timers, async completions, and deferred callbacks.
166. [x] Accessibility tree skeleton for labels, buttons, text inputs, and focus state.
167. [x] Windows/Linux demo smoke tests covering window, input, text, clipboard, and redraw flows.
168. [x] GPUI-core API parity audit document with remaining gaps and Mac parity handoff boundaries.
169. [x] Glyph bitmap/raster data model and deterministic fallback glyph rasterizer.
170. [x] Glyph atlas page allocation, slot packing, and upload-record API.
171. [x] Vulkan textured glyph quad command generation from atlas entries.
172. [x] Vulkan text render report distinguishes glyph-backed draw preparation from metadata placeholders.
173. [x] Wayland clipboard MIME offer/send/receive test-compositor path with text payload extraction.
174. [x] Wayland drag/drop MIME payload extraction for text and URI-list/file payloads.
175. [x] Wayland text-input state machine for enter/leave, surrounding text, preedit, and commit.
176. [x] Windows UIA accessibility adapter skeleton consuming `AccessibilityTreeSnapshot`.
177. [x] Linux AT-SPI accessibility adapter skeleton consuming `AccessibilityTreeSnapshot`.
178. [x] Native additional-window creation slice over the multi-window runtime registry.
179. [x] Vulkan glyph atlas image descriptors and upload-batch planning.
180. [x] Vulkan glyph atlas texture resource lifetime skeleton.
181. [x] Vulkan glyph upload dirty-range tracking.
182. [x] Vulkan text sampler pipeline descriptor and readiness report.
183. [x] Vulkan rounded-rect tessellation records.
184. [x] Vulkan text selection and caret geometry records.
185. [x] Renderer clip stack metadata beyond single optional clips.
186. [x] Renderer opacity and transform stack reports.
187. [x] Renderer batch submission plan records.
188. [x] Renderer frame snapshot report aggregating text, uploads, batches, and gaps.
189. [x] Font fallback chain resolution.
190. [x] Platform font discovery records for Win32 and Wayland/Linux.
191. [x] Grapheme-aware cursor movement skeleton.
192. [x] Word movement and selection actions.
193. [x] Text undo and redo stack.
194. [x] IME delete-surrounding text action.
195. [x] Multiline text model and line navigation.
196. [x] Text measurement cache.
197. [x] Text pointer selection geometry.
198. [x] Soft wrap layout records.
199. [x] Wayland clipboard ownership and send offers.
200. [x] Wayland drag action negotiation.
201. [x] Wayland cursor theme image state.
202. [x] Wayland XDG configure lifecycle state.
203. [x] Win32 OLE drop target skeleton.
204. [x] Native menu and accelerator API skeleton.
205. [x] Native file dialog API skeleton.
206. [x] Window chrome customization skeleton.
207. [x] App command palette registry.
208. [x] Platform diagnostics event stream.
209. [x] UIA provider tree facade.
210. [x] AT-SPI object model facade.
211. [x] Accessibility value and live update events.
212. [x] Additional window renderer ownership.
213. [x] Additional window event routing.
214. [x] Additional window lifecycle cleanup.
215. [x] Runtime theme inheritance and switching.
216. [x] Animation clock and tween primitives.
217. [x] Asset and image pipeline skeleton.
218. [x] Threaded async executor and cancellation.

## Active Step

Current handoff: Steps 179-218 are the active Windows/Linux production-depth
pass in
`docs/superpowers/plans/2026-07-02-gpui-core-depth-steps-179-218-plan.md`.
Step 218, threaded async executor and cancellation, is merged on `master` at
`a8ebfec feat: add threaded async executor` and post-merge verified on Windows
and WSL Arch Linux. The Steps 179-218 Windows/Linux production-depth pass is
now complete; macOS/Cocoa + Metal is still deferred to a separate parity run.

Post-Step-218 structural refactor, UI runtime structure split, is merged on
`master` at `6c30f7a refactor: split ui runtime structure` and post-merge
verified on Windows and WSL Arch Linux. This refactor keeps the public API
source-compatible through `include/cgpui/ui/ui.hpp`, moves public UI
declarations into `paint.hpp`, `view.hpp`, and `runtime.hpp`, and splits the
former `src/ui/ui.cpp` monolith into focused implementation units guarded by
`ui_source_structure_test`. It does not add new GPUI behavior; it makes the next
Windows/Linux or macOS parity slice easier to localize.

Step 218, threaded async executor and cancellation, is merged on `master` at
`a8ebfec feat: add threaded async executor`. RED failed as expected on missing
`TaskCancellationToken`, `spawn_background_task(...)`, task handle
cancel/cancelled queries, and runtime task diagnostics counters. GREEN adds a
small `std::jthread`-backed background task executor, cooperative atomic
cancellation tokens, `TaskHandle::cancel()` / `cancelled()`, mutex-protected
task registry and completion queue state, runtime diagnostics counters for
active/queued/completed/cancelled/background tasks, destructor cancellation and
join cleanup, and main-runtime completion dispatch through the existing task
queue and platform wakeup path. It also refreshes the GPUI-core parity audit
to move threaded async out of Missing while keeping full task pools,
priorities, async I/O integration, and cross-thread entity access as future
work. Feature-worktree targeted tests passed 4/4 on Windows and WSL Arch
Linux, `git diff --check` exited 0 with only expected CRLF warnings, WSL full
debug passed 27/27, and Windows full debug passed 30/30. Post-merge targeted
tests passed 4/4 on Windows and WSL Arch Linux, `git diff --check` produced no
output, WSL full debug passed 27/27, and Windows full debug passed 30/30. The
Steps 179-218 production-depth pass is complete on the Windows/Linux track.

Step 217, asset and image pipeline skeleton, is merged on `master` at
`7b37744 feat: add asset image pipeline skeleton`. RED failed as expected on
missing image asset descriptors, decoded RGBA8 bitmap records, image paint
commands, Vulkan upload planning APIs, renderer image primitive reporting, and
render-view image forwarding. GREEN adds deterministic in-memory bitmap asset
descriptors, image paint commands, `RenderFrame::draw_image(...)`, frame image
statistics, renderer image primitive/report records, and Vulkan-side image
upload batch/region planning without external decoders or real GPU texture
allocation. Feature-worktree targeted tests passed 4/4 on Windows and 3/3 on
WSL Arch Linux, `git diff --check` exited 0 with only expected CRLF warnings,
WSL full debug passed 27/27, and Windows full debug passed 30/30. Post-merge
targeted tests passed 4/4 on Windows and 3/3 on WSL Arch Linux,
`git diff --check` produced no output, WSL full debug passed 27/27, and
Windows full debug passed 30/30. Step 218, threaded async executor and
cancellation, is the next implementation slice.

Step 216, animation clock and tween primitives, is merged on `master` at
`f9e2f85 feat: add animation clock tween primitives`. RED failed as expected
first on missing easing/tween style primitives and then on unresolved runtime
animation symbols. GREEN adds `AnimationEasing`, progress clamping, easing,
float/color/transform/style tween helpers, `StyleTween`, `AnimationId`,
`AnimationOptions`, `AnimationSnapshot`, `AnimationHandle`, runtime
start/snapshot/cancel APIs, context forwarding, and deterministic timer-driven
animation ticks that expose eased progress, completion, cancellation, and
redraw behavior. Feature-worktree targeted tests passed 3/3 on Windows and WSL
Arch Linux, `git diff --check` exited 0 with only expected CRLF warnings, WSL
full debug passed 27/27, and Windows full debug passed 30/30. Post-merge
targeted tests passed 3/3 on Windows and WSL Arch Linux, `git diff --check`
produced no output, WSL full debug passed 27/27, and Windows full debug passed
30/30. Step 217, asset and image pipeline skeleton, is the next implementation
slice.

Step 215, runtime theme inheritance and switching, is merged on `master` at
`46f96e0 feat: add runtime theme switching`. RED failed as expected when the
runtime exposed theme token primitives but no app/window theme slots,
inherited token lookup, or theme-switch invalidation API. GREEN adds
runtime-owned app theme storage, per-`WindowRuntimeId` theme overrides,
window-then-app color/spacing token resolution, `AppContext` and
`WindowRuntimeContext` forwarding, and context-level `window_runtime_id`
propagation for root and child runtime records. Theme changes now request full
render/layout/paint invalidation and redraw; explicit invalidation clearing
also clears pending redraw bookkeeping so tests and diagnostics can reset the
observable scheduling state between changes. Feature-worktree targeted tests
passed 3/3 on Windows and WSL Arch Linux, `git diff --check` exited 0 with
only expected CRLF warnings, WSL full debug passed 27/27, and Windows full
debug passed 30/30. Post-merge targeted tests passed 3/3 on Windows and WSL
Arch Linux, `git diff --check` produced no output, WSL full debug passed
27/27, and Windows full debug passed 30/30. Step 216, animation clock and
tween primitives, is the next implementation slice.

Step 214, additional window lifecycle cleanup, is merged on `master` at
`437ef5e feat: clean up additional window lifecycle`. RED failed as expected
on marker 55 when child-window close left the owned root view alive and
registered after close. GREEN moves child close through deterministic cleanup:
the lifecycle dispatch is recorded first, the child native window ownership is
removed from the runtime-owned native-window list, the owned child root view is
removed and destroyed, view subscriptions are erased, and the child runtime
record releases its window, renderer, active, and ownership fields while
remaining available as historical record metadata. The existing child routing
test now snapshots child-view counters before close because close legitimately
destroys the owned child root. Feature-worktree targeted tests passed 3/3 on
Windows and WSL Arch Linux, `git diff --check` exited 0 with only expected
CRLF warnings, WSL full debug passed 27/27, and Windows full debug passed
30/30. Post-merge targeted tests passed 3/3 on Windows and WSL Arch Linux,
`git diff --check` produced no output, WSL full debug passed 27/27, and
Windows full debug passed 30/30. Step 215, runtime theme inheritance and
switching, is the next implementation slice.

Step 213, additional window event routing, is merged on `master` at
`d8b86fc feat: route additional window events`. RED failed as expected when a
child platform callback could only update resize/close-active metadata and did
not route child focus, pointer, keyboard, redraw, resize, or close behavior by
`WindowRuntimeId`. GREEN adds record-specific runtime contexts, child redraw
handling through the child renderer and root view, child resize forwarding to
the child renderer, child lifecycle dispatch records, and view-event dispatch
to the child root view. Feature-worktree targeted tests passed 4/4 on Windows
and WSL Arch Linux, `git diff --check` exited 0 with only expected CRLF
warnings, WSL full debug passed 27/27, and Windows full debug passed 30/30.
Post-merge targeted tests passed 4/4 on Windows and WSL Arch Linux, the
`git diff --check` check produced no output, WSL full debug passed 27/27, and
Windows full debug passed 30/30. Step 214, additional window lifecycle
cleanup, is the next implementation slice.

Step 212, additional window renderer ownership, is merged on `master` at
`701a2f4 feat: add additional window renderer ownership`. RED failed as
expected when app-opened child windows still owned native windows but exposed
null child `renderer` pointers and only invoked the app renderer factory for
the root window. GREEN makes `run_app` retain all renderers created through the
app renderer factory, creates app-opened child renderers from each child native
surface and window state, and preserves graceful per-record
`native_window_error` storage when child renderer creation fails. Feature
worktree targeted tests passed 3/3 on Windows and WSL Arch Linux,
`git diff --check` exited 0 with only expected CRLF warnings, WSL full debug
passed 27/27, and Windows full debug passed 30/30. Post-merge targeted tests
passed 3/3 on Windows and WSL Arch Linux, `git diff --check` produced no
output, WSL full debug passed 27/27, and Windows full debug passed 30/30. Step
213, additional window event routing, is the next implementation slice.

Step 211, accessibility value and live update events, is merged on `master` at
`1e51aa8 feat: add accessibility live update records`. RED failed as expected
on missing `PlatformAccessibilityLiveUpdate`,
`PlatformAccessibilityLiveUpdateKind`, and
`PlatformAccessibilityTreeUpdate::live_updates` API. GREEN adds
platform-neutral live update records for value, text, and focus changes,
generates them by comparing the latest platform accessibility update against
the previous one, and lets the Win32 UIA and Wayland AT-SPI facades retain the
latest live update batch without emitting production UIA/AT-SPI events.
Feature-worktree targeted tests passed 5/5 on Windows and WSL Arch Linux,
`git diff --check` exited 0 with only expected CRLF warnings, WSL full debug
passed 27/27, and Windows full debug passed 30/30. Post-merge targeted tests
passed 5/5 on Windows and WSL Arch Linux, `git diff --check` produced no
output, WSL full debug passed 27/27, and Windows full debug passed 30/30. Step
212, additional window renderer ownership, is the next implementation slice.

Step 210, AT-SPI object model facade, is merged on `master` at
`069a997 feat: add linux atspi object facade`. RED failed as expected on
missing Wayland AT-SPI object facade source markers, with
`wayland_window_source_test` returning the new 75 marker. GREEN adds
`WaylandAtspiObjectNode` records and stable
`/org/a11y/atspi/accessible/<element-id>` object paths over the existing
`PlatformAccessibilityTreeUpdate`, retaining parent ids/object paths,
role/name/text/value, enabled/focusable/focused state, bounds, and child
counts without exposing D-Bus provider types. Feature-worktree targeted tests
passed 4/4 on Windows and WSL Arch Linux, `git diff --check` exited 0 with
only expected CRLF warnings, WSL full debug passed 27/27, and Windows full
debug passed 30/30. Post-merge targeted tests passed 4/4 on Windows and WSL
Arch Linux, `git diff --check` produced no output, WSL full debug passed
27/27, and Windows full debug passed 30/30 after isolating a transient
`clipboard_test/default` batch failure with a passing targeted rerun. Step
211, accessibility value and live update events, is the next implementation
slice.

Step 209, UIA provider tree facade, is merged on `master` at
`2eb0749 feat: add win32 uia provider facade`. RED failed as expected on
missing `PlatformAccessibilityNodeUpdate::value` and Win32 UIA provider-node
facade source markers. GREEN adds a platform accessibility `value` field for
text-input nodes, maps it from the shared accessibility snapshot, and gives
the Win32 UIA adapter an internal `Win32UiaProviderNode` facade retaining
stable node ids, parent ids, role/name/text/value, enabled/focus/focusable
state, bounds, and child counts without creating COM provider objects yet.
Feature-worktree targeted tests passed 4/4 on Windows and WSL Arch Linux,
`git diff --check` exited 0 with only expected CRLF warnings, WSL full debug
passed 27/27, and Windows full debug passed 30/30. Post-merge targeted tests
passed 4/4 on Windows and WSL Arch Linux, `git diff --check` produced no
output, WSL full debug passed 27/27, and Windows full debug passed 30/30.
Step 210, AT-SPI object model facade, is the next implementation slice.

Step 208, platform diagnostics event stream, is merged on `master` at
`6ff6b0f feat: add platform diagnostics stream`. RED failed as expected on
missing `PlatformDiagnosticEvent`, `PlatformDiagnosticKind`, snapshot
diagnostics storage, and runtime stream APIs. GREEN lifts `EventKind` to the
core event boundary, adds platform diagnostic event metadata, surfaces a
bounded 32-event diagnostics stream through `WindowRuntime`,
`WindowRuntimeContext`, and `RuntimeDiagnosticsSnapshot`, and records
clipboard, drag/drop, IME placement, accessibility tree, lifecycle,
native-menu, and file-dialog platform-facing hooks. Feature-worktree targeted
tests passed 5/5 on Windows and WSL Arch Linux, `git diff --check` exited 0
with only expected CRLF warnings, WSL full debug passed 27/27, and Windows
full debug passed 30/30. Post-merge targeted tests passed 5/5 on Windows and
WSL Arch Linux, `git diff --check` produced no output, WSL full debug passed
27/27, and Windows full debug passed 30/30. Step 209, UIA provider tree
facade, is the next implementation slice.

Step 207, app command palette registry, is merged on `master` at
`92e03ea feat: add command palette registry`. RED failed as expected on
missing `CommandPaletteEntry` and command-palette registration/query/dispatch
APIs. GREEN adds `CommandPaletteEntry` metadata, runtime/AppContext/ViewContext
registration, stable registry and group queries, disabled-command handling, and
dispatch through the existing scoped action registry. Feature-worktree targeted
tests passed 3/3 on Windows and WSL Arch Linux, `git diff --check` exited 0
with only expected CRLF warnings, WSL full debug passed 27/27, and Windows full
debug passed 30/30. Post-merge targeted tests passed 3/3 on Windows and WSL
Arch Linux, `git diff --check` produced no output, WSL full debug passed
27/27, and Windows full debug passed 30/30 after isolating a transient
`clipboard_test/default` batch failure with a passing targeted rerun and
full-suite rerun. Step 208, platform diagnostics event stream, is the next
implementation slice.

Step 206, window chrome customization skeleton, is merged on `master` at
`6387371 feat: add window chrome customization skeleton`. RED failed as
expected on missing `WindowOptions::titlebar_visible(...)`, chrome descriptor
storage, and Win32/Wayland platform chrome markers. GREEN adds
`WindowChromeOptions` to `WindowDescriptor`, fluent `WindowOptions`
titlebar/decorations/resizable/transparent helpers, `PlatformWindowChromeState`
and a default unsupported `PlatformWindow::apply_window_chrome(...)` hook,
Win32 style/ex-style application skeletons, Wayland unsupported xdg-decoration
diagnostics, and header-cleanliness coverage. Feature-worktree targeted tests
passed 5/5 on Windows and WSL Arch Linux, `git diff --check` exited 0 with only
expected CRLF warnings, WSL full debug passed 27/27, and Windows full debug
passed 30/30. Post-merge targeted tests passed 5/5 on Windows and WSL Arch
Linux, `git diff --check` produced no output, WSL full debug passed 27/27, and
Windows full debug passed 30/30. Step 207, app command palette registry, is the
next implementation slice.

Step 205, native file dialog API skeleton, is merged on `master` at
`a7a2ac5 feat: add native file dialog skeleton`. RED failed as expected on
missing platform-neutral file dialog request/result/filter/kind types and
runtime/platform `show_native_file_dialog(...)` forwarding. GREEN adds
`NativeFileDialogKind`, `NativeFileDialogFilter`, `NativeFileDialogOptions`,
`NativeFileDialogResult`, `AppContext`/`WindowRuntime` dialog forwarding,
retained last dialog results, and inert Win32/Wayland skeleton diagnostics that
report backend, requested kind, and filter count while remaining unsupported.
Feature-worktree targeted tests passed 5/5 on Windows and WSL Arch Linux,
`git diff --check` exited 0 with only expected CRLF warnings, WSL full debug
passed 27/27, and Windows full debug passed 30/30. Post-merge targeted tests
passed 5/5 on Windows and WSL Arch Linux, `git diff --check` produced no
output, WSL full debug passed 27/27, and Windows full debug passed 30/30. Step
206, window chrome customization skeleton, is the next implementation slice.

Step 204, native menu and accelerator API skeleton, is merged on `master` at
`25c5e5f feat: add native menu accelerator skeleton`. RED failed as expected
on missing platform-neutral native menu model, accelerator descriptor,
installation result, runtime forwarding, and Win32/Wayland platform hook APIs.
GREEN adds `NativeMenuModel`, `NativeMenuItem`, `NativeMenuAccelerator`,
`PlatformMenuInstallationResult`, `NativeMenuInstallation`, recursive
item/accelerator counters, `AppContext`/`WindowRuntime` installation helpers,
and inert Win32/Wayland backend diagnostics that report item and accelerator
counts while remaining unsupported. Feature-worktree targeted tests passed 5/5
on Windows and WSL Arch Linux, `git diff --check` exited 0 with only expected
CRLF warnings, WSL full debug passed 27/27, and Windows full debug passed 30/30
after isolating a transient `clipboard_test/default` batch failure with a
passing targeted rerun and full-suite rerun. Post-merge targeted tests passed
5/5 on Windows and WSL Arch Linux, `git diff --check` produced no output, WSL
full debug passed 27/27, and Windows full debug passed 30/30 after the same
clipboard targeted/rerun handling. Step 205, native file dialog API skeleton,
is the next implementation slice.

Step 203, Win32 OLE drop target skeleton, is merged on `master` at
`3f26a33 feat: add win32 ole drop target skeleton`. RED failed as expected on
missing Win32 OLE drop-target registration markers and missing drag action
metadata in the existing deterministic Win32 drag/drop test hook. GREEN adds an
internal `Win32OleDropTarget` implementing `IDropTarget`, OLE initialization,
`RegisterDragDrop`/`RevokeDragDrop` registration diagnostics, text/file
`IDataObject` payload conversion boundaries, and `DROPEFFECT_COPY`/`MOVE`
mapping into public `DragDropAction` metadata. The Win32 test hook now carries
drop effects so existing deterministic drag-enter/update/drop coverage checks
copy/move actions without depending on a real desktop drag gesture. Feature
worktree targeted tests passed 5/5 on Windows and 4/4 on WSL Arch Linux,
`git diff --check` exited 0 with only expected CRLF warnings, WSL full debug
passed 27/27, and Windows full debug passed 30/30. Post-merge targeted tests
passed 5/5 on Windows and 4/4 on WSL Arch Linux, `git diff --check` produced
no output, WSL full debug passed 27/27, and Windows full debug passed 30/30.
Step 204, native menu and accelerator API skeleton, is the next implementation
slice.

Step 202, Wayland XDG configure lifecycle state, is merged on `master` at
`6c9b867 feat: add wayland configure lifecycle state`. RED failed as expected on
missing test-compositor helpers for stateful resize configures and last
configure-state inspection. GREEN adds internal `WaylandXdgConfigureState` and
`WaylandXdgToplevelState` records, parses activated/maximized/fullscreen
toplevel states, tracks pending size and last acked configure serial, and
dispatches existing public lifecycle events for activation/restoration. The
Wayland test compositor now sends state arrays with resize configures and
records configure serial/ack state for direct assertions. Feature-worktree
targeted tests passed 4/4 on WSL Arch Linux and 3/3 on Windows,
`git diff --check` exited 0 with only expected CRLF warnings, WSL full debug
passed 27/27, and Windows full debug passed 30/30. Post-merge targeted tests
passed 4/4 on WSL Arch Linux and 3/3 on Windows, `git diff --check` produced no
output, WSL full debug passed 27/27, and Windows full debug passed 30/30. Step
203, Win32 OLE drop target skeleton, is the next implementation slice.

Step 201, Wayland cursor theme image state, is merged on `master` at
`2b5dd4a feat: add wayland cursor theme state`. RED failed as expected on
missing deterministic Wayland cursor theme/image state markers and cursor-name
mapping coverage. GREEN adds internal `WaylandCursorThemeState`,
`WaylandCursorThemeLoadStatus`, `WaylandCursorImageState`, and
`cursor_name_for_shape(...)` records, maps common public cursor shapes to
Wayland cursor names, and records graceful unavailable cursor-image state before
the existing null `wl_pointer_set_cursor` call. Feature-worktree targeted tests
passed 3/3 on WSL Arch Linux and 2/2 on Windows, `git diff --check` exited 0
with only expected CRLF warnings, WSL full debug passed 27/27, and Windows full
debug passed 30/30. Post-merge targeted tests passed 3/3 on WSL Arch Linux and
2/2 on Windows, `git diff --check` produced no output, WSL full debug passed
27/27, and Windows full debug passed 30/30. Step 202, Wayland XDG configure
lifecycle state, followed as the next merged slice.

Step 200, Wayland drag action negotiation, is merged on `master` at
`d45061c feat: add wayland drag action negotiation`. RED failed as expected on
missing public `DragDropAction` metadata and drag-event `.action` fields.
GREEN adds `DragDropAction::{none, copy, move}` to public drag events, records
Wayland `wl_data_offer.source_actions` and `action`, accepts the preferred
payload MIME type, advertises destination copy/move actions with
`wl_data_offer_set_actions`, and calls `wl_data_offer_finish` after drop
payload extraction. The Wayland test compositor now records drag offer
`accept`, `set_actions`, and `finish` requests while emitting source and
selected action events for copy/move coverage. Feature-worktree targeted tests
passed 5/5 on WSL Arch Linux and 4/4 on Windows, `git diff --check` exited 0
with only expected CRLF warnings, WSL full debug passed 27/27, and Windows
full debug passed 30/30. Post-merge targeted tests passed 5/5 on WSL Arch
Linux and 4/4 on Windows, `git diff --check` produced no output, WSL full
debug passed 27/27, and Windows full debug passed 30/30. Step 201, Wayland
cursor theme image state, is the next implementation slice.

Step 199, Wayland clipboard ownership and send offers, is merged on `master`
at `96a5afa feat: add wayland clipboard ownership`. RED failed as expected on
the new Wayland clipboard write test waiting for a client-owned selection:
`write_text(...)` still wrote only to memory fallback, so the test compositor
never observed `wl_data_device.set_selection`. GREEN adds a real
`wl_data_source` ownership path in `WaylandClipboard::Connection`, offers
`text/plain;charset=utf-8` and `text/plain`, owns the selection through
`wl_data_device_set_selection`, and keeps a small dispatch loop alive so the
source can answer compositor `send` requests with the current UTF-8 payload.
The Wayland test compositor now records client-created data sources, offered
MIME types, selected sources, and deterministic payload reads through
`wl_data_source_send_send`. Feature-worktree targeted tests passed 3/3 on
Windows and WSL Arch Linux, `git diff --check` reported only expected CRLF
warnings, WSL full debug passed 27/27, and Windows full debug passed 30/30.
Post-merge targeted tests passed 3/3 on Windows and WSL Arch Linux,
`git diff --check` produced no output, WSL full debug passed 27/27, and the
Windows full debug gate passed 30/30 after isolating one transient
`clipboard_test/default` failure with a passing targeted rerun and a passing
full-suite rerun. Step 200, Wayland drag action negotiation, is the next
implementation slice.

Step 198, soft wrap layout records, is merged on `master` at
`02b534c feat: add text soft wrap records`. RED failed as expected on missing
`TextWrapLayout`, `TextWrapLine`, `wrap_text_measurement(...)`, and
renderer-visible text wrap lines. GREEN adds deterministic greedy glyph-level
wrap records derived from `TextMeasurement`, wrap-aware glyph paint metadata,
`TextPaint` / `TextDraw` line forwarding, and text/label layout sizing through
the current max-width constraint while preserving existing single-line text
behavior. Feature-worktree targeted tests passed 3/3 on Windows and WSL Arch
Linux, `git diff --check` reported only expected CRLF warnings, WSL full debug
passed 27/27, and Windows full debug passed 30/30. Post-merge targeted tests
passed 3/3 on Windows and WSL Arch Linux, `git diff --check` produced no
output, WSL full debug passed 27/27, and Windows full debug passed 30/30.
Step 199, Wayland clipboard ownership and send offers, is the next
implementation slice.

Step 191, grapheme-aware cursor movement skeleton, is merged on `master` at
`2108199 feat: add grapheme-aware text cursor movement`. RED failed as expected
on raw codepoint-boundary cursor movement through combining-mark clusters.
GREEN adds deterministic grapheme-boundary helpers for ASCII, combining marks,
variation selectors, regional indicator pairs, and emoji ZWJ skeleton cases,
then routes cursor movement, selection extension, backspace, and delete through
grapheme boundaries. Feature-worktree targeted tests passed 2/2 on Windows and
WSL Arch Linux, `git diff --check` reported only expected CRLF warnings, WSL
full debug passed 27/27, and Windows full debug passed 30/30 after isolating a
transient `clipboard_test/default` batch failure with a passing targeted rerun.
Post-merge targeted tests passed 2/2 on Windows and WSL Arch Linux,
`git diff --check` produced no output, WSL full debug passed 27/27, and Windows
full debug passed 30/30. Step 192, word movement and selection actions, is the
next implementation slice.

Step 192, word movement and selection actions, is merged on `master` at
`eaf6907 feat: add text word navigation actions`. RED failed as expected on
missing `TextEditAction::move_previous_word`, `move_next_word`,
`extend_previous_word`, and `extend_next_word`. GREEN adds those actions,
public word cursor helpers, and deterministic word-boundary helpers that skip
ASCII and Unicode spaces while walking existing grapheme boundaries. Tests cover
previous/next word movement, forward/backward word selection extension, tab
separators, and ideographic-space separators. Feature-worktree targeted tests
passed 2/2 on Windows and WSL Arch Linux, `git diff --check` reported only
expected CRLF warnings, WSL full debug passed 27/27, and Windows full debug
passed 30/30. Post-merge targeted tests passed 2/2 on Windows and WSL Arch
Linux, `git diff --check` produced no output, WSL full debug passed 27/27, and
Windows full debug passed 30/30. Step 193, text undo and redo stack, is the
next implementation slice.

Step 193, text undo and redo stack, is merged on `master` at
`5406e9a feat: add text undo redo stack`. RED failed as expected on missing
`TextModel::can_undo`, `can_redo`, `undo`, and `redo`, plus missing
`TextEditAction::undo` and `redo`. GREEN adds bounded edit-history snapshots
for text, cursor, and selection state, records insert/delete/selection replace
and composition commit edits, restores snapshots on undo/redo, clears redo on
new edits, and keeps composition state cleared when history restores. Tests
cover insert undo/redo, selection replacement restore, delete undo/redo,
composition commit undo/redo, redo invalidation after a new edit, and action
dispatch. Feature-worktree targeted tests passed 2/2 on Windows and WSL Arch
Linux, `git diff --check` reported only expected CRLF warnings, WSL full debug
passed 27/27, and Windows full debug passed 30/30. Post-merge targeted tests
passed 2/2 on Windows and WSL Arch Linux, `git diff --check` produced no
output, WSL full debug passed 27/27, and Windows full debug passed 30/30.
Step 194, IME delete-surrounding text action, was the next implementation
slice.

Step 194, IME delete-surrounding text action, is merged on `master` at
`98c2902 feat: route ime delete surrounding text`. RED failed as expected on
missing delete-surrounding public event, runtime routing, and text-model edit
API. GREEN adds `ImeDeleteSurroundingText`, maps it to
`EventKind::ime_delete_surrounding_text`, routes it to the focused text model,
adds `TextModel::delete_surrounding_text(...)` with UTF-8 boundary clamping and
undo history, and wires Wayland text-input v3 `delete_surrounding_text` through
the Linux backend and test compositor. Feature-worktree targeted tests passed
4/4 on Windows and 5/5 on WSL Arch Linux, `git diff --check` reported only
expected CRLF warnings, WSL full debug passed 27/27, and Windows full debug
passed 30/30. Post-merge targeted tests passed 4/4 on Windows and 5/5 on WSL
Arch Linux, `git diff --check` produced no output, WSL full debug passed
27/27, and Windows full debug passed 30/30 after isolating a transient
`clipboard_test/default` batch failure with a passing targeted rerun and a
passing full-suite rerun. Step 195, multiline text model and line navigation,
was the next implementation slice.

Step 195, multiline text model and line navigation, is merged on `master` at
`715f7bb feat: add multiline text navigation`. RED failed as expected on
missing line helper APIs and line-navigation edit actions. GREEN adds
deterministic LF-delimited line helpers (`line_count`, `line_index_at`,
`line_start_offset`, and `line_end_offset`), line start/end movement, previous
and next line movement using byte columns clamped to shorter lines, and
matching selection-extension actions. Feature-worktree targeted tests passed
2/2 on Windows and WSL Arch Linux, `git diff --check` reported only expected
CRLF warnings, WSL full debug passed 27/27, and Windows full debug passed
30/30. Post-merge targeted tests passed 2/2 on Windows and WSL Arch Linux,
`git diff --check` produced no output, WSL full debug passed 27/27, and Windows
full debug passed 30/30. Step 196, text measurement cache, is the next
implementation slice.

Step 196, text measurement cache, is merged on `master` at
`31bbe4d feat: add text measurement cache`. RED failed as expected on missing
`TextMeasurementCache`, `TextMeasurementResult`, and render-view cache
injection APIs. GREEN adds deterministic `measure_text(...)`,
`TextMeasurementKey`, `TextMeasurement`, `TextMeasurementResult`, and
`TextMeasurementCache` APIs keyed by text, font, font size, and normalized
scale, plus optional `PaintList` / `render_view` cache injection for text paint
measurement reuse. Feature-worktree targeted tests passed 3/3 on Windows and
WSL Arch Linux, `git diff --check` reported only expected CRLF warnings, WSL
full debug passed 27/27, and Windows full debug passed 30/30. Post-merge
targeted tests passed 3/3 on Windows and WSL Arch Linux, `git diff --check`
produced no output, WSL full debug passed 27/27, and Windows full debug passed
30/30 after isolating a transient `clipboard_test/default` batch failure with a
passing targeted rerun and passing full-suite rerun. Step 197, text pointer
selection geometry, is the next implementation slice.

Step 115, flex alignment and justification primitives, is merged on `master`
at `c443592 feat: add flex alignment justification`. RED failed as expected
on missing `AlignItems`/`JustifyContent` API, feature-worktree targeted tests
passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux full debug
passed 26/26. Post-merge targeted tests passed 3/3, Windows full debug passed
29/29, and WSL Arch Linux full debug passed 26/26. The Step 115 worktree and
feature branch have been removed.

Step 116, flex grow and shrink factors for child layout, is merged on `master`
at `2806a4a feat: add flex grow shrink layout`. RED failed as expected on
missing `flex_grow`/`flex_shrink` style, overlay, builder, and element APIs.
Feature-worktree targeted tests passed 3/3, Windows full debug passed 29/29,
and WSL Arch Linux full debug passed 26/26. Post-merge targeted tests passed
3/3, Windows full debug passed 29/29, and WSL Arch Linux full debug passed
26/26. The Step 116 worktree and feature branch have been removed.

Step 117, absolute positioning and inset style primitive, is merged on
`master` at `210c85d feat: add absolute positioning insets`. RED failed as
expected on missing `Position`, `Style::position`, `Style::inset`, overlay,
builder, and element APIs. Feature-worktree targeted tests passed 3/3, Windows
full debug passed 29/29, and WSL Arch Linux full debug passed 26/26.
Post-merge targeted tests passed 3/3, Windows full debug passed 29/29, and WSL
Arch Linux full debug passed 26/26. The Step 117 worktree and feature branch
have been removed.

The post-Step-128 back-40 plan is ready as Steps 129-168 and now has explicit
completion targets, non-goals, branch/test/verification matrix, per-step exit
artifacts, explicit keep-out-of-scope notes, Step 129 start packet, band
checkpoints after Steps 138, 148, 158, and 168, and an execution gate that
records Steps 115-123 as merged and post-merge verified. Step 129 remains
gated behind completion plus Windows/WSL verification of Steps 124-128 and the
post-Step-128 exit verification.

Step 118, layer/elevation style primitive mapped onto deterministic z order,
is merged on `master` at `9dfc2e7 feat: add layer elevation z order`. RED
failed as expected on missing `Style::layer`, `StyleOverlay::layer`,
`with_layer(...)`, builder `.layer(...)`, and `Element::layer()`/`z_order()`
APIs. Feature-worktree targeted tests passed 3/3, Windows full debug passed
29/29, and WSL Arch Linux full debug passed 26/26. Post-merge targeted tests
passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux full debug
passed 26/26.

Step 119, rounded-rect paint command that preserves border radius metadata, is
merged on `master` at `0893600 feat: add rounded rect paint command`. RED
failed as expected on missing `PaintCommandKind`, `RoundedRect`,
`PaintCommand::kind`, `PaintCommand::rounded_rect`, and
`PaintList::fill_rounded_rect(...)`. Feature-worktree targeted tests passed
3/3, Windows full debug passed 29/29, and WSL Arch Linux full debug passed
26/26. Post-merge targeted tests passed 3/3, Windows full debug passed 29/29,
and WSL Arch Linux full debug passed 26/26.

Step 120, Vulkan renderer honors clip rect metadata for solid rectangles, is
merged on `master` at `9aba0e6 feat: honor vulkan solid rect clips`. RED
failed as expected on missing `SolidRect::clip_rect`. Feature-worktree targeted
tests passed 4/4, Windows full debug passed 29/29, and WSL Arch Linux full
debug passed 26/26. Post-merge targeted tests passed 4/4, Windows full debug
passed 29/29, and WSL Arch Linux full debug passed 26/26.

Step 121, text paint command separates text drawing from placeholder
rectangles, is merged on `master` at `cf180f4 feat: add text paint command`.
RED failed as expected on missing text paint command APIs before the
implementation. GREEN adds `PaintCommandKind::text`, `TextPaint`,
`PaintList::fill_text(...)`, text clip metadata, `TextElement` text-command
painting, and a render path that skips text commands until a later Vulkan text
drawing step. Feature-worktree targeted tests passed 3/3, Windows full debug
passed 29/29, and WSL Arch Linux full debug passed 26/26. Post-merge targeted
tests passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux full
debug passed 26/26. Step 122, font descriptor and basic font-size style
primitives, was the next implementation slice.

Step 122, font descriptor and basic font-size style primitives, is merged on
`master` at `58561b1 feat: add font size style`. RED failed as expected on
missing
`FontDescriptor`, `Style::font`, `Style::font_size`, `StyleOverlay` font
overrides, builder `.font(...)`/`.font_size(...)`, text font metrics, and text
paint font metadata. GREEN adds public font metadata to `Style`/`StyleOverlay`,
deterministic font-size-derived `TextElement` metrics, text builder font
shortcuts, and `TextPaint` font/font-size metadata while preserving the default
16px text metrics. Feature-worktree targeted tests passed 3/3, Windows full
debug passed 29/29, and WSL Arch Linux full debug passed 26/26. Post-merge
targeted tests passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux
full debug passed 26/26. Step 123, text element caret and selection paint
metadata, is the next implementation slice.

Step 123, text element caret and selection paint metadata, is merged on
`master` at `b0b9e00 feat: add text caret selection paint`. RED failed as
expected on missing
`PaintCommandKind::text_selection`, `PaintCommandKind::text_caret`,
`TextSelectionPaint`, `TextCaretPaint`, `PaintCommand` payloads, and
`PaintList::fill_text_selection(...)`/`fill_text_caret(...)`. GREEN adds
metadata-only text selection and caret paint commands, deterministic
font-size-derived byte-offset geometry, caret emission for empty bound text
models, and renderer skipping for text-class commands. Feature-worktree
targeted tests passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux
full debug passed 26/26. Post-merge targeted tests passed 3/3, Windows full
debug passed 29/29, and WSL Arch Linux full debug passed 26/26. Step 124,
platform cursor application for Win32 and Wayland, is the next implementation
slice.

Step 124, platform cursor application for Win32 and Wayland, is merged on
`master` at `74ad787 feat: apply platform cursors`. RED failed as expected on
missing `PlatformWindow::set_cursor(...)`; GREEN moves `CursorShape` into the
core event surface, adds the platform cursor API, applies runtime hover cursor
changes to the platform window, maps Win32 cursor shapes to system cursors, and
adds a Wayland `wl_pointer.set_cursor` skeleton hook without cursor theme
loading. Feature-worktree targeted tests passed on Windows and WSL Arch Linux,
Windows full debug passed 29/29, and WSL Arch Linux full debug passed 26/26.
Post-merge targeted tests passed on Windows for built targets 2/2, Windows
full debug passed 29/29, and WSL Arch Linux full debug passed 26/26. Step 125,
Win32 system clipboard backend for text copy, cut, and paste, is the next
implementation slice.

Step 125, Win32 system clipboard backend for text copy, cut, and paste, is
merged on `master` at `389b9fb feat: add win32 system clipboard`. RED failed as
expected once the test target was force-rebuilt: `clipboard_test/default`
distinguished the existing memory clipboard from Win32 system clipboard
interop. GREEN keeps `MemoryClipboard` unchanged, makes Windows
`create_platform_clipboard()` return a `CF_UNICODETEXT`-backed UTF-8 clipboard
implementation, and updates xmake Windows `user32` links for clipboard
consumers. Feature-worktree targeted tests passed 1/1, Windows full debug
passed 29/29, and WSL Arch Linux full debug passed 26/26. Post-merge targeted
tests passed 1/1, Windows full debug passed 29/29, and WSL Arch Linux full
debug passed 26/26. Step 126, Wayland system clipboard backend skeleton for
text copy, cut, and paste, is the next implementation slice.

Step 126, Wayland system clipboard backend skeleton for text copy, cut, and
paste, is merged on `master` at `ab464d5 feat: add wayland clipboard
skeleton`. RED failed as expected on missing Linux `WaylandClipboard`,
`WaylandClipboardSupport`, and `WaylandClipboardOptions` APIs. GREEN adds a
Linux-only Wayland clipboard skeleton that reports `unsupported`, `no_seat`, or
`available` support states, keeps graceful memory fallback read/write behavior
for unsupported/no-seat runtime copy/cut/paste, and makes Linux
`create_platform_clipboard()` return the Wayland skeleton instead of
`MemoryClipboard`. Feature-worktree targeted tests passed 1/1 on Windows and
WSL Arch Linux, Windows full debug passed 29/29, and WSL Arch Linux full debug
passed 26/26. Post-merge targeted tests passed 1/1, Windows full debug passed
29/29, and WSL Arch Linux full debug passed 26/26.

Step 127, IME composition/candidate rectangle data from the focused text
element, is merged on `master` at `80aadae feat: add focused text ime rect`.
RED failed as expected on missing `ImeCandidateRect` and
`focused_text_ime_rect()` APIs. GREEN adds public `ImeCandidateRect` metadata
plus `WindowRuntime::focused_text_ime_rect()` and
`WindowRuntimeContext::focused_text_ime_rect()`, deriving the candidate rect
from the focused `TextElement` layout bounds, cursor byte offset,
font-size-derived glyph width, and caret height. Feature-worktree targeted
tests passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux full
debug passed 26/26. Post-merge targeted tests passed 3/3, Windows full debug
passed 29/29, and WSL Arch Linux full debug passed 26/26. Step 128,
GPUI-like demo rewrite using the public prelude and new authoring API, is the
next implementation slice.

Step 128, GPUI-like demo rewrite using the public prelude and new authoring
API, is merged on `master` at
`4026899 feat: rewrite demo with public prelude`. RED failed as expected after
the architecture test began requiring `cgpui/cgpui.hpp`, `run_app`,
`AppRunnerOptions`, `AppContext`, `View::render(ViewContext&)`, public element
factories, fluent builder shortcuts, and `ViewContext` model/text helpers
while rejecting manual `WindowRuntime`/`ElementTree` demo setup. GREEN rewrites
`examples/hello_window` around the public prelude and `run_app`, preserves the
first-frame, resize, close, and injected-text smoke env vars, and updates
README foundation wording. Feature-worktree targeted tests passed 2/2, Windows
hello-window smoke tests passed 3/3, Windows full debug passed 29/29, and WSL
Arch Linux full debug passed 26/26. Post-merge targeted tests passed 2/2,
Windows full debug passed 29/29, and WSL Arch Linux full debug passed 26/26.
Step 129, public `Context<T>` authoring alias over `ViewContext`, is the next
implementation slice.

## Risks

- Keep old APIs source-compatible where practical while adding result-returning semantics.
- Avoid building a full view tree before the routing primitives exist.
- Keep each step small enough for Windows and WSL verification.
- Keep macOS readiness as a boundary/audit track until the Windows/Linux core
  API surface is stable enough to justify Metal parity work.
- Do not overfit authoring helpers to the current demo; each helper should be
  general enough to survive later view/model lifecycle work.
- Keep renderer work command-driven and platform-neutral first, then teach the
  Vulkan backend to consume the new commands.

## Active Extension Note

- Phase B Step 275, `Context<T>::async_context() ->
  AsyncContextCapability`, is complete on `master` after post-merge Windows
  and WSL full-debug verification. Phase B Step 276,
  `Context<T>::test_context() -> TestContextCapability`, is complete on
  `master` after post-merge Windows and WSL full-debug verification. Phase B
  Step 277, subscription lifetime / deterministic unsubscribe, is complete on
  `master` after post-merge Windows and WSL full-debug verification. Phase B
  Step 278, entity-to-entity observation, is complete on `master` after
  post-merge Windows and WSL full-debug verification. Phase B Step 279,
  window/view observation, is complete on `master` after post-merge Windows
  and WSL full-debug verification. Phase B Step 280, observer diagnostics for
  window/view subscriptions, is complete on `master` after post-merge Windows
  and WSL full-debug verification. Phase B Step 281, view-handle runtime-token
  boundaries for cross-runtime read/upgrade/observation rejection, is complete
  on `master` after post-merge Windows and WSL full-debug verification. Phase
  B Step 282 observation/subscription closure audit is complete on `master`
  after post-merge Windows and WSL full-debug verification. Phase B Step 283,
  public typed-action authoring surface, is complete on `master` after
  post-merge Windows and WSL full-debug verification; the ownership boundary is
  the focused `include/cgpui/ui/action.hpp` leaf with prelude exposure. Phase B
  Step 284 typed action registration/dispatch overloads is complete on
  `master` at `3d50204` after post-merge Windows full-debug 70/70 and WSL
  Arch Linux full-debug 67/67 verification; it keeps
  `include/cgpui/ui/runtime_action_templates.hpp` as the typed overload leaf
  over the existing string registry. Phase B Step 285 action scope metadata is
  complete on `master` at `266c032` after post-merge Windows full-debug 71/71
  and WSL Arch Linux full-debug 68/68 verification; it keeps
  `ActionRegistration` metadata in the focused runtime action boundary. Phase
  B Step 286 typed action command metadata binding is complete on `master` at
  `43c6040` after post-merge Windows full-debug 72/72 and WSL Arch Linux
  full-debug 69/69 verification; it keeps command template helpers in
  `include/cgpui/ui/runtime_command_palette_templates.hpp`. Phase B Step 287
  action enablement metadata is complete on `master` at `5b1a873` after
  post-merge Windows full-debug 73/73 and WSL Arch Linux full-debug 70/70
  verification. Phase B Step 288 action bubbling through focused routes is
  next. Keep key dispatch, key grammar, and fuller test-context behavior in
  later slices.
