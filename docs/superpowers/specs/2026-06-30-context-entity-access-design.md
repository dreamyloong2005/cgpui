# Context Entity Access Design

## Goal

Expose safe entity read/mutation helpers through `WindowRuntimeContext` so view
code can create and access typed state without directly owning storage.

This keeps the API close to GPUI's context-driven model while staying inside the
current single-root-view runtime.

## API Shape

- `WindowRuntimeContext::insert_entity<T>(T)` inserts a typed entity.
- `WindowRuntimeContext::emplace_entity<T>(...)` constructs and inserts a typed
  entity.
- `WindowRuntimeContext::read_entity<T>(EntityId<T>)` returns a const pointer or
  `nullptr`.
- `WindowRuntimeContext::mutate_entity<T>(EntityId<T>)` returns a mutable
  pointer or `nullptr`.
- `WindowRuntimeContext::remove_entity<T>(EntityId<T>)` removes an entity and
  reports whether it existed.

## Behavior

- Entity storage is owned by `WindowRuntime`.
- Stores are separated by entity type.
- Unknown, removed, or cross-store ids return `nullptr` or `false`.
- Context helpers remain usable during event callbacks and frame callbacks.
