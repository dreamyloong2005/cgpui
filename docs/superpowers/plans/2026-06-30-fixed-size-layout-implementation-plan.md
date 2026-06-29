# Fixed Size Layout Implementation Plan

## Steps

1. Add failing `element_test` coverage for `Element::layout` and
   `FixedSizeElement::layout`.
2. Verify RED: the test fails because the layout APIs do not exist.
3. Include `layout.hpp` from `element.hpp` and add the minimal layout virtual.
4. Add `FixedSizeElement` as a small leaf element type.
5. Verify targeted, Windows full, and WSL full suites.
6. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/fixed-size-layout && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
