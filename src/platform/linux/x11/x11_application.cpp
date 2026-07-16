#include "x11_application_internal.hpp"
#include "x11_scale_internal.hpp"
#include "x11_keyboard_internal.hpp"

#include <fcntl.h>
#include <unistd.h>

namespace cgpui {

Result<std::unique_ptr<X11Application>> X11Application::create() {
  auto application = std::unique_ptr<X11Application>(new X11Application());
  auto initialized = application->initialize();
  if (!initialized) return std::unexpected(initialized.error());
  return application;
}

Result<void> X11Application::initialize() {
  int screen_index = 0;
  connection_ = xcb_connect(nullptr, &screen_index);
  if (connection_ == nullptr || xcb_connection_has_error(connection_) != 0) {
    return std::unexpected(x11_error(
        ErrorCode::platform_initialization_failed,
        "xcb_connect failed for DISPLAY"));
  }
  screen_ = x11_screen(connection_, screen_index);
  if (screen_ == nullptr) {
    return std::unexpected(x11_error(
        ErrorCode::platform_initialization_failed,
        "X11 display has no selected screen"));
  }
  auto atoms = x11_load_atoms(connection_);
  if (!atoms) return std::unexpected(atoms.error());
  atoms_ = *atoms;
  scale_ = x11_display_scale(connection_, screen_);
  auto keyboard = create_x11_keyboard_state(connection_);
  if (!keyboard) return std::unexpected(keyboard.error());
  keyboard_ = std::move(*keyboard);
  if (xcb_cursor_context_new(connection_, screen_, &cursor_context_) < 0) {
    return std::unexpected(x11_error(
        ErrorCode::platform_initialization_failed,
        "X11 cursor context initialization failed"));
  }
  if (pipe2(wakeup_pipe_, O_NONBLOCK | O_CLOEXEC) != 0) {
    return std::unexpected(x11_error(
        ErrorCode::platform_initialization_failed,
        "pipe2 failed for X11 event-loop wakeup"));
  }
  return {};
}

X11Application::~X11Application() {
  if (cursor_context_ != nullptr) xcb_cursor_context_free(cursor_context_);
  if (wakeup_pipe_[0] != -1) close(wakeup_pipe_[0]);
  if (wakeup_pipe_[1] != -1) close(wakeup_pipe_[1]);
  if (connection_ != nullptr) xcb_disconnect(connection_);
}

PlatformReopenResult X11Application::request_reopen() {
  return dispatch_reopen("x11");
}

}  // namespace cgpui
