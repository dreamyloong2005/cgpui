# Event Routing Shell Design

## Goal

Move the "which target receives this event" decision into a tiny routing layer
instead of hardcoding the root view in `WindowRuntime::handle_event`.

This is intentionally a shell. The runtime still has only one root view, but the
public API now names the route and target so later element/view-tree routing can
replace the internals without changing dispatch records again.

## API Shape

- `EventRoute` stores the target `ViewId` and `EventKind`.
- `EventRouter::route_to_root(event, root_view_id)` returns the route for a
  root-only runtime.
- `WindowRuntimeContext` exposes the active `event_route` while a view handles
  an event.
- `EventDispatchRecord` stores the route that produced the dispatch.

## Behavior

- Routed view events still go to the root view.
- Dispatch sequence behavior is unchanged.
- Runtime-only lifecycle events still do not produce a view dispatch.
- Existing pointer capture and keyboard focus ownership behavior is unchanged.
