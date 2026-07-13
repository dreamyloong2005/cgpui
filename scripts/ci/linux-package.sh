#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "linux-package.sh requires Linux" >&2
  exit 2
fi

mode="${1:-}"
case "$mode" in
  debug|release) ;;
  *)
    echo "usage: linux-package.sh <debug|release> [output-root]" >&2
    exit 2
    ;;
esac

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
output_input="${2:-build/phase-g-ci/linux-$mode}"
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

rm -rf -- "$output_root"
build_root="$output_root/build-root"
global_root="$output_root/global"
config_root="$output_root/config"
package_cache="$output_root/pkg-cache"
package_install="$output_root/pkg-install"
cleanup_tmp_root=""
if [[ -n "${CGPUI_CI_TMPDIR:-}" ]]; then
  tmp_parent="$(realpath -m "$CGPUI_CI_TMPDIR")"
  mkdir -p "$tmp_parent"
  tmp_root="$(mktemp -d "$tmp_parent/cgpui-linux-$mode.XXXXXX")"
  cleanup_tmp_root="$tmp_root"
else
  tmp_root="$output_root/tmp"
fi
cleanup() {
  if [[ -n "$cleanup_tmp_root" ]]; then
    rm -rf -- "$cleanup_tmp_root"
  fi
}
trap cleanup EXIT
package_root="$output_root/package"
lib_root="$package_root/lib"
bin_root="$package_root/bin"
include_root="$package_root/include"
mkdir -p "$build_root" "$global_root" "$config_root" "$package_cache" \
  "$package_install" "$tmp_root" "$lib_root" "$bin_root" "$include_root"

if ! command -v meson >/dev/null || ! command -v ninja >/dev/null; then
  tool_root="$output_root/python-tools"
  python3 -m venv "$tool_root"
  "$tool_root/bin/python" -m ensurepip --upgrade
  "$tool_root/bin/python" -m pip install --disable-pip-version-check \
    "meson==1.11.1" "ninja==1.13.0"
  export PATH="$tool_root/bin:$PATH"
fi

export XMAKE_GLOBALDIR="$global_root"
export XMAKE_CONFIGDIR="$config_root"
export XMAKE_PKG_CACHEDIR="$package_cache"
export XMAKE_PKG_INSTALLDIR="$package_install"
export TMPDIR="$tmp_root"

xrepo update-repo
xmake f -P "$repo_root" -y -c -m "$mode" --ccache=n -o "$build_root"
xmake build -P "$repo_root" -j 1 cgpui_app
xmake build -P "$repo_root" -j 1 hello_window

binary_root="$build_root/linux/x86_64/$mode"
libraries=(
  "libcgpui_core.a"
  "libcgpui_platform.a"
  "libcgpui_platform_linux_wayland.a"
  "libcgpui_renderer.a"
  "libcgpui_renderer_vulkan.a"
  "libcgpui_ui.a"
  "libcgpui_app.a"
)
for library in "${libraries[@]}"; do
  source_path="$binary_root/$library"
  if [[ ! -f "$source_path" ]]; then
    echo "missing Linux $mode library: $source_path" >&2
    exit 3
  fi
  install -m 0644 "$source_path" "$lib_root/$library"
done

demo="$binary_root/hello_window"
if [[ ! -f "$demo" ]]; then
  echo "missing Linux $mode demo: $demo" >&2
  exit 3
fi
install -m 0755 "$demo" "$bin_root/hello_window"
cp -R "$repo_root/include/cgpui" "$include_root/"
install -m 0644 "$repo_root/README.md" "$package_root/README.md"

manifest_path="$package_root/manifest.json"
python3 - "$manifest_path" "$mode" "${libraries[@]}" <<'PY'
import json
import pathlib
import sys

path = pathlib.Path(sys.argv[1])
mode = sys.argv[2]
manifest = {
    "schema_version": 1,
    "package": f"cgpui-linux-{mode}",
    "platform": "linux",
    "architecture": "x86_64",
    "mode": mode,
    "header_root": "include/cgpui",
    "libraries": [f"lib/{name}" for name in sys.argv[3:]],
    "executables": ["bin/hello_window"],
    "documents": ["README.md"],
}
path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
parsed = json.loads(path.read_text(encoding="utf-8"))
if parsed != manifest:
    raise SystemExit(f"Linux {mode} manifest validation failed")
PY

required_paths=(
  "include/cgpui"
  "bin/hello_window"
  "README.md"
  "manifest.json"
)
for library in "${libraries[@]}"; do
  required_paths+=("lib/$library")
done
for relative_path in "${required_paths[@]}"; do
  if [[ ! -e "$package_root/$relative_path" ]]; then
    echo "missing packaged artifact: $relative_path" >&2
    exit 3
  fi
done

repo_header_count="$(find "$repo_root/include/cgpui" -type f | wc -l)"
package_header_count="$(find "$package_root/include/cgpui" -type f | wc -l)"
if [[ "$repo_header_count" -eq 0 || "$package_header_count" -ne "$repo_header_count" ]]; then
  echo "Linux $mode public header count mismatch" >&2
  exit 3
fi

printf '%s\n' "$package_root"
