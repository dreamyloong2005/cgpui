#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "macos-example-smoke.sh requires macOS" >&2
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
cd "$repo_root"

while IFS= read -r target; do
  [[ -z "$target" || "$target" == \#* ]] && continue
  if [[ ! "$target" =~ ^api_parity_[a-z0-9_]+$ ]]; then
    echo "invalid example target: $target" >&2
    exit 3
  fi
  xmake -r -P "$repo_root" -y "$target"
  if [[ "$target" == "api_parity_hello_world" ]]; then
    CGPUI_EXIT_AFTER_FIRST_FRAME=1 xmake run -P "$repo_root" "$target"
  else
    xmake run -P "$repo_root" "$target"
  fi
done < "$repo_root/scripts/ci/macos-example-targets.txt"

xmake test -y -P "$repo_root" macos_example_smoke_test/default
for smoke in \
  hello_window/macos_first_frame \
  hello_window/macos_resize_after_first_frame \
  hello_window/macos_demo_smoke_flow \
  hello_window/macos_close_after_first_frame; do
  xmake test -y -P "$repo_root" "$smoke"
done

printf '%s\n' "macOS example and smoke matrix passed"
