#!/usr/bin/env bash
set -euo pipefail

[[ "$(uname -s)" == "Darwin" ]] || exit 2
arch=${1:-}
mode=${2:-}
[[ "$arch" =~ ^(arm64|x86_64)$ ]] || { echo "invalid architecture" >&2; exit 2; }
[[ "$mode" =~ ^(debug|release)$ ]] || { echo "invalid mode" >&2; exit 2; }
[[ "$(uname -m)" == "$arch" ]] || { echo "native architecture required" >&2; exit 2; }
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 2
xmake_version=$(xmake --version | sed -n '1p')
[[ "$xmake_version" == *"xmake v3.0.9"* ]] || exit 3

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
output_input=${3:-build/phase-j-release/macos-$arch}
[[ "/$output_input/" != */../* ]] || exit 2
if [[ "$output_input" == /* ]]; then
  output_candidate="$output_input"
else
  output_candidate="$repo_root/$output_input"
fi
output_probe="$output_candidate"
output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
[[ -d "$output_probe" ]] || exit 2
output_root="$(cd "$output_probe" && pwd -P)$output_suffix"
[[ "$output_root" != "$repo_root" && "$output_root/" == "$repo_root/"* ]] || exit 2

rm -rf -- "$output_root"
build_root="$output_root/build-root"
package_root="$output_root/package"
lib_root="$package_root/lib"
bin_root="$package_root/bin"
include_root="$package_root/include"
metadata_root="$package_root/metadata"
mkdir -p "$build_root" "$output_root/config" "$output_root/global" \
  "$output_root/pkg-cache" "$output_root/pkg-install" "$output_root/tmp" \
  "$lib_root" "$bin_root" "$include_root" "$metadata_root"

export XMAKE_CONFIGDIR="${XMAKE_CONFIGDIR:-$output_root/config}"
export XMAKE_GLOBALDIR="${XMAKE_GLOBALDIR:-$output_root/global}"
export XMAKE_PKG_CACHEDIR="${XMAKE_PKG_CACHEDIR:-$output_root/pkg-cache}"
export XMAKE_PKG_INSTALLDIR="${XMAKE_PKG_INSTALLDIR:-$output_root/pkg-install}"
export TMPDIR="${TMPDIR:-$output_root/tmp}"

xmake f -P "$repo_root" -y -c -m "$mode" -a "$arch" --ccache=n -o "$build_root"
xmake build -P "$repo_root" -y -j 1 cgpui_app
xmake build -P "$repo_root" -y -j 1 hello_window

binary_root="$build_root/macosx/$arch/$mode"
libraries=(
  libcgpui_core.a
  libcgpui_platform.a
  libcgpui_platform_macos.a
  libcgpui_renderer.a
  libcgpui_renderer_metal.a
  libcgpui_ui.a
  libcgpui_app.a
)
for library in "${libraries[@]}"; do
  source_path="$binary_root/$library"
  [[ -f "$source_path" ]] || { echo "missing macOS library: $source_path" >&2; exit 3; }
  install -m 0644 "$source_path" "$lib_root/$library"
done

demo="$binary_root/hello_window"
[[ -f "$demo" ]] || { echo "missing macOS demo: $demo" >&2; exit 3; }
install -m 0755 "$demo" "$bin_root/hello_window"
cp -R "$repo_root/include/cgpui" "$include_root/"
install -m 0644 "$repo_root/README.md" "$package_root/README.md"
install -m 0644 "$repo_root/xmake-requires.lock" \
  "$metadata_root/xmake-requires.lock"

product_version=$(sw_vers -productVersion)
build_version=$(sw_vers -buildVersion)
xcode_version=$(xcodebuild -version | paste -sd ';' -)
python3 - "$metadata_root/build.json" "$arch" "$xmake_version" \
  "$product_version" "$build_version" "$xcode_version" <<'PY'
import json
import pathlib
import sys

path = pathlib.Path(sys.argv[1])
metadata = {
    "schema_version": 1,
    "architecture": sys.argv[2],
    "deployment_target": "13.0",
    "xmake_version": sys.argv[3],
    "macos_product_version": sys.argv[4],
    "macos_build_version": sys.argv[5],
    "xcode_version": sys.argv[6],
}
path.write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
if json.loads(path.read_text(encoding="utf-8")) != metadata:
    raise SystemExit("macOS build metadata validation failed")
PY

python3 - "$package_root/manifest.json" "$arch" "$mode" \
  "${libraries[@]}" <<'PY'
import json
import pathlib
import sys

path = pathlib.Path(sys.argv[1])
arch = sys.argv[2]
mode = sys.argv[3]
libraries = sys.argv[4:]
manifest = {
    "schema_version": 1,
    "package": f"cgpui-macos-{arch}-{mode}",
    "platform": "macos",
    "architecture": arch,
    "mode": mode,
    "deployment_target": "13.0",
    "header_root": "include/cgpui",
    "libraries": [f"lib/{name}" for name in libraries],
    "executables": ["bin/hello_window"],
    "documents": ["README.md"],
    "metadata": ["metadata/build.json", "metadata/xmake-requires.lock"],
}
path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
if json.loads(path.read_text(encoding="utf-8")) != manifest:
    raise SystemExit("macOS package manifest validation failed")
PY

required_paths=(
  include/cgpui bin/hello_window README.md manifest.json
  metadata/build.json metadata/xmake-requires.lock
)
for library in "${libraries[@]}"; do required_paths+=("lib/$library"); done
for relative_path in "${required_paths[@]}"; do
  [[ -e "$package_root/$relative_path" ]] || {
    echo "missing packaged artifact: $relative_path" >&2
    exit 3
  }
done

repo_header_count=$(find "$repo_root/include/cgpui" -type f | wc -l)
package_header_count=$(find "$package_root/include/cgpui" -type f | wc -l)
[[ "$repo_header_count" -gt 0 && "$package_header_count" -eq "$repo_header_count" ]] || {
  echo "macOS public header count mismatch" >&2
  exit 3
}
for library in "${libraries[@]}"; do
  cmp -s "$binary_root/$library" "$lib_root/$library" || {
    echo "staged library differs from build output: $library" >&2
    exit 3
  }
done

printf '%s\n' "$package_root"
