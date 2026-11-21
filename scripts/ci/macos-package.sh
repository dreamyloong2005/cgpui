#!/usr/bin/env bash
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
source "$repo_root/scripts/ci/macos-dependencies.sh"
arch=${1:-}
cgpui_require_macos_arm64 "$arch"
mode=${2:-}
[[ "$mode" =~ ^(debug|release)$ ]] || { echo "invalid mode" >&2; exit 2; }
# mcpp reads MACOSX_DEPLOYMENT_TARGET as the minimum macOS it links for; the
# xmake script passed --target_minver="$MACOSX_DEPLOYMENT_TARGET".
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 2

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
workspace_root="$output_root/workspace"
package_root="$output_root/package"
include_root="$package_root/include"
metadata_root="$package_root/metadata"
mkdir -p "$workspace_root" "$include_root" \
  "$metadata_root"

if [[ -z "${MCPP:-}" ]]; then
  cgpui_configure_macos_dependency_environment "$output_root/dependencies"
fi
mcpp_version=$("$MCPP" --version | sed -n '1p')
[[ "$mcpp_version" == "mcpp 2026."* ]] || exit 3
profile=dev
[[ "$mode" == release ]] && profile=release
# libc++ of the Xcode toolchain (host-coupled), as consumers link it.
python3 "$repo_root/scripts/ci/sdk-package.py" workspace \
  --repo-root "$repo_root" --output "$workspace_root" --cxx-runtime host-coupled
# Every member: selecting programs alone archives no library.
(cd "$workspace_root" && "$MCPP" build --profile "$profile")
build_dirs=()
for candidate in "$workspace_root"/target/*/*/; do
  [[ -f "$candidate/build.ninja" ]] && build_dirs+=("${candidate%/}")
done
[[ "${#build_dirs[@]}" -eq 1 ]] || {
  echo "expected one mcpp build directory under $workspace_root/target" >&2
  exit 3
}
cp -R "$repo_root/include/cgpui" "$include_root/"
install -m 0644 "$repo_root/docs/macos-sdk.md" "$package_root/README.md"
# Build inputs (the xmake package shipped xmake-requires.lock here): the
# workspace manifest and the vendored dependency record. mcpp keeps no lock
# file for this workspace (no registry dependencies).
install -m 0644 "$workspace_root/mcpp.toml" "$metadata_root/mcpp.toml"
install -m 0644 "$repo_root/third_party/VENDORED.md" \
  "$metadata_root/VENDORED.md"

product_version=$(sw_vers -productVersion)
build_version=$(sw_vers -buildVersion)
xcode_version=$(xcodebuild -version | paste -sd ';' -)
python3 - "$metadata_root/build.json" "$arch" "$mcpp_version" \
  "$product_version" "$build_version" "$xcode_version" <<'PY'
import json
import pathlib
import sys

path = pathlib.Path(sys.argv[1])
metadata = {
    "schema_version": 2,
    "architecture": sys.argv[2],
    "deployment_target": "13.0",
    "mcpp_version": sys.argv[3],
    "macos_product_version": sys.argv[4],
    "macos_build_version": sys.argv[5],
    "xcode_version": sys.argv[6],
}
path.write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
if json.loads(path.read_text(encoding="utf-8")) != metadata:
    raise SystemExit("macOS build metadata validation failed")
PY

python3 "$repo_root/scripts/ci/sdk-package.py" stage-macos \
  --repo-root "$repo_root" --build-dir "${build_dirs[0]}" --package-root "$package_root" \
  --arch "$arch" --mode "$mode"

required_paths=(
  include/cgpui bin/hello_window README.md manifest.json
  metadata/build.json metadata/mcpp.toml metadata/VENDORED.md
)
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
printf '%s\n' "$package_root"
