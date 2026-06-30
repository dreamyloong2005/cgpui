# GPUI Core Steps 89-128 Execution Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete Steps 89-128 as small, verified slices toward a GPUI-like Windows/Linux core API.

**Architecture:** Keep the public API additive and source-compatible while moving from low-level runtime primitives to GPUI-like element authoring, render hooks, model/view lifecycle, event propagation, layout depth, and platform-backed text/cursor integration. Windows and Linux continue to use Vulkan, Linux continues to target Wayland, and macOS/Cocoa + Metal remains a readiness boundary only.

**Tech Stack:** C++23, xmake, Win32/Vulkan, Wayland/Vulkan, header-cleanliness tests, targeted UI/runtime tests, Windows full debug verification, and WSL Arch full debug verification.

---

## Current State

- Steps 89-105 are implemented, merged to `master`, and post-merge verified on
  Windows and WSL Arch Linux.
- `master` is at `58c5b8a docs: refresh back forty planning after step 105`.
- The main worktree has no tracked/staged changes; the only known untracked
  local item is `.vscode/`.
- Step 106 is implemented in `.worktrees/window-root-view-lifecycle` and
  feature-worktree verified on Windows and WSL Arch Linux.
- The next implementation slice after merging Step 106 is Step 107:
  view registry skeleton for multiple view ids.

## File Map

- `include/cgpui/ui/style.hpp`: style units, colors, edge helpers, style state, flex/positioning style fields, font style fields.
- `include/cgpui/ui/element.hpp`: public element factories, builder shortcuts, handler wrappers, stateful style wrappers, child views, scroll elements, layout behavior, text paint metadata.
- `include/cgpui/ui/ui.hpp`: `ViewContext`, `View::render`, runtime render installation, model helpers, subscriptions, route metadata, propagation, focus traversal, scroll routing, app/window context APIs.
- `include/cgpui/core/entity.hpp`: typed entity/model ids and weak entity primitives.
- `include/cgpui/core/geometry.hpp`: geometry/color primitives if a helper belongs below the UI style layer.
- `include/cgpui/platform/clipboard.hpp` and `src/platform/clipboard.cpp`: cross-platform clipboard interface and memory backend compatibility.
- `src/platform/win32/win32_application.cpp`: Win32 cursor and system clipboard integration.
- `src/platform/linux/wayland_application.cpp`: Wayland cursor/clipboard hooks and skeleton protocol surface.
- `include/cgpui/renderer/renderer.hpp`: paint command vocabulary for rounded rects, text, caret, selection, and clip metadata.
- `src/renderer/vulkan/vulkan_renderer.cpp`: Vulkan consumption of clip metadata and new paint command shapes.
- `examples/hello_window/main.cpp`: final GPUI-like demo rewrite.
- `tests/ui/*.cpp`: primary RED/GREEN coverage for authoring, runtime, layout, render, model, text, and app APIs.
- `tests/platform/*.cpp`: Win32/Wayland cursor, clipboard, IME, and input integration coverage.
- `tests/renderer/*.cpp`: Vulkan render-command coverage.
- `tests/header_cleanliness/*.cpp`: public header/prelude compile coverage for each API slice.

## Per-Step Workflow

Use one branch and one isolated worktree per roadmap step:

```powershell
git worktree add .worktrees/<slug> -b codex/<slug> master
```

For each step:

- [ ] Add the RED test first in the smallest relevant test target.
- [ ] Run the targeted command and confirm it fails for the expected missing API or behavior.
- [ ] Implement the minimal GREEN change.
- [ ] Run the targeted command again.
- [ ] Run Windows full debug verification.
- [ ] Run WSL Arch full debug verification for shared UI/runtime/renderer/platform/build surfaces.
- [ ] Update `task_plan.md`, `progress.md`, and `findings.md`.
- [ ] Commit the feature branch, fast-forward merge to `master`, and repeat targeted/full verification on `master`.
- [ ] Remove the feature worktree and delete the branch after post-merge WSL verification passes.

Standard commands:

```powershell
xmake test -P . <target>/default <header_target>/default
xmake f -c -m debug -P .
xmake test -P .
wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/<slug> && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'
git checkout master
git merge --ff-only codex/<slug>
wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'
git worktree remove .worktrees/<slug>
git branch -d codex/<slug>
```

## Milestone Bands

### Band A: Authoring and Render Entry, Steps 89-98

