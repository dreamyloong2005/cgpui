# Context Entity Access Implementation Plan

## Steps

1. Add a failing runtime test that uses entity helpers from
   `WindowRuntimeContext`.
2. Add type-erased per-type entity stores to `WindowRuntime`.
3. Add context helper templates that forward to runtime-owned stores.
4. Verify targeted, Windows full, and WSL full suites.
5. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default entity_store_test/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/context-entity-access && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
