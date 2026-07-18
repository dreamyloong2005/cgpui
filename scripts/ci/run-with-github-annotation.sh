#!/usr/bin/env bash
set -uo pipefail

if [[ $# -lt 3 || "$2" != "--" ]]; then
  echo "usage: run-with-github-annotation.sh <log-path> -- <command> [args...]" >&2
  exit 2
fi

log_path=$1
shift 2
mkdir -p "$(dirname "$log_path")"

set +e
"$@" 2>&1 | tee "$log_path"
status=${PIPESTATUS[0]}
set -e

if [[ $status -ne 0 && "${GITHUB_ACTIONS:-}" == "true" ]]; then
  message=$(tail -n 80 "$log_path")
  message=${message//'%'/'%25'}
  message=${message//$'\r'/'%0D'}
  message=${message//$'\n'/'%0A'}
  printf '::error title=CI command failed (exit %s)::%s\n' \
    "$status" "$message" >&2
fi

exit "$status"
