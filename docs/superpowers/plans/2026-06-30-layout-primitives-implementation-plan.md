# Layout Primitives Implementation Plan

## Steps

1. Add a failing `layout_test` for default constraints, min/max storage, size
   clamping, and layout output origin/size storage.
2. Register `layout_test` in `xmake.lua` and include the future layout header in
   `ui_header_cleanliness`.
3. Verify the test fails because `cgpui/ui/layout.hpp` is missing.
4. Add `include/cgpui/ui/layout.hpp` with header-only layout primitives.
5. Verify targeted, Windows full, and WSL full suites.
6. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . layout_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/layout-primitives && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
