#!/usr/bin/env bash
# Build the cgpui workspace on Linux: scripts/dev/build.sh [mcpp build args]
# (see scripts/dev/build.ps1 for examples; the default profile is dev-lines).
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/mcpp-env.sh"
cd "$REPO_ROOT"
export MCPP_COMPILE_COMMANDS="${MCPP_COMPILE_COMMANDS:-compile_commands.json}"
exec "$MCPP" build "${profile_args[@]}" "$@"