Purpose: make the public API feel GPUI-like before deeper lifecycle work depends on it.

- [x] Step 89: `AnyElement` and `into_element(...)`.
- [x] Step 90: free factories `div()`, `h_flex()`, `v_flex()`, `v_stack()`, `text(...)`.
- [x] Step 91: child overloads for builders and owned element values.
- [x] Step 92: style unit/color helpers.
- [x] Step 93: builder style shortcuts.
- [x] Step 94: pointer handler shortcuts.
- [x] Step 95: focus/hover/disabled style-state primitives.
- [x] Step 96: `View::render(ViewContext&)` skeleton.
- [x] Step 97: runtime render pass installs rendered element trees.
- [x] Step 98: render invalidation helper and after-render observability.

Acceptance at the end of Band A:

- A root view can author a tree through the public prelude using free element factories, child conversion, style helpers, handler shortcuts, and `View::render`.
- Runtime redraw can install the rendered root element tree without breaking the existing `View::paint` contract.
- Existing direct `WindowRuntime` APIs remain source-compatible.

### Band B: Model, App, Window, and View Lifecycle, Steps 99-108

Purpose: introduce the GPUI-style model and app shell needed for real applications.

- [x] Step 99: public `Model<T>`/`Entity<T>` aliases over typed entity ids.
- [x] Step 100: `ViewContext` model helpers for create, read, update, and remove.
- [x] Step 101: weak entity/view handle primitives with soft-fail upgrade.
- [x] Step 102: observe/subscribe callback helper for model changes.
- [x] Step 103: model update notification invalidates subscribed views.
- [x] Step 104: public `AppContext` wrapper over app setup.
- [x] Step 105: `WindowOptions` builder and `AppContext::open_window(...)` skeleton.
- [x] Step 106: runtime root view lifecycle storage for app-opened windows.
- [ ] Step 107: view registry skeleton for multiple view ids.
- [ ] Step 108: child-view element placeholder that embeds another view's rendered output.

Acceptance at the end of Band B:

- User code can create and update typed models through `ViewContext`.
- Views can observe model changes and receive invalidation without manual runtime plumbing.
- `run_app` can expose an `AppContext` and a first `open_window(...)` path without committing to a complete multi-window platform event-loop rewrite.
- A view registry exists before child-view rendering and event ancestry become more detailed.

### Band C: Event Propagation, Focus, Scroll, and Layout Depth, Steps 109-118

Purpose: turn the element tree from a hit-test target into a richer interactive layout tree.

- [ ] Step 109: event route carries element and view ancestry metadata.
- [ ] Step 110: target handling and ancestor bubbling before view fallback.
- [ ] Step 111: Tab and Shift+Tab focus traversal over enabled focusable elements.
- [ ] Step 112: scroll element binding helper backed by `ScrollState`.
- [ ] Step 113: wheel and trackpad scroll routing into bound scroll state.
- [ ] Step 114: hidden overflow participates in hit testing.
- [ ] Step 115: flex alignment and justification primitives.
- [ ] Step 116: flex grow and shrink factors.
- [ ] Step 117: absolute positioning and inset style primitive.
- [ ] Step 118: layer/elevation style primitive mapped onto deterministic z order.

Acceptance at the end of Band C:

- Runtime event records can explain both target and ancestor paths.
- Element handlers can consume or bubble events before the root view fallback.
- Focus and scroll behavior use element tree queries rather than ad hoc ids.
- Layout supports the minimum flex/absolute/layer controls needed for a usable app-like UI.

### Band D: Render Commands, Text, Platform Backends, and Demo, Steps 119-128

Purpose: replace placeholder rendering and memory-only platform behaviors with command and backend surfaces that Windows/Linux can actually consume.

- [ ] Step 119: rounded-rect paint command preserves border radius metadata.
- [ ] Step 120: Vulkan honors clip rect metadata for solid rectangles.
- [ ] Step 121: text paint command separates text drawing from placeholder rectangles.
- [ ] Step 122: font descriptor and basic font-size style primitives.
- [ ] Step 123: text element emits caret and selection paint metadata.
- [ ] Step 124: platform cursor application for Win32 and Wayland.
- [ ] Step 125: Win32 system clipboard backend for text copy, cut, and paste.
- [ ] Step 126: Wayland system clipboard backend skeleton for text copy, cut, and paste.
- [ ] Step 127: IME composition/candidate rectangle data from the focused text element.
- [ ] Step 128: GPUI-like demo rewrite using the public prelude and new authoring API.

