# Element Tree Container Implementation Plan

## Steps

1. Add failing tests for root storage, child order, parent links, lookup, and
   unknown-parent append behavior.
2. Implement `ElementTree` in `include/cgpui/ui/element.hpp`.
3. Verify targeted, Windows full, and WSL full suites.
4. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . element_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/element-tree-container && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
