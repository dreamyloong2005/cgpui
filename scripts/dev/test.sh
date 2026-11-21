#!/usr/bin/env bash
# Build and run the cgpui tests on Linux or macOS:
#   scripts/dev/test.sh [--asan [--per-member-cleanup]] [mcpp test args]
# --asan builds in build/asan (scripts/dev/asan_workspace.py: every member,
# -fsanitize=address,undefined, libstdc++ or libc++ assertions); --per-member-cleanup
# runs one test member at a time and deletes its output after it.
# docs/getting-started.md section 7.
set -euo pipefail
asan=0
per_member=0
if [ "${1:-}" = --asan ]; then asan=1; shift; fi
if [ "$asan" = 1 ] && [ "${1:-}" = --per-member-cleanup ]; then per_member=1; shift; fi
source "$(dirname "${BASH_SOURCE[0]}")/mcpp-env.sh"
# The X11 tests run on a private Xvfb display, never on the session's DISPLAY:
# x11_clipboard_test owns the CLIPBOARD selection, which a desktop (WSLg too)
# copies into the host clipboard. CGPUI_TEST_DISPLAY=session keeps DISPLAY.
platform=linux
if [ "$(uname -s)" = Darwin ]; then
  platform=macos
  # The minimum macOS (docs/macos-sdk.md); mcpp's own default is 14.0.
  export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-26.0}"
else
  source "$REPO_ROOT/scripts/dev/test-display.sh"
  cgpui_start_test_display
fi
if [ "$asan" = 1 ]; then
  python3 "$REPO_ROOT/scripts/dev/asan_workspace.py" "$platform" "$REPO_ROOT/build/asan"
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
  # No strict_string_checks: libxkbcommon-x11 1.13.2 passes XCB reply strings
  # that are not NUL-terminated to strndup-like reads, which that option
  # reports inside the system library (every X11 test).
  export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_stack_use_after_return=1}"
  export UBSAN_OPTIONS="${UBSAN_OPTIONS:-print_stacktrace=1}"
  # Leaks inside Mesa's Vulkan drivers (scripts/dev/lsan-suppressions.txt).
  export LSAN_OPTIONS="${LSAN_OPTIONS:-suppressions=$REPO_ROOT/scripts/dev/lsan-suppressions.txt}"
  # Keep the Vulkan drivers loaded until exit: unloaded, their allocations
  # are reported as leaks with unnamed ([unknown module]) frames.
  export VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING=1
  if [ "$per_member" = 0 ]; then
    status=0
    "$MCPP" test "${profile_args[@]}" ${args[@]+"${args[@]}"} || status=$?
    exit "$status"
  fi
  # --per-member-cleanup: one member with tests at a time, then its test
  # programs and objects are deleted (the libraries' objects are kept). A
  # full ASan build does not fit on a small disk. CGPUI_ASAN_STOP_USED_GB
  # stops before the next member once the file system holding build/asan
  # uses more than that many GB.
  status=0
  while IFS='|' read -r member bin_dir obj_dir; do
    used=$(df -Pk . | awk 'NR == 2 { print int($3 / 1048576) }')
    if [ -n "${CGPUI_ASAN_STOP_USED_GB:-}" ] && [ "$used" -gt "$CGPUI_ASAN_STOP_USED_GB" ]; then
      echo "asan: ${used} GB used > ${CGPUI_ASAN_STOP_USED_GB} GB; stopping before $member" >&2
      exit 3
    fi
    echo "asan member $member (${used} GB used)"
    "$MCPP" test "${profile_args[@]}" -p "../../$member" ${args[@]+"${args[@]}"} || status=1
    for dir in target/*/*; do rm -rf "$dir/bin/$bin_dir" "$dir/obj/$obj_dir"; done
  done < <(python3 "$REPO_ROOT/scripts/dev/asan_workspace.py" members "$REPO_ROOT")
  exit "$status"
fi
cd "$REPO_ROOT"
export MCPP_COMPILE_COMMANDS="${MCPP_COMPILE_COMMANDS:-compile_commands.json}"
# Not exec: the EXIT trap of test-display.sh stops Xvfb after the tests.
status=0
"$MCPP" test "${profile_args[@]}" "$@" || status=$?
exit "$status"
