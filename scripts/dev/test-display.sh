# Private X display for the Linux tests; sourced by scripts/dev/test.sh.
#
# cgpui_start_test_display starts Xvfb and exports DISPLAY to it for this
# process and the tests it starts; the EXIT trap stops it. The session's
# display is left alone: on WSLg or a desktop, a test that owns the X11
# CLIPBOARD selection would otherwise replace the host clipboard (WSLg copies
# it into Windows), and runs on Windows at the same time would fail on the
# busy clipboard.
#
# The display number is the first from :90 with no lock file. Readiness is
# a reply to an X request (xdpyinfo when installed), because on WSLg
# /tmp/.X11-unix is a read-only mount: Xvfb listens on its abstract socket
# only and no socket file appears.
#
# CGPUI_TEST_DISPLAY=session  use the session's DISPLAY instead (no Xvfb).
# Without Xvfb (Arch: xorg-server-xvfb, Debian/Ubuntu: xvfb) the tests run
# on the session's DISPLAY, and a warning says so.
# Without xdpyinfo, the lock file Xvfb writes before it accepts clients.
cgpui_test_display_ready() {
  if command -v xdpyinfo > /dev/null 2>&1; then
    xdpyinfo -display "$1" > /dev/null 2>&1
  else
    [ -e "/tmp/.X${1#:}-lock" ]
  fi
}

cgpui_start_test_display() {
  if [ "${CGPUI_TEST_DISPLAY:-}" = session ]; then
    return 0
  fi
  if ! command -v Xvfb > /dev/null 2>&1; then
    echo "warning: Xvfb not found; X11 tests use the session DISPLAY (${DISPLAY:-unset})" >&2
    return 0
  fi
  local number log
  log=$(mktemp "${TMPDIR:-/tmp}/cgpui-xvfb.XXXXXX")
  # test.sh runs under set -e: every test below is an if, never a bare &&.
  for number in $(seq 90 189); do
    if [ -e "/tmp/.X$number-lock" ]; then
      continue
    fi
    Xvfb ":$number" -screen 0 1280x720x24 -nolisten tcp > "$log" 2>&1 &
    CGPUI_XVFB_PID=$!
    trap 'kill "$CGPUI_XVFB_PID" 2> /dev/null; wait "$CGPUI_XVFB_PID" 2> /dev/null; rm -f "'"$log"'"' EXIT
    local _
    for _ in $(seq 1 100); do
      if ! kill -0 "$CGPUI_XVFB_PID" 2> /dev/null; then
        break
      fi
      if cgpui_test_display_ready ":$number"; then
        export DISPLAY=":$number"
        return 0
      fi
      sleep 0.05
    done
    kill "$CGPUI_XVFB_PID" 2> /dev/null || true
    wait "$CGPUI_XVFB_PID" 2> /dev/null || true
    trap - EXIT
  done
  echo "warning: no Xvfb display could be started; X11 tests use the session DISPLAY" >&2
  tail -3 "$log" >&2 || true
  rm -f "$log"
}
