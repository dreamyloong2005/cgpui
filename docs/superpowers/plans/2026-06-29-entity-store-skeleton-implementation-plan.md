# Entity Store Skeleton Implementation Plan

## Steps

1. Add a failing core test for typed entity ids and insert/get/remove lifecycle.
2. Add a public `cgpui/core/entity.hpp` header.
3. Add the `entity_store_test` xmake target.
4. Implement the minimal header-only `EntityId<T>` and `EntityStore<T>`.
5. Verify targeted, Windows full, and WSL full suites.
6. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . entity_store_test/default core_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/entity-store-skeleton && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