Acceptance at the end of Band D:

- Renderer command streams distinguish solid rects, rounded rects, text, caret, and selection metadata.
- Vulkan clips solid rectangles using command clip metadata.
- Windows and Wayland cursor APIs are connected to runtime cursor state.
- Clipboard operations are no longer limited to memory-only tests on Windows; Wayland has a protocol-shaped skeleton.
- The demo exercises the public prelude instead of low-level runtime setup.

## Remaining Execution Queue From Step 96

This is the practical remaining sequence after Step 98. Steps 89-98 are kept
as completed foundation; the active remaining queue is Steps 99-128.

### Checkpoint 1: Finish Authoring Entry, Steps 93-98

Goal: make user code author UI through GPUI-like factories and `View::render`
without touching model lifecycle yet.

- [x] Step 93: land fluent builder style shortcuts on `ElementBuilder`.
- [x] Step 94: add pointer-down/up/move/click authoring shortcuts.
- [x] Step 95: add base/hover/focus/disabled style overlay primitives and
  deterministic style resolution.
- [x] Step 96: add optional `View::render(ViewContext&)` while preserving the
  existing `paint(...)` contract.
- [x] Step 97: make redraw install the root view's rendered element tree.
- [x] Step 98: expose render invalidation and after-render observability.

Exit check: a root view can return a public-prelude-authored element tree from
`render(...)`, and redraw can install that tree for layout, hit testing, and
paint while old views still compile.

### Checkpoint 2: Add GPUI-Like Data And App Shell, Steps 99-108

Goal: introduce typed models, app setup, window opening, and view identity
storage before nested view/event work.

- [x] Step 99: add public `Model<T>` and `Entity<T>` aliases over typed entity
  ids.
- [x] Step 100: add `ViewContext` helpers for model create/read/update/remove.
- [x] Step 101: add weak entity/view handles with soft-fail upgrade.
- [x] Step 102: add model observe/subscribe callbacks.
- [x] Step 103: make model updates notify observers and invalidate subscribed
  views.
- [x] Step 104: expose an `AppContext` setup wrapper.
- [x] Step 105: add `WindowOptions` and `AppContext::open_window(...)`
  skeleton.
- [x] Step 106: store app-opened root view lifetimes explicitly.
- [ ] Step 107: add the first view registry for multiple `ViewId`s.
- [ ] Step 108: add a child-view element placeholder that references a
  registered view.

Exit check: app setup can create a window with a root view, user code can
mutate typed models through `ViewContext`, and model updates can trigger view
invalidation.

### Checkpoint 3: Deepen Interaction And Layout, Steps 109-118

Goal: make routed events, focus, scroll, and layout behavior tree-aware instead
of single-target-only.

- [ ] Step 109: attach element and view ancestry metadata to event routes.
- [ ] Step 110: implement target handling, ancestor bubbling, and view fallback
  propagation phases.
- [ ] Step 111: add Tab and Shift+Tab focus traversal over enabled focusable
  elements.
- [ ] Step 112: add a scroll-element binding helper backed by `ScrollState`.
- [ ] Step 113: route wheel/trackpad scroll events into bound scroll state.
- [ ] Step 114: make hidden overflow constrain hit testing.
- [ ] Step 115: add flex alignment and justification primitives.
- [ ] Step 116: add flex grow and shrink factors.
- [ ] Step 117: add absolute positioning and inset style.
- [ ] Step 118: add layer/elevation mapped to deterministic z order.

Exit check: event records can explain target and ancestor paths, bubbling can
stop on consumed results, focus/scroll use tree queries, and layout has the
minimum controls for app-like UI.

### Checkpoint 4: Replace Placeholders With Backend-Ready Rendering, Steps 119-128

Goal: harden the renderer/platform surface for Windows and Wayland/Vulkan while
leaving macOS/Metal for a later parity track.

- [ ] Step 119: add rounded-rect paint commands with border-radius metadata.
- [ ] Step 120: make Vulkan honor clip rect metadata for solid rectangles.
- [ ] Step 121: add text paint commands instead of placeholder rectangles.
- [ ] Step 122: add font descriptors and basic font-size style.
- [ ] Step 123: emit caret and selection paint metadata from text elements.
- [ ] Step 124: apply runtime cursor state through Win32 and Wayland platform
  hooks.
