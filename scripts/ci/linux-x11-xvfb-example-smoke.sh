#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "linux-x11-xvfb-example-smoke.sh requires Linux" >&2
  exit 2
fi
if [[ "${CGPUI_XVFB_MOUNT_NAMESPACE:-}" != "1" ]]; then
  if [[ "$(id -u)" -ne 0 ]]; then
    echo "isolated Xvfb socket setup requires root" >&2
    exit 2
  fi
  exec unshare --mount --fork env CGPUI_XVFB_MOUNT_NAMESPACE=1 \
    bash "$0" "$@"
fi

mount --make-rprivate /
mount -t tmpfs -o mode=1777,nosuid,nodev tmpfs /tmp
display_number="${CGPUI_XVFB_DISPLAY_NUMBER:-107}"
export DISPLAY=":$display_number"
Xvfb "$DISPLAY" -screen 0 1280x720x24 -nolisten tcp >/tmp/cgpui-xvfb.log 2>&1 &
xvfb_pid="$!"
cleanup() {
  kill "$xvfb_pid" 2>/dev/null || true
  wait "$xvfb_pid" 2>/dev/null || true
}
trap cleanup EXIT

for _ in {1..100}; do
  if xdpyinfo -display "$DISPLAY" >/dev/null 2>&1; then
    break
  fi
  kill -0 "$xvfb_pid" 2>/dev/null || {
    cat /tmp/cgpui-xvfb.log >&2
    exit 3
  }
  sleep 0.1
done
xdpyinfo -display "$DISPLAY" >/dev/null

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
bash "$repo_root/scripts/ci/linux-x11-example-smoke.sh" "$@"
