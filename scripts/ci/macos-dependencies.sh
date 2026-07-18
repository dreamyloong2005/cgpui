#!/usr/bin/env bash
set -euo pipefail

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

  export XMAKE_CONFIGDIR="$root/config"
  export XMAKE_GLOBALDIR="$root/global"
  export XMAKE_PKG_CACHEDIR="$root/pkg-cache"
  export XMAKE_PKG_INSTALLDIR="$root/pkg-install"
  export TMPDIR="$root/tmp"
  mkdir -p "$XMAKE_CONFIGDIR" "$XMAKE_GLOBALDIR/.xmake/repositories" \
    "$XMAKE_PKG_CACHEDIR" "$XMAKE_PKG_INSTALLDIR" "$TMPDIR"

  local repository commit locked_repository
  repository="$XMAKE_GLOBALDIR/.xmake/repositories/xmake-repo"
  commit="b9256335e0b6e70808e23dfe71627d8a4dcc0abf"
  if [[ -d "$repository/.git" ]] &&
     [[ "$(git -C "$repository" config --get remote.origin.promisor || true)" == "true" ]]; then
    rm -rf -- "$repository"
  fi
  if [[ ! -d "$repository/.git" ]]; then
    git clone --no-checkout https://github.com/xmake-io/xmake-repo.git \
      "$repository"
  fi
  git -C "$repository" fetch --depth 1 origin "$commit"
  git -C "$repository" checkout --detach "$commit"
  touch "$repository/updated"

  locked_repository="$XMAKE_CONFIGDIR/repositories/efa340bf3b6f6de54c5f0ab8c98fba7d.lock"
  if [[ -d "$locked_repository/.git" ]] &&
     ! git -C "$locked_repository" cat-file -e "$commit^{commit}"; then
    rm -rf -- "$locked_repository"
  fi
  if [[ ! -d "$locked_repository/.git" ]]; then
    mkdir -p "$(dirname "$locked_repository")"
    git clone --shared "$repository" "$locked_repository"
  fi
  git -C "$locked_repository" checkout --detach "$commit"

  if [[ "$export_github" == "--github-env" ]]; then
    if [[ -z "${GITHUB_ENV:-}" ]]; then
      echo "GITHUB_ENV is required when exporting dependency variables" >&2
      return 2
    fi
    printf '%s\n' \
      "XMAKE_CONFIGDIR=$XMAKE_CONFIGDIR" \
      "XMAKE_GLOBALDIR=$XMAKE_GLOBALDIR" \
      "XMAKE_PKG_CACHEDIR=$XMAKE_PKG_CACHEDIR" \
      "XMAKE_PKG_INSTALLDIR=$XMAKE_PKG_INSTALLDIR" \
      "TMPDIR=$TMPDIR" >> "$GITHUB_ENV"
  elif [[ -n "$export_github" ]]; then
    echo "usage: macos-dependencies.sh [dependency-root] [--github-env]" >&2
    return 2
  fi
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  cgpui_configure_macos_dependency_environment "$@"
fi
