# Event Routing Shell Implementation Plan

## Steps

1. Add a failing runtime/header test for `EventRouter`, `EventRoute`, and
   `WindowRuntimeContext::event_route`.
2. Add the new routing structs/classes to the UI public header.
3. Route runtime view events through the root router before calling
   `View::handle_event`.
4. Preserve existing event dispatch records and input/focus behavior.
5. Verify targeted, Windows full, and WSL full suites.
6. Update root planning files, commit, merge to `master`, and clean up the
   feature worktree.

## Verification

- `xmake test -P . window_runtime_test/default`
- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/event-routing-shell && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
