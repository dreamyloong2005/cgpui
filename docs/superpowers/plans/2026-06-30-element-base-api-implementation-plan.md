# Element Base API Implementation Plan

## Steps

1. Add a failing UI test for `ElementId` and the base `Element` id contract.
2. Add the `include/cgpui/ui/element.hpp` public header.
3. Add an `element_test` xmake target.
4. Include the new header in UI header cleanliness coverage.
5. Verify targeted, Windows full, and WSL full suites.
6. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . element_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/element-base-api && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
