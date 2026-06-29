# Pointer Hit Routing Implementation Plan

## Steps

1. Add failing runtime coverage for pointer moved, pointer button, and pointer
   scroll events carrying the hit `ElementId` in `EventRoute`.
2. Verify RED: the test fails because `EventRoute::target_element_id` and
   `WindowRuntime::set_element_root` do not exist.
3. Add `target_element_id` to `EventRoute` and expose a non-owning
   `set_element_root` on `WindowRuntime`.
4. Route pointer events by hit-testing the installed element root at the event
   position; leave non-pointer events without an element target.
5. Verify targeted, Windows full, and WSL full suites.
6. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default element_test/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/pointer-hit-routing && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
