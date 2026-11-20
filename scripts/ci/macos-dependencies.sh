#!/usr/bin/env bash
set -euo pipefail

# The mcpp environment of the macOS CI scripts: MCPP_HOME, the mcpp
# executable ($MCPP) and a TMPDIR inside the dependency root. Dependencies
# are Xcode's toolchain and frameworks (Cocoa, Metal, CoreText) and the
# vendored third_party/ libraries; mcpp downloads nothing for them. (The
# xmake version pinned an xmake-repo commit here; mcpp has no package
# repository to pin.) Not run since the mcpp migration (no Mac available).
cgpui_configure_macos_dependency_environment() {
  if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "macos-dependencies.sh requires macOS" >&2
    return 2
  fi

  local repo_root input candidate probe suffix root export_github
  repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
  input="${1:-build/phase-j-ci/dependencies/macos-$(uname -m)}"
  export_github="${2:-}"
  case "/$input/" in
    */../*)
      echo "dependency root must not contain parent traversal: $input" >&2
      return 2
      ;;
  esac
  if [[ "$input" == /* ]]; then
    candidate="$input"
  else
    candidate="$repo_root/$input"
  fi
  probe="$candidate"
  suffix=""
  while [[ ! -e "$probe" ]]; do
    suffix="/$(basename "$probe")$suffix"
    probe="$(dirname "$probe")"
  done
  if [[ ! -d "$probe" ]]; then
    echo "dependency root ancestor must be a directory: $probe" >&2
    return 2
  fi
  root="$(cd "$probe" && pwd -P)$suffix"
  if [[ "$root" == "$repo_root" ]]; then
    echo "dependency root must not be the repository root" >&2
    return 2
  fi
  case "$root/" in
    "$repo_root/"*) ;;
    *)
      echo "dependency root must stay inside the repository: $root" >&2
      return 2
      ;;
  esac

  export MCPP_HOME="${MCPP_HOME:-$HOME/.mcpp}"
  if [[ -z "${MCPP:-}" ]]; then
    MCPP="$(command -v mcpp || true)"
  fi
  if [[ -z "$MCPP" || ! -x "$MCPP" ]]; then
    echo "mcpp not found (set MCPP)" >&2
    return 3
  fi
  export MCPP
  export TMPDIR="$root/tmp"
  mkdir -p "$TMPDIR"
  if ! xcrun --find clang++ >/dev/null 2>&1; then
    echo "Xcode command line tools are required (xcrun clang++)" >&2
    return 3
  fi

  if [[ "$export_github" == "--github-env" ]]; then
    if [[ -z "${GITHUB_ENV:-}" ]]; then
      echo "GITHUB_ENV is required when exporting dependency variables" >&2
      return 2
    fi
    printf '%s\n' "MCPP_HOME=$MCPP_HOME" "MCPP=$MCPP" "TMPDIR=$TMPDIR" \
      >> "$GITHUB_ENV"
  elif [[ -n "$export_github" ]]; then
    echo "usage: macos-dependencies.sh [dependency-root] [--github-env]" >&2
    return 2
  fi
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  cgpui_configure_macos_dependency_environment "$@"
fi
