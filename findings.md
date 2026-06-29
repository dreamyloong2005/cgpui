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