- [ ] Step 125: add a Win32 system clipboard backend for UTF-8 text.
- [ ] Step 126: add a Wayland clipboard backend skeleton with graceful
  unsupported behavior.
- [ ] Step 127: expose IME candidate/composition rectangle data from the
  focused text element.
- [ ] Step 128: rewrite the demo around public prelude, `run_app`,
  `AppContext`, `View::render`, factories, builder shortcuts, and text/model
  interactions.

Exit check: paint streams distinguish rectangles, rounded rectangles, text,
caret, and selection; Vulkan consumes clipping metadata; Win32 and Wayland have
cursor/clipboard integration points; the demo uses the public GPUI-like API.

## Current Recommended Next Step

After Step 106 is committed, merged, and post-merge verified, start Step 107
in an isolated worktree:

```powershell
git worktree add .worktrees/view-registry-skeleton -b codex/view-registry-skeleton master
xmake test -P . window_runtime_test/default ui_header_cleanliness/default
```

Add RED tests showing views can be registered, found, and removed by `ViewId`
while preserving the existing root view id, then follow the standard per-step
verification/merge workflow above.

## Step Details

### Step 92: Style Unit and Color Helpers

Status: complete on `master` at `6e7d34c`.

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`
- Modify: `tests/header_cleanliness/prelude_header_cleanliness.cpp`

- [ ] Add RED tests for `px(float) -> float`, `rgb(255, 128, 0)`, `rgba(255, 128, 0, 0.5F)`, `edges(float)`, `edges(float, float)`, and `edges(float, float, float, float)`.
- [ ] Implement inline helpers in `style.hpp`, mapping colors into normalized `Color` channels and edges into the existing `EdgeSizes` layout.
- [ ] Targeted test command: `xmake test -P . style_test/default ui_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 93: Element Builder Fluent Style Shortcuts

Status: complete on `master` after the Step 93 merge.

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`
- Modify: `tests/header_cleanliness/prelude_header_cleanliness.cpp`

- [ ] Add RED tests for builder calls that avoid manual `Style{}` construction: `size(Size)`, `size(float, float)`, `padding(EdgeSizes)`, `margin(EdgeSizes)`, `background(Color)`, `foreground(Color)`, `border_width(EdgeSizes)`, `border_color(Color)`, `border_radius(BorderRadii)`, and `gap(float)`.
- [ ] Implement the shortcuts as style mutation on the builder's retained `Style`.
- [ ] Targeted test command: `xmake test -P . element_test/default style_test/default ui_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 94: Pointer Handler Shortcuts

Status: complete on `master` after the Step 94 merge.

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`

- [ ] Add RED tests for `on_pointer_down`, `on_pointer_up`, `on_pointer_move`, and `on_click`.
- [ ] Preserve the existing `on_click` behavior while extending pointer handler wrappers to route concrete pointer event data.
- [ ] Targeted test command: `xmake test -P . element_test/default window_runtime_test/default ui_header_cleanliness/default`.

### Step 95: Style-State Overlay Primitives

Status: complete on `master` after the Step 95 merge.

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`

- [ ] Add RED tests for a `StyleState` or equivalent value that stores base, hover, focus, and disabled overlays.
- [ ] Add builder authoring methods for hover/focus/disabled style overlays without requiring runtime application in this slice.
- [ ] Add a deterministic `resolved_style(...)` helper that applies disabled over focus over hover over base for testable merge order.
- [ ] Targeted test command: `xmake test -P . style_test/default element_test/default ui_header_cleanliness/default`.

### Step 96: GPUI-Like `View::render(ViewContext&)` Skeleton

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/header_cleanliness/ui_header_cleanliness.cpp`
- Modify: `tests/header_cleanliness/prelude_header_cleanliness.cpp`

- [ ] Add RED tests proving existing views still compile with `paint(...)` and can optionally override `render(ViewContext&)`.
- [ ] Add a default `View::render(ViewContext&)` implementation that returns an empty `AnyElement`.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 97: Runtime Render Pass Installs Rendered Element Tree

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for a `render(...)` override returning an element tree that is installed before layout/hit testing on redraw.
- [ ] Implement the redraw path so a non-null render result replaces the owned root element tree.
- [ ] Keep `paint(...)` invocation compatible for existing views.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default element_test/default ui_header_cleanliness/default`.

### Step 98: Render Invalidation Helper and After-Render Observability

