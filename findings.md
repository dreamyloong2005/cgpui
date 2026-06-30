# CGPUI GPUI-Core Findings

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
