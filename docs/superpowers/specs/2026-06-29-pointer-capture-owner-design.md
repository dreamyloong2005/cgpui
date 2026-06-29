# Pointer Capture Owner Design

## Goal

Make pointer capture use the same owner model as keyboard focus so later event routing can reason about input ownership by `ViewId` instead of by window-level booleans alone.

## Scope

- Add `ViewInputState::pointer_capture_owner` as `std::optional<ViewId>`.
- Keep `ViewInputState::pointer_captured`, but derive it from `pointer_capture_owner == context.view_id`.
- Add owner-aware `WindowRuntime::capture_pointer(ViewId)` and `WindowRuntime::release_pointer(ViewId)` overloads.
- Keep the existing no-argument pointer capture APIs as root-view conveniences.
- Treat `release_pointer(ViewId)` from a non-owner as a no-op.
- Reset the pointer capture owner at the start of each `WindowRuntime::run`.

## Non-Goals

- No platform pointer capture calls yet.
- No view tree or child view allocation.
- No event retargeting to the capture owner.
- No drag gesture abstraction.
- No pointer capture owner for multiple simultaneous pointer ids.

## Testing

- Extend `window_runtime_test` to verify a view can capture pointer ownership with `context.view_id`.
- Verify later pointer events report `pointer_captured == true` and a matching `pointer_capture_owner`.
- Verify a non-owner release does not clear capture.
- Verify the owning view can release capture and later contexts see it cleared.
