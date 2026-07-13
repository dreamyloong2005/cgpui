#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
exec "$script_dir/linux-package.sh" release \
  "${1:-build/phase-g-ci/linux-release}"
