# Hit Testing Implementation Plan

## Steps

1. Add failing `element_test` coverage for retained layout bounds, leaf hit
   testing, vertical-stack child bounds, and child-before-self stack hit
   testing.
2. Verify RED: the test fails because `layout_bounds()` and `hit_test()` do not
   exist.
3. Add a geometry containment helper and retained bounds to `Element`.
4. Update `FixedSizeElement` and `VerticalStackElement` layouts to record
   bounds, then implement child-first stack hit testing.
5. Verify targeted, Windows full, and WSL full suites.
6. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/hit-testing && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