Status: complete on `master` after the Step 98 merge.

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED tests for `ViewContext::request_render()` and an after-render callback or record that reports render sequence and installed root element id.
- [x] Implement render invalidation as a narrow layer over existing layout/paint invalidation and redraw scheduling.
- [x] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 99: Public `Model<T>` and `Entity<T>` Aliases

Status: complete on `master` after the Step 99 merge.

**Files:**
- Modify: `include/cgpui/core/entity.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `tests/core/entity_store_test.cpp`
- Modify: `tests/header_cleanliness/core_header_cleanliness.cpp`
- Modify: `tests/header_cleanliness/prelude_header_cleanliness.cpp`

- [x] Add RED compile tests for `Model<T>` and `Entity<T>` as public authoring aliases over typed ids.
- [x] Implement aliases without changing `EntityStore<T>` semantics.
- [x] Targeted test command: `xmake test -P . entity_store_test/default core_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 100: ViewContext Model Helpers

Status: complete on `master` after the Step 100 merge.

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED tests for `new_model`, `read_model`, `update_model`, and `remove_model` wrappers.
- [x] Implement helpers over the existing runtime entity store and notify changed models after successful updates.
- [x] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 101: Weak Entity and View Handles

Status: complete on `master` after the Step 101 merge.

**Files:**
- Modify: `include/cgpui/core/entity.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `tests/core/entity_store_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED tests for `WeakEntity<T>` and `WeakView` storing typed ids and upgrading through a context/runtime.
- [x] Implement soft-fail upgrade that returns null/empty when the model or view id is missing.
- [x] Targeted test command: `xmake test -P . entity_store_test/default window_runtime_test/default ui_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 102: Observe/Subscribe Callback Helper

Status: complete on `master` after the Step 102 merge.

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED tests for registering a model observer callback through `ViewContext`.
- [x] Store observer callbacks by model type and id alongside existing subscription metadata.
- [x] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 103: Model Update Invalidates Subscribed Views

Status: complete on `master` after the Step 103 merge.

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED tests proving `update_model(...)` and `remove_model(...)` trigger render/layout/paint invalidation for subscribed views.
- [x] Wire update notification to render invalidation and existing redraw scheduling.
- [x] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 104: AppContext Wrapper

Status: complete on `master` after the Step 104 merge.

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/header_cleanliness/prelude_header_cleanliness.cpp`

- [x] Add RED tests for setup callbacks receiving an `AppContext`.
- [x] Keep the existing `AppSetupCallback(WindowRuntime&)` path source-compatible or provide an additive overload.
- [x] Targeted test command: `xmake test -P . app_runner_test/default ui_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 105: WindowOptions and `open_window(...)`

Status: complete on `master` after the Step 105 merge.

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`

- [x] Add RED tests for a fluent `WindowOptions` builder that maps to `WindowDescriptor`.
- [x] Add `AppContext::open_window(...)` as a skeleton that records/creates the initial runtime-backed window path without full multi-window platform ownership yet.
- [x] Targeted test command: `xmake test -P . app_runner_test/default ui_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 106: Runtime Root View Lifecycle Storage

Status: implemented and feature-worktree verified; commit, merge, and
post-merge verification still need to finish before Step 107 starts.

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/app_runner_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [x] Add RED tests showing app-opened windows keep root view ownership/lifetime stable through `run_app`.
- [x] Add runtime/app storage that owns or references root views through an explicit lifecycle container.
- [x] Targeted test command: `xmake test -P . app_runner_test/default window_runtime_test/default ui_header_cleanliness/default prelude_header_cleanliness/default`.

### Step 107: View Registry Skeleton

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for registering, finding, and removing views by `ViewId`.
- [ ] Implement a minimal registry that coexists with the existing root view id.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`.

### Step 108: Child-View Element Placeholder

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for an element that references a child `ViewId` and exposes placeholder layout/hit-test/render metadata.
- [ ] Add a builder/helper for child-view placeholders after the view registry exists.
- [ ] Targeted test command: `xmake test -P . element_test/default window_runtime_test/default ui_header_cleanliness/default`.

### Step 109: Event Route Ancestry Metadata

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for route element ancestry and view ancestry arrays.
- [ ] Populate ancestry from `ElementTree::parent(...)` and the view registry/child-view placeholder metadata.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default element_test/default ui_header_cleanliness/default`.

### Step 110: Event Propagation Phases

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for target handling, ancestor bubbling, consumed stop behavior, and view fallback.
- [ ] Implement propagation using route ancestry without changing disabled-element gates.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default element_test/default ui_header_cleanliness/default`.

