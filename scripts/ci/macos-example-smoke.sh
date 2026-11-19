#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "macos-example-smoke.sh requires macOS" >&2
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
cd "$repo_root"
# Not run since the mcpp migration (no Mac available); mcpp runs the
# programs it builds, and test names are those of tests/platform and
# examples/hello_window.
mcpp="${MCPP:-mcpp}"

while IFS= read -r target; do
  [[ -z "$target" || "$target" == \#* ]] && continue
  if [[ ! "$target" =~ ^api_parity_[a-z0-9_]+$ ]]; then
    echo "invalid example target: $target" >&2
    exit 3
  fi
  if [[ "$target" == "api_parity_hello_world" ]]; then
    CGPUI_EXIT_AFTER_FIRST_FRAME=1 "$mcpp" run -p examples/api_parity "$target"
  else
    "$mcpp" run -p examples/api_parity "$target"
  fi
done < "$repo_root/scripts/ci/macos-example-targets.txt"

"$mcpp" test -p tests/platform macos_example_smoke_test
for smoke in \
  macos_first_frame \
  macos_resize_after_first_frame \
  macos_demo_smoke_flow \
  macos_close_after_first_frame; do
  "$mcpp" test -p examples/hello_window "$smoke"
done

printf '%s\n' "macOS example and smoke matrix passed"
