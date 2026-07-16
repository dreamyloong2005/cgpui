#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "macos-debug.sh requires macOS" >&2
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
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

export XMAKE_CONFIGDIR="$output_root/config"
export XMAKE_GLOBALDIR="$output_root/global"
export XMAKE_PKG_CACHEDIR="$output_root/pkg-cache"
export XMAKE_PKG_INSTALLDIR="$output_root/pkg-install"
export TMPDIR="$output_root/tmp"
mkdir -p "$XMAKE_CONFIGDIR" "$XMAKE_GLOBALDIR/.xmake/repositories" \
  "$XMAKE_PKG_CACHEDIR" "$XMAKE_PKG_INSTALLDIR" "$TMPDIR"

repository="$XMAKE_GLOBALDIR/.xmake/repositories/xmake-repo"
repository_commit="b9256335e0b6e70808e23dfe71627d8a4dcc0abf"
if [[ -d "$repository/.git" ]] &&
   [[ "$(git -C "$repository" config --get remote.origin.promisor || true)" == "true" ]]; then
  rm -rf -- "$repository"
fi
if [[ ! -d "$repository/.git" ]]; then
  git clone --no-checkout \
    https://github.com/xmake-io/xmake-repo.git "$repository"
fi
git -C "$repository" fetch --depth 1 origin "$repository_commit"
git -C "$repository" checkout --detach "$repository_commit"
touch "$repository/updated"

locked_repository="$XMAKE_CONFIGDIR/repositories/efa340bf3b6f6de54c5f0ab8c98fba7d.lock"
if [[ -d "$locked_repository/.git" ]] &&
   ! git -C "$locked_repository" cat-file -e "$repository_commit^{commit}"; then
  rm -rf -- "$locked_repository"
fi
if [[ ! -d "$locked_repository/.git" ]]; then
  mkdir -p "$(dirname "$locked_repository")"
  git clone --shared "$repository" "$locked_repository"
fi
git -C "$locked_repository" checkout --detach "$repository_commit"

cd "$repo_root"
xmake f -y -c -m debug -P "$repo_root"
xmake test -y -P "$repo_root"
git diff --check

printf '%s\n' "macOS Debug verification passed"
