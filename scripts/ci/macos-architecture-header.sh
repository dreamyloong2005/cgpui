#!/usr/bin/env bash
set -euo pipefail

[[ "$(uname -s)" == "Darwin" ]] || exit 2
repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
arch=${CGPUI_EXPECTED_ARCH:-$(uname -m)}
[[ "$arch" =~ ^(arm64|x86_64)$ && "$(uname -m)" == "$arch" ]] || exit 3
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 3
mcpp="${MCPP:-mcpp}"
[[ "$("$mcpp" --version | sed -n '1p')" == "mcpp 2026."* ]] || exit 3

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
# The matrix names every test program of the two suites (mcpp test names
# are file stems), as linux-architecture-header.sh checks.
listed="$("$mcpp" test --list -p tests/architecture -p tests/header_cleanliness |
  grep -E '^[a-z0-9_]+$' | sort)"
[[ "$listed" == "$(printf '%s\n' "${targets[@]}" | sort)" ]] || exit 4
"$mcpp" test -p tests/architecture -p tests/header_cleanliness
