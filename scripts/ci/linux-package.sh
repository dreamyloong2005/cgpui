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
workspace_root="$output_root/workspace"
package_root="$output_root/package"
lib_root="$package_root/lib"
bin_root="$package_root/bin"
include_root="$package_root/include"
mkdir -p "$workspace_root" "$lib_root" "$bin_root" "$include_root"

source "$repo_root/scripts/ci/linux-dependencies.sh"
cgpui_configure_linux_dependency_environment
profile=dev
[[ "$mode" == release ]] && profile=release
# The system libstdc++ (host-coupled), as consumers of the SDK link it.
python3 "$repo_root/scripts/ci/sdk-package.py" workspace \
  --repo-root "$repo_root" --output "$workspace_root" --cxx-runtime host-coupled
# Every member: selecting programs alone archives no library (mcpp links a
# program from its dependencies' objects).
(cd "$workspace_root" && "$MCPP" build --profile "$profile")

build_dirs=()
for candidate in "$workspace_root"/target/x86_64-linux-gnu/*/; do
  [[ -f "$candidate/build.ninja" ]] && build_dirs+=("${candidate%/}")
done
if [[ "${#build_dirs[@]}" -ne 1 ]]; then
  echo "expected one mcpp build directory under $workspace_root/target" >&2
  exit 3
fi
binary_root="${build_dirs[0]}/bin"
printf '%s\n' "${build_dirs[0]}" > "$output_root/build-dir.txt"
libraries=(
  "core/libcgpui_core.a"
  "platform/libcgpui_platform.a"
  "platform-linux-wayland/libcgpui_platform_linux_wayland.a"
  "platform-linux-x11/libcgpui_platform_linux_x11.a"
  "platform-linux-atspi/libcgpui_platform_linux_atspi.a"
  "renderer/libcgpui_renderer.a"
  "renderer-vulkan/libcgpui_renderer_vulkan.a"
  "ui/libcgpui_ui.a"
  "app/libcgpui_app.a"
)
for entry in "${libraries[@]}"; do
  source_path="$binary_root/$entry"
  if [[ ! -f "$source_path" ]]; then
    echo "missing Linux $mode library: $source_path" >&2
    exit 3
  fi
  install -m 0644 "$source_path" "$lib_root/${entry#*/}"
done
libraries=("${libraries[@]#*/}")

demo="$binary_root/hello-window/hello_window"
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
