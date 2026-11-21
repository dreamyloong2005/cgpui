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

escape_message() {
  local message=$1
  message=${message//%/%25}
  message=${message//$'\r'/%0D}
  message=${message//$'\n'/%0A}
  printf '%s' "$message"
}

if [[ $status -ne 0 && "${GITHUB_ACTIONS:-}" == "true" ]]; then
  message=$(escape_message "$(tail -n 80 "$log_path")")
  if [[ ${#message} -gt 3500 ]]; then
    message=${message: -3500}
  fi
  # Parallel builds can bury the first compiler, linker or mcpp diagnostic.
  first=$(awk '
    /(^|[[:space:]])(fatal )?error:|undefined reference to/ { found=1 }
    found { print substr($0, 1, 1200); if (++lines == 3) exit }
  ' "$log_path")
  first=$(escape_message "$first")
  if [[ -n "$first" && "$message" != *"$first"* ]]; then
    if [[ ${#message} -gt 2000 ]]; then message=${message: -2000}; fi
    message="First diagnostic: ${first:0:1400}%0A...%0A$message"
  fi
  printf '::error title=CI command failed (exit %s)::%s\n' \
    "$status" "$message" >&2
fi

exit "$status"
