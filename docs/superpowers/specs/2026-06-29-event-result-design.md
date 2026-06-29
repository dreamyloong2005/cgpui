# Event Result Design

## Goal

Make event handling report whether an event was consumed or cancelled, giving future routing layers a concrete result to propagate.

## Scope

- Add `EventResult` to the UI layer.
- Change `View::handle_event` to return `EventResult`.
- Make the default view handler return an unhandled result.
- Have `WindowRuntime` store the last view event result for tests and later callbacks.
- Expose that stored result through `WindowRuntimeContext`.

## Result Semantics

- `consumed == false` and `cancelled == false`: the event was observed but not handled.
- `consumed == true` and `cancelled == false`: routing should stop because a view handled the event.
- `cancelled == true`: routing should stop and later default behavior should not run.
- `cancelled == true` implies the result is also treated as consumed by routing.

## Non-Goals

- No bubbling or tunneling yet.
- No child element routing yet.
- No default platform behavior changes yet.
- No action/key binding dispatch yet.

## Testing

- Extend `window_runtime_test` with a view that consumes and cancels selected events.
- Verify default/unhandled events leave `last_event_result` unconsumed.
- Verify a consumed event updates `last_event_result.consumed`.
- Verify a cancelled event updates both `cancelled` and consumed-equivalent state.
