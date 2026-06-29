# View Identity Allocation Implementation Plan

## Steps

1. Add a failing runtime test that allocates view ids from
   `WindowRuntimeContext` during event handling.
2. Add context and runtime APIs for view id allocation and membership checks.
3. Store the next runtime view id value in `WindowRuntime`.
4. Verify targeted, Windows full, and WSL full suites.
5. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/view-identity-allocation && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
