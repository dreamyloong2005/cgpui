#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "linux-architecture-header.sh requires Linux" >&2
  exit 2
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
output_input="${1:-build/phase-g-ci/linux-architecture-header}"
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

if [[ -z "${CGPUI_CI_REUSE_PREPARED_ROOT:-}" ]]; then
  bash "$repo_root/scripts/ci/linux-package.sh" debug "$output_root"
fi
if [[ ! -f "$output_root/package/manifest.json" ]]; then
  echo "prepared Linux package manifest is missing under $output_root" >&2
  exit 3
fi

source "$repo_root/scripts/ci/linux-dependencies.sh"
cgpui_configure_linux_dependency_environment
export XMAKE_CONFIGDIR="$output_root/config"
if [[ -d "$output_root/python-tools/bin" ]]; then
  export PATH="$output_root/python-tools/bin:$PATH"
fi
export TMPDIR="$output_root/tmp"
mkdir -p "$TMPDIR"

targets=()
matrix="$repo_root/scripts/ci/architecture-header-targets.txt"
while IFS='|' read -r target source extra; do
  [[ -z "$target" || "$target" == \#* ]] && continue
  if [[ -z "$source" || -n "$extra" || ! "$target" =~ ^[a-z0-9_]+$ ]]; then
    echo "invalid architecture/header matrix entry: $target|$source" >&2
    exit 3
  fi
  targets+=("$target")
done < "$matrix"
if [[ "${#targets[@]}" -eq 0 ]]; then
  echo "architecture/header target matrix is empty" >&2
  exit 3
fi

cd "$repo_root"
for target in "${targets[@]}"; do
  xmake build -P "$repo_root" -y -j 1 "$target"
done
filters=()
for target in "${targets[@]}"; do filters+=("$target/*"); done
xmake test -P "$repo_root" -j 1 -v "${filters[@]}"
printf '%s\n' "Linux architecture/header matrix passed (${#targets[@]} targets)"
