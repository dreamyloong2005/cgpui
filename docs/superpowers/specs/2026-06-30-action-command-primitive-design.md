# Action Command Primitive Design

## Goal

Introduce a minimal named action primitive so input routing can later dispatch
semantic commands instead of hard-coding key handling.

## Decisions

- Keep actions in the UI runtime layer because handlers need
  `WindowRuntimeContext` capabilities.
- Use `std::string` names for the first primitive, such as `app.save`.
- `ActionHandler` returns `EventResult`, reusing existing consumed/cancelled
  semantics.
- `ActionDispatchResult` records the name, whether a handler was found, and the
  handler result.
- `WindowRuntime` stores the last action dispatch for observability in tests
  and later command/key-binding plumbing.

## Non-Goals

- No key binding lookup yet.
- No command palette or global action registry.
- No element-scoped action bubbling.
