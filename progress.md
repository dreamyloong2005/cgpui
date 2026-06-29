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
- Started Step 4: EntityId and EntityStore skeleton.
- Added design and implementation plan docs for typed entity ids and the minimal entity store lifecycle.
- Added a RED core test for `EntityId<T>`, `EntityStore<T>::insert`, mutable/const `get`, `remove`, and monotonic id allocation; the test failed because `cgpui/core/entity.hpp` did not exist.
- Implemented Step 4 in `codex/entity-store-skeleton`: `EntityId<T>` is a typed id wrapper, and `EntityStore<T>` supports header-only insert/emplace/get/remove with non-reused monotonic ids.
- Added the `entity_store_test` xmake target and included the new header in `core_header_cleanliness`.
- Verified targeted tests: `xmake test -P . entity_store_test/default core_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 20/20.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 17/17.
- Started Step 3: Event routing shell.
- Added design and implementation plan docs for the root event routing shell.
- Added a RED runtime test for `EventRoute`, `EventRouter::route_to_root`, `WindowRuntimeContext::event_route`, and `EventDispatchRecord::route`; the test failed because those APIs did not exist.
- Implemented Step 3 in `codex/event-routing-shell`: view-dispatched events route through a root-only `EventRouter`, runtime contexts expose the active route, and dispatch records store the route used for the dispatch.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 19/19.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 16/16.
