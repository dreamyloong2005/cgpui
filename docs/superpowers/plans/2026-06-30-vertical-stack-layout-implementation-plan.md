# Vertical Stack Layout Implementation Plan

## Steps

1. Add failing `element_test` coverage for empty stack layout, child ownership,
   max-width/summed-height layout, and stack-level constraint clamping.
2. Verify RED: the test fails because `VerticalStackElement` does not exist.
3. Add `VerticalStackElement` to `include/cgpui/ui/element.hpp`.
4. Verify targeted, Windows full, and WSL full suites.
5. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/vertical-stack-layout && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
