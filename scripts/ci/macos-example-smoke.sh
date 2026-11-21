#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "macos-example-smoke.sh requires macOS" >&2
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
expected_arch="${CGPUI_EXPECTED_ARCH:-$(uname -m)}"
if [[ "$(uname -m)" != "$expected_arch" ]] ||
   [[ ! "$expected_arch" =~ ^(arm64|x86_64)$ ]]; then
  echo "macOS host architecture does not match: $expected_arch" >&2
  exit 3
fi
# mcpp reads MACOSX_DEPLOYMENT_TARGET as the minimum macOS of what it links
# (mcpp docs 04, macos_deployment_target); the xmake script passed it as
# --target_minver="$MACOSX_DEPLOYMENT_TARGET".
if [[ "${MACOSX_DEPLOYMENT_TARGET:-}" != "13.0" ]]; then
  echo "MACOSX_DEPLOYMENT_TARGET must be 13.0" >&2
  exit 3
fi
cd "$repo_root"
# mcpp runs the programs it builds, and test names are those of tests/platform,
# tests/renderer, tests/ui and examples/hello_window.
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
done < "$repo_root/scripts/ci/example-targets.txt"

# The native tests of the xmake script, by member. The xmake list named
# phase_f_multi_window_lifecycle_churn_test/default, which is no target
# (docs/migration/README.md, macOS follow-up).
"$mcpp" test -p tests/platform macos_example_smoke_test
"$mcpp" test -p tests/renderer metal_primitive_pixel_test
"$mcpp" test -p tests/renderer metal_text_image_pixel_test
"$mcpp" test -p tests/renderer metal_clip_transform_pixel_test
"$mcpp" test -p tests/platform macos_input_event_test
"$mcpp" test -p tests/platform macos_text_input_test
"$mcpp" test -p tests/platform macos_platform_services_test
"$mcpp" test -p tests/platform macos_accessibility_test
for smoke in \
  macos_first_frame \
  macos_resize_after_first_frame \
  macos_demo_smoke_flow \
  macos_close_after_first_frame; do
  "$mcpp" test -p examples/hello_window "$smoke"
done

printf '%s\n' "macOS example and smoke matrix passed"
