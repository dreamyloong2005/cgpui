# Text Input Routing Implementation Plan

## Steps

1. Add failing runtime coverage for binding a `TextModel` to an element,
   focusing that element, routing text input into the model, and ignoring text
   input after focus release.
2. Verify RED: the test fails because `bind_text_model` does not exist.
3. Include the text model API in `ui.hpp`.
4. Store non-owning text model pointers in `WindowRuntime` by `ElementId`.
5. On `TextInput`, insert text into the model bound to the current focused
   element owner.
6. Verify targeted, Windows full, and WSL full suites.
7. Update planning files, commit, merge to `master`, clean up the worktree, and
   run the final completion audit.

## Verification

- `xmake test -P . window_runtime_test/default text_model_test/default ui_header_cleanliness/default`
- `xmake f -c -m debug -P .; xmake test -P .`
- `wsl.exe -d archlinux -- bash -lc 'cd /mnt/d/Dev/Projects/cgpui/.worktrees/text-input-routing && XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .'`
