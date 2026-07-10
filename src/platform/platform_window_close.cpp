#include "cgpui/platform/platform_window.hpp"
#include "platform_window_close_internal.hpp"

namespace cgpui {

bool PlatformWindowCloseController::begin(WindowCloseRequestSource source) {
  if (state_.pending || state_.accepted) {
    ++state_.coalesced_count;
    return false;
  }
  state_.pending = true;
  state_.accepted = false;
  state_.source = source;
  ++state_.sequence;
  return true;
}

bool PlatformWindowCloseController::resolve(
    PlatformWindowCloseResolution resolution) {
  if (!state_.pending) {
    return false;
  }
  state_.pending = false;
  state_.accepted = resolution == PlatformWindowCloseResolution::accept;
  if (state_.accepted) {
    ++state_.accepted_count;
  } else {
    ++state_.cancelled_count;
  }
  return true;
}

PlatformWindowCloseState PlatformWindowCloseController::state() const {
  return state_;
}

WindowCloseRequested PlatformWindowCloseController::event() const {
  return WindowCloseRequested{
      .source = state_.source,
      .sequence = state_.sequence};
}

PlatformWindowCloseState PlatformWindow::close_request_state() const {
  const bool requested = state().close_requested;
  return PlatformWindowCloseState{
      .pending = false,
      .accepted = requested,
      .sequence = requested ? 1U : 0U};
}

bool PlatformWindow::resolve_close_request(
    PlatformWindowCloseResolution resolution) {
  return resolution == PlatformWindowCloseResolution::accept &&
      state().close_requested;
}

} // namespace cgpui
