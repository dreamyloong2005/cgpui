# Action Command Primitive Implementation Plan

## Steps

1. Add failing runtime coverage for registering a named action, dispatching it,
   missing-action results, and last-dispatch observability.
2. Verify RED: the test fails because action types and runtime APIs do not
   exist.
3. Add `ActionDispatchResult` and `ActionHandler` to the UI API.
4. Add runtime storage for action handlers and the last action dispatch.
5. Implement `register_action`, `dispatch_action`, and
   `last_action_dispatch`.
6. Verify targeted, Windows full, and WSL full suites.
7. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/action-command-primitive && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
