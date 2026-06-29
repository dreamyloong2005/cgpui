# Event Dispatch Observability Design

## Goal

Expose one stable observation point for every platform event that the window
runtime dispatches to the root view. The record is a small routing prerequisite:
later steps can route to element/view ids while tests can still assert which
target handled an event and what result it returned.

## API Shape

- Add `EventKind` as a compact semantic tag for `PlatformEvent` variants.
- Add `EventDispatchRecord` with `sequence`, `view_id`, `event_kind`, and
  `result`.
- Add `WindowRuntimeContext::last_event_dispatch` so view code can inspect the
  previous dispatched event.
- Add `WindowRuntime::set_after_event_callback(...)` for runtime users and tests.

## Behavior

- Only events passed to `View::handle_event` create a dispatch record.
- Lifecycle events handled by the runtime itself do not create records:
  close requests, resizes, and redraw requests.
- The dispatch sequence starts at `1` for each `run` call.
- The after-event callback runs after `View::handle_event` returns and after
  `last_event_result` / `last_event_dispatch` have been updated.
