# Environment for running mcpp on Linux; sourced by build.sh/test.sh.
#
# MCPP_HOME  mcpp's home. Default: ~/.mcpp.
# MCPP       mcpp to run. Default: mcpp on PATH, else the newest
#            $MCPP_HOME/mcpp-*/bin/mcpp.
# The dev-lines profile (line tables only, -g1) is the default: full -g
# debug information for ~560 test programs once filled the WSL disk. Builds
# and tests use the same profile so they share one build directory.
export MCPP_HOME="${MCPP_HOME:-$HOME/.mcpp}"

if [ -z "${MCPP:-}" ]; then
  MCPP=$(command -v mcpp || ls -d "$MCPP_HOME"/mcpp-*/bin/mcpp 2>/dev/null | sort | tail -1)
fi
[ -x "$MCPP" ] || { echo "mcpp not found (set MCPP)" >&2; exit 2; }
REPO_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
profile_args=(--profile dev-lines)
for arg in "$@"; do
  case "$arg" in --profile|--profile=*|--release|--dev) profile_args=() ;; esac
done
