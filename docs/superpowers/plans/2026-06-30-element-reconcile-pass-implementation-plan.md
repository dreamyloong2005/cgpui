# Element Reconcile Pass Implementation Plan

## Steps

1. Add failing tests for root id preservation, child id preservation by parent
   index, append-on-new-index behavior, and unknown-parent rejection.
2. Add `ElementTree::reconcile_root`.
3. Add `ElementTree::reconcile_child`.
4. Verify targeted, Windows full, and WSL full suites.
5. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . element_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/element-reconcile-pass && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