### Step 111: Focus Traversal

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for Tab and Shift+Tab over enabled focusable elements in traversal order.
- [ ] Implement traversal over `ElementTree::enabled_preorder_ids()` plus `Element::focusable()`.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default element_test/default ui_header_cleanliness/default`.

### Step 112: Scroll Element Binding Helper

**Files:**
- Modify: `include/cgpui/ui/scroll.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/scroll_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for binding `ScrollState` to an element and querying the binding.
- [ ] Implement a scroll wrapper/helper that keeps layout behavior compatible with the wrapped child.
- [ ] Targeted test command: `xmake test -P . scroll_test/default element_test/default ui_header_cleanliness/default`.

### Step 113: Wheel and Trackpad Scroll Routing

**Files:**
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/ui/scroll_test.cpp`

- [ ] Add RED tests for `PointerScrolled` events mutating the bound `ScrollState` at the routed element.
- [ ] Implement soft-fail routing when no scroll state is bound.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default scroll_test/default ui_header_cleanliness/default`.

### Step 114: Hidden Overflow Hit Testing

**Files:**
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests proving hidden overflow prevents hits outside the clipping bounds while visible overflow preserves current behavior.
- [ ] Apply existing `Style::overflow` and `clip_rect` data to hit testing.
- [ ] Targeted test command: `xmake test -P . element_test/default window_runtime_test/default ui_header_cleanliness/default`.

### Step 115: Flex Alignment and Justification

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for `AlignItems`, `JustifyContent`, and builder/style setters.
- [ ] Implement row/column child origins for start, center, end, and space-between where current constraints make the free space observable.
- [ ] Targeted test command: `xmake test -P . style_test/default element_test/default ui_header_cleanliness/default`.

### Step 116: Flex Grow and Shrink

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for child flex grow/shrink factors affecting main-axis allocation.
- [ ] Store flex item data on elements or builder-created wrappers without changing default zero-grow behavior.
- [ ] Targeted test command: `xmake test -P . element_test/default style_test/default ui_header_cleanliness/default`.

### Step 117: Absolute Positioning and Insets

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for position mode and inset edges.
- [ ] Keep default layout unchanged and only apply absolute positioning when explicitly authored.
- [ ] Targeted test command: `xmake test -P . style_test/default element_test/default ui_header_cleanliness/default`.

### Step 118: Layer/Elevation Style Primitive

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for `Style::layer` or `elevation` mapping onto deterministic paint order.
- [ ] Preserve `z_index` compatibility by defining a clear precedence between explicit z-index and layer/elevation.
- [ ] Targeted test command: `xmake test -P . style_test/default element_test/default ui_header_cleanliness/default`.

### Step 119: Rounded-Rect Paint Command

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [ ] Add RED tests for paint commands carrying border radius metadata without losing existing solid rect behavior.
- [ ] Extend command data before teaching Vulkan new drawing behavior.
- [ ] Targeted test command: `xmake test -P . element_test/default render_view_test/default ui_header_cleanliness/default`.

### Step 120: Vulkan Clip Rect Metadata

**Files:**
- Modify: `src/renderer/vulkan/vulkan_renderer.cpp`
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `tests/renderer/vulkan_solid_rect_test.cpp`

- [ ] Add RED renderer tests or command-level tests for clipped solid rectangles.
- [ ] Map `PaintCommand::clip_rect` to Vulkan scissor state for solid rectangles.
- [ ] Targeted test command: `xmake test -P . vulkan_solid_rect_test/default`.

### Step 121: Text Paint Command

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/render_view_test.cpp`

- [ ] Add RED tests for text commands containing text content, bounds, color, and model-derived metadata.
- [ ] Change `TextElement::paint` from placeholder rectangles to a text command while preserving older tests through updated expectations.
- [ ] Targeted test command: `xmake test -P . element_test/default render_view_test/default ui_header_cleanliness/default`.

### Step 122: Font Descriptor and Font Size Style

**Files:**
- Modify: `include/cgpui/ui/style.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/style_test.cpp`
- Modify: `tests/ui/element_test.cpp`

- [ ] Add RED tests for `FontDescriptor`, `Style::font_size`, and builder setters.
- [ ] Use font size in `TextElement` skeleton metrics before real shaping exists.
- [ ] Targeted test command: `xmake test -P . style_test/default element_test/default ui_header_cleanliness/default`.

### Step 123: Text Caret and Selection Paint Metadata

**Files:**
- Modify: `include/cgpui/renderer/renderer.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `tests/ui/element_test.cpp`
- Modify: `tests/ui/text_model_test.cpp`

