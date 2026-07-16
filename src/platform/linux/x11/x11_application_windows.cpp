#include "x11_application_internal.hpp"
#include "x11_window_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<PlatformWindow>> X11Application::create_window(
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback) {
  return create_window_with_parent(descriptor, std::move(callback), screen_->root);
}

Result<std::unique_ptr<PlatformWindow>> X11Application::create_child_window(
    const WindowDescriptor& descriptor,
    PlatformWindow& parent,
    PlatformEventCallback callback) {
  const NativeSurfaceHandle surface = parent.native_surface();
  const auto* x11 = std::get_if<X11SurfaceHandle>(&surface);
  if (x11 == nullptr || x11->window == 0 || x11->display != connection_) {
    return std::unexpected(x11_error(
        ErrorCode::invalid_argument, "X11 child window requires an X11 parent"));
  }
  return create_window_with_parent(
      descriptor, std::move(callback), static_cast<xcb_window_t>(x11->window));
}

Result<std::unique_ptr<PlatformWindow>> X11Application::create_window_with_parent(
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    xcb_window_t parent) {
  auto window = X11Window::create(
      connection_, screen_, atoms_, scale_, parent, descriptor, std::move(callback),
      [this](X11Window* removed) { unregister_window(removed); });
  if (!window) return std::unexpected(window.error());
  windows_.push_back(window->get());
  return std::unique_ptr<PlatformWindow>(std::move(*window));
}

void X11Application::unregister_window(X11Window* window) {
  windows_.erase(
      std::remove(windows_.begin(), windows_.end(), window), windows_.end());
}

}  // namespace cgpui
