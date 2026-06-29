# Key Binding Table Implementation Plan

## Steps

1. Add failing runtime coverage for binding Ctrl+S to a named action and
   ignoring non-matching key events.
2. Verify RED: the test fails because `KeyBinding` and `bind_key` do not exist.
3. Add the `KeyBinding` API type.
4. Store key bindings in `WindowRuntime`.
5. Match `KeyboardKey` events against bindings and dispatch the matched action.
6. Verify targeted, Windows full, and WSL full suites.
7. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/key-binding-table && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
