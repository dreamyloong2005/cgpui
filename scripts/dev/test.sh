#!/usr/bin/env bash
# Build and run the cgpui tests on Linux: scripts/dev/test.sh [--asan] [mcpp test args]
# --asan builds in build/asan (scripts/dev/asan_workspace.py: every member,
# -fsanitize=address,undefined, _GLIBCXX_ASSERTIONS); docs/getting-started.md
# section 7.
set -euo pipefail
asan=0
if [ "${1:-}" = --asan ]; then asan=1; shift; fi
source "$(dirname "${BASH_SOURCE[0]}")/mcpp-env.sh"
if [ "$asan" = 1 ]; then
  python3 "$REPO_ROOT/scripts/dev/asan_workspace.py" linux "$REPO_ROOT/build/asan"
  cd "$REPO_ROOT/build/asan"
  # Members are listed relative to build/asan: -p tests/ui => ../../tests/ui.
  args=()
  prev=
  for arg in "$@"; do
    if [ "$prev" = -p ] && [ "${arg#..}" = "$arg" ]; then arg="../../$arg"; fi
    args+=("$arg")
    prev=$arg
  done
  # Leak checking is on (LSan, the default on Linux); a report fails the test.
  export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_stack_use_after_return=1:strict_string_checks=1}"
  export UBSAN_OPTIONS="${UBSAN_OPTIONS:-print_stacktrace=1}"
  exec "$MCPP" test "${profile_args[@]}" ${args[@]+"${args[@]}"}
fi
cd "$REPO_ROOT"
export MCPP_COMPILE_COMMANDS="${MCPP_COMPILE_COMMANDS:-compile_commands.json}"
exec "$MCPP" test "${profile_args[@]}" "$@"
