# View Identity Allocation Design

## Goal

Add the first runtime-owned view identity allocator so future element/view tree
work can create stable `ViewId` values beyond the root view.

## API Shape

- `WindowRuntimeContext::allocate_view_id()` allocates a new runtime-owned
  `ViewId`.
- `WindowRuntimeContext::is_view_id_allocated(ViewId)` reports whether an id is
  the root view id or has been allocated by the runtime.
- `WindowRuntime` exposes matching public methods for tests and future routing
  internals.

## Behavior

- `ViewId{0}` stays invalid.
- The root view keeps `ViewId{1}`.
- Runtime-allocated ids are monotonic, non-zero, and greater than the root id.
- Allocated ids remain recognized for the lifetime of the runtime.
- No deallocation or reconcile behavior is introduced in this step; later steps
  will bind these ids to element/view tree nodes.
