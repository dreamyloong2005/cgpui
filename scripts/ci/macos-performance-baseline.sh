#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "macos-performance-baseline.sh requires macOS" >&2
  exit 2
fi

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
arch="${CGPUI_EXPECTED_ARCH:-$(uname -m)}"
if [[ ! "$arch" =~ ^(arm64|x86_64)$ ]] || [[ "$(uname -m)" != "$arch" ]]; then
  echo "macOS host architecture does not match: $arch" >&2
  exit 3
fi
if [[ "${MACOSX_DEPLOYMENT_TARGET:-}" != "13.0" ]]; then
  echo "MACOSX_DEPLOYMENT_TARGET must be 13.0" >&2
  exit 3
fi
mcpp="${MCPP:-mcpp}"
if [[ "$("$mcpp" --version | sed -n '1p')" != "mcpp 2026."* ]]; then
  echo "macos-performance-baseline.sh requires mcpp 2026.11.20.1 or later" >&2
  exit 3
fi

output_input="${CGPUI_PERFORMANCE_OUTPUT_DIR:-artifacts/phase-j-performance}"
case "/$output_input/" in
  */../*) echo "performance output must not contain parent traversal" >&2; exit 3 ;;
esac
if [[ "$output_input" == /* ]]; then
  output_candidate="$output_input"
else
  output_candidate="$root/$output_input"
fi
output_probe="$output_candidate"
output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
if [[ ! -d "$output_probe" ]]; then
  echo "performance output ancestor must be a directory" >&2
  exit 3
fi
output_path="$(cd "$output_probe" && pwd -P)$output_suffix"
if [[ "$output_path" == "$root" ]]; then
  echo "performance output must not be the repository root" >&2
  exit 3
fi
case "$output_path/" in
  "$root/"*) ;;
  *) echo "performance output must stay inside the repository" >&2; exit 3 ;;
esac

iterations="${CGPUI_PERFORMANCE_ITERATIONS:-64}"
mkdir -p "$output_path"
cd "$root"
# The runner is the test program of tests/performance; mcpp test passes the
# arguments after -- to it (as linux-performance-baseline.sh).
"$mcpp" test --profile release -p tests/performance phase_j_performance_runner -- \
  --platform "macos-$arch" --output "$output_path/macos-$arch.json" \
  --iterations "$iterations"
python3 tools/gpui_parity/validate_performance_baseline.py \
  --policy docs/gpui-performance-baseline-policy.json \
  --report "$output_path/macos-$arch.json"
