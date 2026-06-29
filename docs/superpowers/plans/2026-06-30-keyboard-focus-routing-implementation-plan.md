# Keyboard Focus Routing Implementation Plan

## Steps

1. Add failing runtime coverage for element-level keyboard focus routing,
   wrong-owner release, owner-matched release, and text-input routing.
2. Verify RED: the test fails because element keyboard focus APIs and input
   state do not exist.
3. Add element keyboard focus owner state to `WindowRuntime` and
   `ViewInputState`.
4. Add `request_keyboard_focus(ElementId)` and
   `release_keyboard_focus(ElementId)` overloads.
5. Route keyboard and text-input events to the focused element before falling
   back to root-view-only routes.
6. Verify targeted, Windows full, and WSL full suites.
7. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . window_runtime_test/default ui_header_cleanliness/default element_test/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/keyboard-focus-routing && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
