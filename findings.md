# CGPUI GPUI-Core Findings

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
