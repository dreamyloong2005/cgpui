#!/usr/bin/env bash
set -euo pipefail

cgpui_configure_linux_dependency_environment() {
  if [[ "$(uname -s)" != "Linux" ]]; then
    echo "linux-dependencies.sh requires Linux" >&2
    return 2
  fi

  local repo_root dependency_input dependency_candidate dependency_root
  local export_github=""
  repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
  if [[ "${1:-}" == "--github-env" ]]; then
    dependency_input="$repo_root/build/phase-g-ci/dependencies/linux"
    export_github=1
  else
    dependency_input="${1:-$repo_root/build/phase-g-ci/dependencies/linux}"
    [[ "${2:-}" == "--github-env" ]] && export_github=1
  fi
  if [[ "$dependency_input" == /* ]]; then
    dependency_candidate="$dependency_input"
  else
    dependency_candidate="$repo_root/$dependency_input"
  fi
  dependency_root="$(realpath -m "$dependency_candidate")"
  if [[ "$dependency_root" == "$repo_root" ]]; then
    echo "dependency root must not be the repository root" >&2
    return 2
  fi
  case "$dependency_root/" in
    "$repo_root/"*) ;;
    *)
      echo "dependency root must stay inside the repository: $dependency_root" >&2
      return 2
      ;;
  esac

  export XMAKE_GLOBALDIR="$dependency_root/global"
  export XMAKE_PKG_CACHEDIR="$dependency_root/pkg-cache"
  export XMAKE_PKG_INSTALLDIR="$dependency_root/pkg-install"
  mkdir -p "$XMAKE_GLOBALDIR" "$XMAKE_PKG_CACHEDIR" \
    "$XMAKE_PKG_INSTALLDIR"

  if [[ -n "$export_github" ]]; then
    if [[ -z "${GITHUB_ENV:-}" ]]; then
      echo "GITHUB_ENV is required when exporting dependency variables" >&2
      return 2
    fi
    printf '%s\n' \
      "XMAKE_GLOBALDIR=$XMAKE_GLOBALDIR" \
      "XMAKE_PKG_CACHEDIR=$XMAKE_PKG_CACHEDIR" \
      "XMAKE_PKG_INSTALLDIR=$XMAKE_PKG_INSTALLDIR" >> "$GITHUB_ENV"
  fi
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  cgpui_configure_linux_dependency_environment "$@"
fi
