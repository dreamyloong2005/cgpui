#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "macos-debug.sh requires macOS" >&2
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
arch=${CGPUI_EXPECTED_ARCH:-$(uname -m)}
if [[ ! "$arch" =~ ^(arm64|x86_64)$ || "$(uname -m)" != "$arch" ]]; then
  echo "macos-debug.sh requires the requested native architecture" >&2
  exit 2
fi
export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-13.0}"
if [[ "$MACOSX_DEPLOYMENT_TARGET" != "13.0" ]]; then
  echo "macos-debug.sh requires macOS deployment target 13.0" >&2
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

xmake_version="$(xmake --version | sed -n '1p')"
if [[ "$xmake_version" != *"xmake v3.0.9"* ]]; then
  echo "macos-debug.sh requires xmake v3.0.9: $xmake_version" >&2
  exit 3
fi

printf 'macOS %s (%s)\n' "$(sw_vers -productVersion)" \
  "$(sw_vers -buildVersion)"
xcodebuild -version
printf '%s\n' "$xmake_version"

needs_dependency_setup=false
for variable in XMAKE_CONFIGDIR XMAKE_GLOBALDIR XMAKE_PKG_CACHEDIR \
  XMAKE_PKG_INSTALLDIR TMPDIR; do
  [[ -n "${!variable:-}" ]] || needs_dependency_setup=true
done
if [[ "$needs_dependency_setup" == true ]]; then
  source "$repo_root/scripts/ci/macos-dependencies.sh"
  cgpui_configure_macos_dependency_environment "$output_root/dependencies"
fi
export XMAKE_CONFIGDIR="${XMAKE_CONFIGDIR:-$output_root/config}"
export XMAKE_GLOBALDIR="${XMAKE_GLOBALDIR:-$output_root/global}"
export XMAKE_PKG_CACHEDIR="${XMAKE_PKG_CACHEDIR:-$output_root/pkg-cache}"
export XMAKE_PKG_INSTALLDIR="${XMAKE_PKG_INSTALLDIR:-$output_root/pkg-install}"
export TMPDIR="${TMPDIR:-$output_root/tmp}"
mkdir -p "$XMAKE_CONFIGDIR" "$XMAKE_GLOBALDIR/.xmake/repositories" \
  "$XMAKE_PKG_CACHEDIR" "$XMAKE_PKG_INSTALLDIR" "$TMPDIR"

cd "$repo_root"
xmake f -y -c -m debug -P "$repo_root"
xmake test -y -P "$repo_root"
git diff --check

printf '%s\n' "macOS Debug verification passed"
