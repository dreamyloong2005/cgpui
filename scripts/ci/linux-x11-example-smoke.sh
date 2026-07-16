#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "linux-x11-example-smoke.sh requires Linux" >&2
  exit 2
fi
if [[ -z "${DISPLAY:-}" ]]; then
  echo "linux-x11-example-smoke.sh requires an X11 DISPLAY" >&2
  exit 2
fi
if [[ "$(id -u)" -eq 0 ]]; then
  export XMAKE_ROOT=y
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
mapfile -t targets < <(
  sed -e '/^[[:space:]]*$/d' -e '/^[[:space:]]*#/d' \
    "$repo_root/scripts/ci/example-targets.txt"
)
if [[ "${#targets[@]}" -eq 0 ]]; then
  echo "example target matrix is empty" >&2
  exit 3
fi

cd "$repo_root"
for target in "${targets[@]}"; do
  if [[ ! "$target" =~ ^api_parity_[a-z0-9_]+$ ]]; then
    echo "invalid example target: $target" >&2
    exit 3
  fi
  if [[ "${CGPUI_CI_SKIP_BUILD:-}" == "1" ]]; then
    binary_root="${CGPUI_CI_BINARY_ROOT:?skip-build mode requires CGPUI_CI_BINARY_ROOT}"
    binary="$binary_root/$target"
    [[ -x "$binary" ]] || { echo "missing example binary: $binary" >&2; exit 3; }
    if [[ "$target" == "api_parity_hello_world" ]]; then
      CGPUI_LINUX_BACKEND=x11 CGPUI_EXIT_AFTER_FIRST_FRAME=1 "$binary"
    else
      CGPUI_LINUX_BACKEND=x11 "$binary"
    fi
  else
    xmake build -P "$repo_root" -y -j 1 "$target"
    if [[ "$target" == "api_parity_hello_world" ]]; then
      CGPUI_LINUX_BACKEND=x11 CGPUI_EXIT_AFTER_FIRST_FRAME=1 \
        xmake run -P "$repo_root" "$target"
    else
      CGPUI_LINUX_BACKEND=x11 xmake run -P "$repo_root" "$target"
    fi
  fi
done

for target in \
  api_parity_public_animation_example \
  api_parity_public_opacity_example \
  api_parity_public_image_example \
  api_parity_public_gif_viewer_example; do
  if [[ "${CGPUI_CI_SKIP_BUILD:-}" == "1" ]]; then
    CGPUI_LINUX_BACKEND=x11 "$CGPUI_CI_BINARY_ROOT/$target"
  else
    CGPUI_LINUX_BACKEND=x11 xmake test -P "$repo_root" -j 1 -v "$target/*"
  fi
done

if [[ "${CGPUI_CI_SKIP_BUILD:-}" == "1" ]]; then
  demo="$CGPUI_CI_BINARY_ROOT/hello_window"
  CGPUI_LINUX_BACKEND=x11 CGPUI_EXIT_AFTER_FIRST_FRAME=1 "$demo"
  CGPUI_LINUX_BACKEND=x11 CGPUI_RESIZE_AFTER_FIRST_FRAME=1 "$demo"
  CGPUI_LINUX_BACKEND=x11 CGPUI_CLOSE_AFTER_FIRST_FRAME=1 "$demo"
  CGPUI_LINUX_BACKEND=x11 CGPUI_DEMO_SMOKE_FLOW=1 \
    CGPUI_CLOSE_AFTER_FIRST_FRAME=0 "$demo"
else
  for test_name in x11_first_frame x11_resize_after_first_frame \
    x11_close_after_first_frame x11_demo_smoke_flow; do
    CGPUI_LINUX_BACKEND=x11 \
      xmake test -P "$repo_root" -j 1 -v "hello_window/$test_name"
  done
fi

printf '%s\n' "Linux X11 example and smoke matrix passed"
