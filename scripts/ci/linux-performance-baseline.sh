#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
backend=${CGPUI_LINUX_BACKEND:-wayland}
case "$backend" in
  wayland|x11) ;;
  *) echo "CGPUI_LINUX_BACKEND must be wayland or x11" >&2; exit 2 ;;
esac
output_dir=${CGPUI_PERFORMANCE_OUTPUT_DIR:-artifacts/phase-j-performance}
iterations=${CGPUI_PERFORMANCE_ITERATIONS:-64}
mkdir -p "$root/$output_dir"
cd "$root"
xmake build -j 1 phase_j_performance_runner
xmake run phase_j_performance_runner \
  --platform "$backend" --output "$output_dir/$backend.json" \
  --iterations "$iterations"
python tools/gpui_parity/validate_performance_baseline.py \
  --policy docs/gpui-performance-baseline-policy.json \
  --report "$output_dir/$backend.json"
