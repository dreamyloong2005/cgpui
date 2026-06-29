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
