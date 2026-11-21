#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
source "$root/scripts/ci/macos-dependencies.sh"
arch=${CGPUI_EXPECTED_ARCH:-arm64}
cgpui_require_macos_arm64 "$arch"
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "26.0" ]] || exit 3
mcpp="${MCPP:-mcpp}"
[[ "$("$mcpp" --version | sed -n '1p')" == "mcpp 2026."* ]] || exit 3

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
# The runner is the test program of tests/stress (arguments after --).
"$mcpp" test --profile release -p tests/stress phase_j_stress_runner -- --platform "macos-$arch" \
  --output "$output_path/macos-$arch.json" --scale "$scale"
python3 tools/gpui_parity/validate_stress_report.py \
  --profile docs/gpui-stress-profile.json \
  --report "$output_path/macos-$arch.json"