- [ ] Add RED tests for caret and selection paint metadata emitted from `TextElement` when the model has cursor/selection state.
- [ ] Keep geometry simple and derived from the current fixed/font-size text metrics.
- [ ] Targeted test command: `xmake test -P . element_test/default text_model_test/default ui_header_cleanliness/default`.

### Step 124: Platform Cursor Application

**Files:**
- Modify: `include/cgpui/platform/platform.hpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/platform/win32_input_event_test.cpp`
- Modify: `tests/platform/wayland_pointer_button_test.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`

- [ ] Add RED tests for runtime cursor state reaching a platform cursor application hook.
- [ ] Implement Win32 cursor mapping and a Wayland hook/skeleton that can be exercised by tests without requiring full compositor cursor themes.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default win32_input_event_test/default wayland_pointer_button_test/default`.

### Step 125: Win32 System Clipboard Backend

**Files:**
- Modify: `include/cgpui/platform/clipboard.hpp`
- Modify: `src/platform/clipboard.cpp`
- Modify: `src/platform/win32/win32_application.cpp`
- Modify: `tests/platform/clipboard_test.cpp`

- [ ] Add RED tests for a Win32 clipboard implementation that reads/writes UTF-8 text through the system clipboard.
- [ ] Keep `MemoryClipboard` unchanged for deterministic runtime tests.
- [ ] Targeted test command: `xmake test -P . clipboard_test/default`.

### Step 126: Wayland System Clipboard Backend Skeleton

**Files:**
- Modify: `include/cgpui/platform/clipboard.hpp`
- Modify: `src/platform/clipboard.cpp`
- Modify: `src/platform/linux/wayland_application.cpp`
- Modify: `tests/platform/clipboard_test.cpp`

- [ ] Add RED tests for constructing a Wayland clipboard backend skeleton and exercising graceful unsupported/no-seat behavior.
- [ ] Preserve runtime copy/cut/paste behavior through the shared `Clipboard` interface.
- [ ] Targeted test command: `xmake test -P . clipboard_test/default`.

### Step 127: IME Candidate Rectangle Data

**Files:**
- Modify: `include/cgpui/ui/text.hpp`
- Modify: `include/cgpui/ui/element.hpp`
- Modify: `include/cgpui/ui/ui.hpp`
- Modify: `src/ui/ui.cpp`
- Modify: `tests/ui/window_runtime_test.cpp`
- Modify: `tests/platform/win32_text_input_test.cpp`

- [ ] Add RED tests for focused text elements reporting a candidate/composition rectangle based on caret geometry.
- [ ] Expose the data through runtime/context so Win32 and Wayland IME code can consume it later.
- [ ] Targeted test command: `xmake test -P . window_runtime_test/default win32_text_input_test/default ui_header_cleanliness/default`.

### Step 128: GPUI-Like Demo Rewrite

**Files:**
- Modify: `examples/hello_window/main.cpp`
- Modify: `README.md`
- Modify: `tests/architecture/hello_window_lifetime_test.cpp`

- [ ] Add RED architecture/demo coverage that looks for public prelude usage, `run_app`/`AppContext`, `View::render`, free factories, builder shortcuts, and model/text interactions.
- [ ] Rewrite the demo to use the public prelude and new authoring API while keeping Windows/Linux Vulkan startup intact.
- [ ] Targeted test command: `xmake test -P . hello_window_lifetime_test/default prelude_header_cleanliness/default`.

## Risk Controls

- Keep every step additive unless the step explicitly replaces placeholder behavior.
- Keep public headers platform-neutral; platform-specific code stays in `src/platform/win32`, `src/platform/linux`, or renderer backends.
- Prefer command metadata before backend behavior for renderer changes.
- Keep memory and system clipboard paths separate until platform backends are proven.
- Do not start full macOS parity inside this plan.
- If a step grows beyond one branch, split it into a preparatory data/API slice and a behavior slice, then update `task_plan.md` before continuing.
