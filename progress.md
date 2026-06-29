# CGPUI GPUI-Core Progress

## 2026-06-29

- Created the 20-step plan for the Windows/Linux GPUI-core milestone.
- Started Step 1: EventResult.
- Completed Step 1 implementation in `codex/core-event-result`.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 19/19.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 16/16.
- Started Step 2: Event dispatch observability.
- Added design and implementation plan docs for event dispatch records and the after-event callback.
- Added a RED runtime test for `EventDispatchRecord`, `EventKind`, `WindowRuntimeContext::last_event_dispatch`, and `WindowRuntime::set_after_event_callback`; the test failed because those APIs did not exist.
- Implemented Step 2 in `codex/event-dispatch-observability`: dispatch records include sequence, target root view id, event kind, and event result; runtime after-event callbacks see the current record after view handling.
- Fixed a test-only aggregation bug where a boolean accumulator started as `false`, then re-ran the target test.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 19/19.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 16/16.
- Started Step 3: Event routing shell.
- Added design and implementation plan docs for the root event routing shell.
- Added a RED runtime test for `EventRoute`, `EventRouter::route_to_root`, `WindowRuntimeContext::event_route`, and `EventDispatchRecord::route`; the test failed because those APIs did not exist.
- Implemented Step 3 in `codex/event-routing-shell`: view-dispatched events route through a root-only `EventRouter`, runtime contexts expose the active route, and dispatch records store the route used for the dispatch.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 19/19.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 16/16.
- Started Step 4: EntityId and EntityStore skeleton.
- Added design and implementation plan docs for typed entity ids and the minimal entity store lifecycle.
- Added a RED core test for `EntityId<T>`, `EntityStore<T>::insert`, mutable/const `get`, `remove`, and monotonic id allocation; the test failed because `cgpui/core/entity.hpp` did not exist.
- Implemented Step 4 in `codex/entity-store-skeleton`: `EntityId<T>` is a typed id wrapper, and `EntityStore<T>` supports header-only insert/emplace/get/remove with non-reused monotonic ids.
- Added the `entity_store_test` xmake target and included the new header in `core_header_cleanliness`.
- Verified targeted tests: `xmake test -P . entity_store_test/default core_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 20/20.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 17/17.

## 2026-06-30

- Started Step 5: Context entity access.
- Added design and implementation plan docs for context-forwarded runtime entity helpers.
- Added a RED runtime test that inserts, reads, mutates, removes, and emplaces an entity from `WindowRuntimeContext`; the test failed because `EntityId` and context entity helpers were not available through the UI runtime API.
- Implemented Step 5 in `codex/context-entity-access`: `WindowRuntime` owns type-erased per-type `EntityStore<T>` instances and `WindowRuntimeContext` forwards typed insert/emplace/read/mutate/remove helpers.
- Fixed the context helper constness so event handlers receiving `const WindowRuntimeContext&` can still mutate runtime-owned entity storage through the controlled capability methods.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default entity_store_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 20/20.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 17/17.
- Started Step 6: View identity allocation.
- Added design and implementation plan docs for runtime-owned `ViewId` allocation.
- Added a RED runtime test for `WindowRuntimeContext::allocate_view_id`, `WindowRuntimeContext::is_view_id_allocated`, monotonic allocation after the root view, and cross-event allocation stability; the test failed because those context APIs did not exist.
- Implemented Step 6 in `codex/view-identity-allocation`: `WindowRuntime` allocates monotonic view ids starting after the root view and recognizes root/allocated ids for the runtime lifetime.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 20/20.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 17/17.
- Started Step 7: ElementId and Element base API.
- Added design and implementation plan docs for the minimal element identity contract.
- Added a RED UI test for `ElementId`, polymorphic `Element`, default invalid ids, and stable assigned ids; the test failed because `cgpui/ui/element.hpp` did not exist.
- Implemented Step 7 in `codex/element-base-api`: `ElementId` is a typed numeric id wrapper and `Element` stores a runtime/tree-assigned id through `id()` and `assign_id()`.
- Added the `element_test` xmake target and included the new header in `ui_header_cleanliness`.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 21/21.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 18/18.
- Started Step 8: Element tree container.
- Added design and implementation plan docs for an owning `ElementTree` with root, children, and parent links.
- Added RED tests for root storage, child order, parent lookup, unknown-parent append rejection, and root replacement; the test failed because `ElementTree` did not exist.
- Implemented Step 8 in `codex/element-tree-container`: `ElementTree` owns elements, assigns monotonic `ElementId` values, stores child lists in append order, and exposes parent/get/children/root lookup helpers.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 21/21.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 18/18.
- Started Step 9: Element reconcile pass.
- Added design and implementation plan docs for a simple rebuild pass that preserves element ids.
- Added RED tests for `ElementTree::reconcile_root`, `ElementTree::reconcile_child`, root id preservation, child id preservation by parent-local index, append-on-new-index behavior, and unknown-parent rejection; the test failed because the reconcile APIs did not exist.
- Implemented Step 9 in `codex/element-reconcile-pass`: root reconcile replaces the root element while preserving id, and child reconcile replaces or appends children by parent/index while preserving existing child ids.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 21/21.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 18/18.
- Started Step 10: LayoutInput/LayoutOutput primitives.
- Added design and implementation plan docs for layout constraints, inputs,
  outputs, and clamping.
- Added a RED UI test for `LayoutInput`, `LayoutConstraints`,
  `constrain_size`, and `LayoutOutput`; the test failed because
  `cgpui/ui/layout.hpp` did not exist.
- Implemented Step 10 in `codex/layout-primitives`: `LayoutConstraints`
  stores min/max sizes, `LayoutInput` carries constraints, `LayoutOutput`
  stores origin/size, and `constrain_size` clamps preferred sizes per axis.
- Added the `layout_test` xmake target and included the new header in
  `ui_header_cleanliness`.
- Verified targeted tests: `xmake test -P . layout_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 11: Layout pass for fixed-size leaf elements.
- Added design and implementation plan docs for a leaf-only element layout
  pass.
