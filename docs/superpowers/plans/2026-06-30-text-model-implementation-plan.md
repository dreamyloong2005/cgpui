# Text Model Implementation Plan

## Steps

1. Add failing text-model coverage for empty defaults, UTF-8 insertion, cursor
   movement, backspace, and forward delete.
2. Verify RED: the test fails because `cgpui/ui/text.hpp` does not exist.
3. Add a header-only `TextModel` that stores text and cursor byte offset.
4. Implement insertion and UTF-8 codepoint boundary navigation.
5. Implement backspace and forward delete.
6. Add the header to UI header cleanliness and add a `text_model_test` target.
7. Verify targeted, Windows full, and WSL full suites.
8. Update planning files, commit, merge to `master`, and clean up the worktree.

## Verification

- `xmake test -P . text_model_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/text-model && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
