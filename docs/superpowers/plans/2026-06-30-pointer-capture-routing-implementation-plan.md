# Pointer Capture Routing Implementation Plan

## Steps

1. Add failing runtime coverage for element-level pointer capture overriding
   hit-tested pointer routes and releasing only by the matching owner id.
2. Verify RED: the test fails because element capture APIs and input state do
   not exist.
3. Add element capture owner state to `WindowRuntime` and `ViewInputState`.
4. Add `capture_pointer(ElementId)` and `release_pointer(ElementId)` overloads.
5. Route pointer events to the captured element before falling back to fresh
   hit testing.
6. Verify targeted, Windows full, and WSL full suites.
7. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default element_test/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/pointer-capture-routing && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
