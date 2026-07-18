#!/usr/bin/env bash
set -euo pipefail

[[ "$(uname -s)" == "Darwin" ]] || { echo "macOS required" >&2; exit 2; }
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
arch=${CGPUI_EXPECTED_ARCH:-$(uname -m)}
[[ "$arch" =~ ^(arm64|x86_64)$ && "$(uname -m)" == "$arch" ]] || exit 3
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 3
[[ "$(xmake --version | sed -n '1p')" == *"xmake v3.0.9"* ]] || exit 3

output_dir=${CGPUI_STRESS_OUTPUT_DIR:-artifacts/phase-j-stress}
scale=${CGPUI_STRESS_SCALE:-1}
[[ "/$output_dir/" != */../* ]] || exit 3
if [[ "$output_dir" == /* ]]; then
  output_path="$output_dir"
else
  output_path="$root/$output_dir"
fi
output_probe="$output_path"
output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
[[ -d "$output_probe" ]] || exit 3
output_path="$(cd "$output_probe" && pwd -P)$output_suffix"
[[ "$output_path" != "$root" && "$output_path/" == "$root/"* ]] || exit 3
mkdir -p "$output_path"

cd "$root"
xmake f -P "$root" -y -m release -a "$arch" \
  --target_minver="$MACOSX_DEPLOYMENT_TARGET"
xmake build -P "$root" -y -j 1 phase_j_stress_runner
xmake run -P "$root" phase_j_stress_runner --platform "macos-$arch" \
  --output "$output_path/macos-$arch.json" --scale "$scale"
python3 tools/gpui_parity/validate_stress_report.py \
  --profile docs/gpui-stress-profile.json \
  --report "$output_path/macos-$arch.json"
