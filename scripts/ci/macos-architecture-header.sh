#!/usr/bin/env bash
set -euo pipefail

[[ "$(uname -s)" == "Darwin" ]] || exit 2
repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
arch=${CGPUI_EXPECTED_ARCH:-$(uname -m)}
[[ "$arch" =~ ^(arm64|x86_64)$ && "$(uname -m)" == "$arch" ]] || exit 3
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 3
[[ "$(xmake --version | sed -n '1p')" == *"xmake v3.0.9"* ]] || exit 3

matrix="$repo_root/scripts/ci/architecture-header-targets.txt"
targets=()
while IFS='|' read -r target source extra; do
  [[ -z "$target" || "$target" == \#* ]] && continue
  [[ -n "$source" && -z "$extra" && "$target" =~ ^[a-z0-9_]+$ ]] || exit 4
  [[ -f "$repo_root/$source" ]] || exit 4
  targets+=("$target")
done < "$matrix"
[[ ${#targets[@]} -gt 0 ]] || exit 4

cd "$repo_root"
for target in "${targets[@]}"; do
  xmake build -P "$repo_root" -y -j 1 "$target"
done
filters=()
for target in "${targets[@]}"; do filters+=("$target/default"); done
xmake test -P "$repo_root" -y -j 1 -v "${filters[@]}"
