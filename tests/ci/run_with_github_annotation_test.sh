#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
helper="$repo_root/scripts/ci/run-with-github-annotation.sh"
test_root=$(mktemp -d "${TMPDIR:-/tmp}/cgpui-ci-annotation.XXXXXX")
trap 'rm -rf -- "$test_root"' EXIT

success_output=$("$helper" "$test_root/success.log" -- \
  bash -c 'printf "%s\n" success')
[[ "$success_output" == "success" ]]
[[ "$(cat "$test_root/success.log")" == "success" ]]

set +e
failure_output=$(GITHUB_ACTIONS=true \
  "$helper" "$test_root/failure.log" -- \
  bash -c 'printf "first\nsecond\n"; exit 7' 2>&1)
failure_status=$?
set -e
[[ "$failure_status" -eq 7 ]]
[[ "$(cat "$test_root/failure.log")" == $'first\nsecond' ]]
expected_annotation='::error title=CI command failed (exit 7)::first%0Asecond'
[[ "$failure_output" == *"$expected_annotation"* ]]

set +e
long_output=$(GITHUB_ACTIONS=true \
  "$helper" "$test_root/long.log" -- bash -c \
  'for _ in $(seq 1 100); do printf "%0100d\n" 0; done; printf "final-error\n"; exit 9' \
  2>&1)
long_status=$?
set -e
annotation_line=$(printf '%s\n' "$long_output" | sed -n '/^::error/p')
[[ "$long_status" -eq 9 ]]
[[ "$annotation_line" == ::error* ]]
[[ "$annotation_line" == *"final-error"* ]]
[[ ${#annotation_line} -lt 4096 ]] || exit 1

set +e
"$helper" "$test_root/usage.log" >/dev/null 2>&1
usage_status=$?
set -e
[[ "$usage_status" -eq 2 ]]
