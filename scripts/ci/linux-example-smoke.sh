#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "linux-example-smoke.sh requires Linux" >&2
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
output_input="${1:-build/phase-g-ci/linux-examples-smoke}"
if [[ "$output_input" == /* ]]; then
  output_candidate="$output_input"
else
  output_candidate="$repo_root/$output_input"
fi
output_root="$(realpath -m "$output_candidate")"
if [[ "$output_root" == "$repo_root" ]]; then
  echo "output root must not be the repository root" >&2
  exit 2
fi
case "$output_root/" in
  "$repo_root/"*) ;;
  *)
    echo "output root must stay inside the repository: $output_root" >&2
    exit 2
    ;;
esac

if [[ "${CGPUI_CI_USE_CURRENT_BUILD:-}" == "1" ]]; then
  : "${XMAKE_GLOBALDIR:?current-build mode requires XMAKE_GLOBALDIR}"
  : "${XMAKE_PKG_CACHEDIR:?current-build mode requires XMAKE_PKG_CACHEDIR}"
  : "${XMAKE_PKG_INSTALLDIR:?current-build mode requires XMAKE_PKG_INSTALLDIR}"
  export TMPDIR="${TMPDIR:-/dev/shm/cgpui}"
  mkdir -p "$output_root" "$TMPDIR"
else
  if [[ -z "${CGPUI_CI_REUSE_PREPARED_ROOT:-}" ]]; then
    bash "$repo_root/scripts/ci/linux-package.sh" debug "$output_root"
  fi
  manifest="$output_root/package/manifest.json"
  build_root="$output_root/build-root"
  if [[ ! -f "$manifest" || ! -d "$build_root" ]]; then
    echo "prepared Linux package/build root is missing under $output_root" >&2
    exit 3
  fi

  source "$repo_root/scripts/ci/linux-dependencies.sh"
  cgpui_configure_linux_dependency_environment
  export XMAKE_CONFIGDIR="$output_root/config"
  if [[ -d "$output_root/python-tools/bin" ]]; then
    export PATH="$output_root/python-tools/bin:$PATH"
  fi
  export TMPDIR="$output_root/tmp"
  mkdir -p "$TMPDIR"
fi

weston_pid=""
runtime_owned=""
cleanup() {
  if [[ -n "$weston_pid" ]]; then
    kill "$weston_pid" 2>/dev/null || true
    wait "$weston_pid" 2>/dev/null || true
  fi
  if [[ -n "$runtime_owned" ]]; then
    rm -rf -- "$runtime_owned"
  fi
}
trap cleanup EXIT

if [[ -z "${XDG_RUNTIME_DIR:-}" ]]; then
  runtime_owned="$output_root/wayland-runtime"
  rm -rf -- "$runtime_owned"
  mkdir -m 0700 -p "$runtime_owned"
  export XDG_RUNTIME_DIR="$runtime_owned"
fi
if [[ -z "${WAYLAND_DISPLAY:-}" ||
      ! -S "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY" ]]; then
  if ! command -v weston >/dev/null; then
    echo "weston is required when no Wayland display is available" >&2
    exit 3
  fi
  export WAYLAND_DISPLAY="cgpui-ci"
  weston --backend=headless-backend.so --socket="$WAYLAND_DISPLAY" \
    --idle-time=0 --log="$output_root/weston.log" &
  weston_pid="$!"
  for _ in {1..100}; do
    [[ -S "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY" ]] && break
    kill -0 "$weston_pid" 2>/dev/null || {
      cat "$output_root/weston.log" >&2
      exit 3
    }
    sleep 0.1
  done
  if [[ ! -S "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY" ]]; then
    echo "headless Weston did not create its Wayland socket" >&2
    exit 3
  fi
fi

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
      CGPUI_EXIT_AFTER_FIRST_FRAME=1 "$binary"
    else
      "$binary"
    fi
  else
    xmake build -P "$repo_root" -y -j 1 "$target"
    if [[ "$target" == "api_parity_hello_world" ]]; then
      CGPUI_EXIT_AFTER_FIRST_FRAME=1 xmake run -P "$repo_root" "$target"
    else
      xmake run -P "$repo_root" "$target"
    fi
  fi
done

registration_smokes=(
  api_parity_public_animation_example
  api_parity_public_opacity_example
  api_parity_public_image_example
  api_parity_public_gif_viewer_example
)
for target in "${registration_smokes[@]}"; do
  if [[ "${CGPUI_CI_SKIP_BUILD:-}" == "1" ]]; then
    "$CGPUI_CI_BINARY_ROOT/$target"
  else
    xmake test -P "$repo_root" -j 1 -v "$target/*"
  fi
done

if [[ "${CGPUI_CI_SKIP_BUILD:-}" == "1" ]]; then
  demo="$CGPUI_CI_BINARY_ROOT/hello_window"
  CGPUI_EXIT_AFTER_FIRST_FRAME=1 "$demo"
  CGPUI_RESIZE_AFTER_FIRST_FRAME=1 "$demo"
  CGPUI_CLOSE_AFTER_FIRST_FRAME=1 "$demo"
  CGPUI_DEMO_SMOKE_FLOW=1 CGPUI_CLOSE_AFTER_FIRST_FRAME=0 "$demo"
else
  for test_name in linux_first_frame linux_resize_after_first_frame \
    linux_close_after_first_frame linux_demo_smoke_flow; do
    xmake test -P "$repo_root" -j 1 -v "hello_window/$test_name"
  done
fi

printf '%s\n' "Linux example and smoke matrix passed"
