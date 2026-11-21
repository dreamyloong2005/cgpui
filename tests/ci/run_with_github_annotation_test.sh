#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
helper="$repo_root/scripts/ci/run-with-github-annotation.sh"
test_root=$(mktemp -d "${TMPDIR:-/tmp}/cgpui-ci-annotation.XXXXXX")
trap 'rm -rf -- "$test_root"' EXIT

success_output=$("$helper" "$test_root/success.log" -- \
  bash -c 'printf "%s\n" success')
# Bash 3.2 (macOS) needs explicit failure for standalone [[ ]] assertions.
[[ "$success_output" == "success" ]] || exit 1
[[ "$(cat "$test_root/success.log")" == "success" ]] || exit 1

set +e
failure_output=$(GITHUB_ACTIONS=true \
  "$helper" "$test_root/failure.log" -- \
  bash -c 'printf "first\nsecond\n"; exit 7' 2>&1)
failure_status=$?
set -e
[[ "$failure_status" -eq 7 ]] || exit 1
[[ "$(cat "$test_root/failure.log")" == $'first\nsecond' ]] || exit 1
expected_annotation='::error title=CI command failed (exit 7)::first%0Asecond'
[[ "$failure_output" == *"$expected_annotation"* ]] || exit 1

set +e
long_output=$(GITHUB_ACTIONS=true \
  "$helper" "$test_root/long.log" -- bash -c \
  'for _ in $(seq 1 100); do printf "%0100d\n" 0; done; printf "final-error\n"; exit 9' \
  2>&1)
long_status=$?
set -e
annotation_line=$(printf '%s\n' "$long_output" | sed -n '/^::error/p')
[[ "$long_status" -eq 9 ]] || exit 1
[[ "$annotation_line" == ::error* ]] || exit 1
[[ "$annotation_line" == *"final-error"* ]] || exit 1
[[ ${#annotation_line} -lt 4096 ]] || exit 1

set +e
early_output=$(GITHUB_ACTIONS=true "$helper" "$test_root/early.log" -- bash -c '
  printf "engine.cpp:4:2: fatal error: load at 50%%\r\n  missing header\n" >&2
  for ((i=0; i<120; ++i)); do printf "%0100d\n" "$i"; done
  echo "ninja: error: build stopped"; exit 2' 2>&1)
early_status=$?
set -e
annotation_line=$(printf '%s\n' "$early_output" | sed -n '/^::error/p')
[[ "$early_status" -eq 2 ]] || exit 1
[[ "$annotation_line" == *'engine.cpp:4:2: fatal error: load at 50%25%0D%0A  missing header'* ]] || exit 1
[[ "$annotation_line" == *'ninja: error: build stopped' ]] || exit 1
[[ ${#annotation_line} -lt 4096 ]] || exit 1

local_status=0
test_output=$(GITHUB_ACTIONS=true "$helper" "$test_root/test.log" -- bash -c '
  echo "cocoa_window ... FAIL (exit 9, 0.41s)"
  for ((i=0; i<120; ++i)); do printf "%0100d\n" "$i"; done
  echo "error: workspace test: 1 member failed"; exit 1' 2>&1) || local_status=$?
[[ "$local_status" -eq 1 ]] || exit 1
annotation_line=$(printf '%s\n' "$test_output" | sed -n '/^::error/p')
[[ "$annotation_line" == *'First diagnostic: cocoa_window ... FAIL (exit 9, 0.41s)'* ]] || exit 1

local_status=0
local_output=$(GITHUB_ACTIONS=false "$helper" "$test_root/local.log" -- \
  bash -c 'echo "error: local failure" >&2; exit 4' 2>&1) || local_status=$?
[[ "$local_status" -eq 4 && "$local_output" == 'error: local failure' ]] || exit 1

set +e
"$helper" "$test_root/usage.log" >/dev/null 2>&1
usage_status=$?
set -e
[[ "$usage_status" -eq 2 ]] || exit 1
