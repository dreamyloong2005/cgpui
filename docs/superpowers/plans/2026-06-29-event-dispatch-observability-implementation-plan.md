# Event Dispatch Observability Implementation Plan

## Steps

1. Add a failing runtime test that uses the desired dispatch-record API.
2. Add `EventKind`, `EventDispatchRecord`, and callback declarations to the UI
   header.
3. Record event dispatches inside `WindowRuntime::handle_event`.
4. Surface the latest dispatch record through `WindowRuntimeContext`.
5. Verify targeted, Windows full, and WSL full test suites.
6. Update root planning files, commit, merge, and clean up the feature worktree.

## Verification

- `xmake test -P . window_runtime_test/default`
- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/event-dispatch-observability && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
