#!/usr/bin/env bash
# Build and run the cgpui tests on Linux: scripts/dev/test.sh [mcpp test args]
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/mcpp-env.sh"
cd "$REPO_ROOT"
export MCPP_COMPILE_COMMANDS="${MCPP_COMPILE_COMMANDS:-compile_commands.json}"
exec "$MCPP" test "${profile_args[@]}" "$@"
