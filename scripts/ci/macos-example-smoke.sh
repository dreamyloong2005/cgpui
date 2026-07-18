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
if [[ "${MACOSX_DEPLOYMENT_TARGET:-}" != "13.0" ]]; then
  echo "MACOSX_DEPLOYMENT_TARGET must be 13.0" >&2
  exit 3
fi
if [[ "$(xmake --version | sed -n '1p')" != *"xmake v3.0.9"* ]]; then
  echo "macos-example-smoke.sh requires xmake v3.0.9" >&2
  exit 3
fi
cd "$repo_root"

while IFS= read -r target; do
  [[ -z "$target" || "$target" == \#* ]] && continue
  if [[ ! "$target" =~ ^api_parity_[a-z0-9_]+$ ]]; then
    echo "invalid example target: $target" >&2
    exit 3
  fi
  xmake build -P "$repo_root" -y -j 1 "$target"
  if [[ "$target" == "api_parity_hello_world" ]]; then
    CGPUI_EXIT_AFTER_FIRST_FRAME=1 xmake run -P "$repo_root" "$target"
  else
    xmake run -P "$repo_root" "$target"
  fi
done < "$repo_root/scripts/ci/example-targets.txt"

native_tests=(
  macos_example_smoke_test/default
  metal_primitive_pixel_test/default
  metal_text_image_pixel_test/default
  metal_clip_transform_pixel_test/default
  macos_input_event_test/default
  macos_text_input_test/default
  macos_platform_services_test/default
  macos_accessibility_test/default
  phase_f_multi_window_lifecycle_churn_test/default
)
xmake test -y -P "$repo_root" -j 1 -v "${native_tests[@]}"
for smoke in \
  hello_window/macos_first_frame \
  hello_window/macos_resize_after_first_frame \
  hello_window/macos_demo_smoke_flow \
  hello_window/macos_close_after_first_frame; do
  xmake test -y -P "$repo_root" -j 1 -v "$smoke"
done

printf '%s\n' "macOS example and smoke matrix passed"
