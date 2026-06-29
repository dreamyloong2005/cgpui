# Entity Store Skeleton Design

## Goal

Add the first typed entity identity and storage primitive for the GPUI-core API.
This is deliberately small: it proves id allocation and lifecycle semantics
without adding contexts, views, or element ownership yet.

## API Shape

- `EntityId<T>` is a typed id wrapper around a non-zero integer value.
- `EntityStore<T>` inserts values, returns stable typed ids, supports mutable
  and const lookup, and removes entities by id.
- Removed or unknown ids return `nullptr`.
- Ids are monotonic and are not reused in this skeleton.

## Behavior

- The first inserted entity receives id value `1`.
- `insert` returns a typed id.
- `get` returns a pointer to the stored entity or `nullptr`.
- `remove` returns `true` only when an entity existed and was removed.
- Different `EntityId<T>` instantiations are intentionally incompatible types.
