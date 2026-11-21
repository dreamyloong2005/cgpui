#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
source "$repo_root/scripts/ci/macos-dependencies.sh"
cgpui_require_macos_arm64
# mcpp reads MACOSX_DEPLOYMENT_TARGET before [build] macos_deployment_target
# (mcpp docs 04, macos_deployment_target); its own default is 14.0.
export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-26.0}"
if [[ "$MACOSX_DEPLOYMENT_TARGET" != "26.0" ]]; then
  echo "macos-debug.sh requires macOS deployment target 26.0" >&2
  exit 2
fi
output_input="${1:-$repo_root/build/phase-h-ci/macos-debug}"
if [[ "$output_input" != /* ]]; then
  output_input="$repo_root/$output_input"
fi
case "/$output_input/" in
  */../*)
    echo "output root must not contain parent traversal: $output_input" >&2
    exit 2
    ;;
esac
output_probe="$output_input"
output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
if [[ ! -d "$output_probe" ]]; then
  echo "output root ancestor must be a directory: $output_probe" >&2
  exit 2
fi
output_root="$(cd "$output_probe" && pwd -P)$output_suffix"
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
mkdir -p "$output_root"
output_root="$(cd "$output_root" && pwd -P)"

mcpp="${MCPP:-mcpp}"
mcpp_version="$("$mcpp" --version | sed -n '1p')"
if [[ "$mcpp_version" != "mcpp 2026."* ]]; then
  echo "macos-debug.sh requires mcpp 2026.11.20.1 or later: $mcpp_version" >&2
  exit 3
fi

printf 'macOS %s (%s)\n' "$(sw_vers -productVersion)" \
  "$(sw_vers -buildVersion)"
xcodebuild -version
printf '%s\n' "$mcpp_version"

if [[ -z "${TMPDIR:-}" ]]; then
  cgpui_configure_macos_dependency_environment "$output_root/dependencies"
fi
export TMPDIR="${TMPDIR:-$output_root/tmp}"
mkdir -p "$TMPDIR"

cd "$repo_root"
"$mcpp" build
"$mcpp" test
git diff --check

printf '%s\n' "macOS Debug verification passed"