- Added a RED UI test for `Element::layout`,
  `FixedSizeElement::layout`, preferred-size output, zero-origin output, and
  constraint clamping; the test failed because those APIs did not exist.
- Implemented Step 11 in `codex/fixed-size-layout`: `Element` now has a
  minimal virtual `layout(LayoutInput)` returning constrained zero size, and
  `FixedSizeElement` returns its constrained preferred size.
- Verified targeted tests: `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 14: Pointer routing to hit-tested element id.
- Added design and implementation plan docs for attaching hit-tested
  `ElementId` values to pointer event routes.
- Added a RED runtime test for `EventRoute::target_element_id`,
  `WindowRuntime::set_element_root`, pointer moved/button/scroll hit targets,
  and non-pointer events without element targets; the test failed because the
  routing APIs did not exist.
- Implemented Step 14 in `codex/pointer-hit-routing`: `EventRoute` now carries
  an optional `ElementId`, and `WindowRuntime` hit-tests an installed element
  root for pointer events while preserving root-view delivery.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default element_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 12: Layout pass for simple vertical stack.
- Added design and implementation plan docs for a simple owned-child vertical
  stack element.
- Added a RED UI test for `VerticalStackElement`, child ownership, empty-stack
  constrained size, max-child-width layout, summed-child-height layout, and
  stack-level constraint clamping; the test failed because
  `VerticalStackElement` did not exist.
- Implemented Step 12 in `codex/vertical-stack-layout`: `VerticalStackElement`
  owns child elements, ignores null appends, exposes read-only children, and
  computes layout as max child width plus summed child heights.
- Verified targeted tests: `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 13: Hit testing over laid-out element bounds.
- Added design and implementation plan docs for retained element bounds and
  element-local hit testing.
- Added RED UI tests for `layout_bounds()`, leaf `hit_test(Point)`, vertical
  stack child bounds, and child-before-self stack hit testing; the test failed
  because the hit-testing APIs did not exist.
- Implemented Step 13 in `codex/hit-testing`: elements retain their latest
  layout bounds, `FixedSizeElement` and `VerticalStackElement` record bounds
  during layout, and stack hit testing checks children before falling back to
  the stack id.
- Verified targeted tests: `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
