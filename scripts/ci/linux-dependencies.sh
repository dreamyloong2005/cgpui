#!/usr/bin/env bash
set -euo pipefail

# The mcpp environment of the CI scripts (scripts/dev/mcpp-env.sh): MCPP_HOME
# and the mcpp executable ($MCPP). Dependencies are the system GCC (toolchain
# path /usr) and system libraries found through pkg-config (see
# docs/platform-dependencies.md); mcpp downloads no toolchain for them.
cgpui_configure_linux_dependency_environment() {
  if [[ "$(uname -s)" != "Linux" ]]; then
    echo "linux-dependencies.sh requires Linux" >&2
    return 2
  fi
  local repo_root
  repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
  # shellcheck source=../dev/mcpp-env.sh
  source "$repo_root/scripts/dev/mcpp-env.sh"
  export MCPP
  local module
  for module in wayland-client wayland-cursor wayland-server dbus-1 xkbcommon \
      xkbcommon-x11 xcb xcb-xkb xcb-cursor harfbuzz freetype2 fontconfig vulkan; do
    if ! pkg-config --exists "$module"; then
      echo "missing system library (pkg-config): $module" >&2
      return 3
    fi
  done
  if [[ "${1:-}" == "--github-env" ]]; then
    if [[ -z "${GITHUB_ENV:-}" ]]; then
      echo "GITHUB_ENV is required when exporting dependency variables" >&2
      return 2
    fi
    printf '%s\n' "MCPP_HOME=$MCPP_HOME" "MCPP=$MCPP" >> "$GITHUB_ENV"
  fi
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  cgpui_configure_linux_dependency_environment "$@"
fi
