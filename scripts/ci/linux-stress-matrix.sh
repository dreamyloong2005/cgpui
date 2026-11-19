#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
backend=${CGPUI_LINUX_BACKEND:-wayland}
case "$backend" in wayland|x11) ;; *) exit 2 ;; esac
output_dir=${CGPUI_STRESS_OUTPUT_DIR:-artifacts/phase-j-stress}
scale=${CGPUI_STRESS_SCALE:-1}
mkdir -p "$root/$output_dir"
cd "$root"
source scripts/ci/linux-dependencies.sh
cgpui_configure_linux_dependency_environment
# The runner is the test program of tests/stress (arguments after --).
"$MCPP" test "${profile_args[@]}" -p tests/stress -- --platform "$backend" \
  --output "$output_dir/$backend.json" --scale "$scale"
python tools/gpui_parity/validate_stress_report.py \
  --profile docs/gpui-stress-profile.json --report "$output_dir/$backend.json"
