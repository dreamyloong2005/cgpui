#include "wayland_application_internal.hpp"
#include "wayland_window_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<PlatformWindow>> WaylandApplication::create_child_window(
    const WindowDescriptor& descriptor,
    PlatformWindow& parent,
    PlatformEventCallback callback) {
  auto* registered_parent = dynamic_cast<RegisteredWaylandWindow*>(&parent);
  if (registered_parent == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "Wayland child window requires a Wayland parent"});
  }
  return create_window_with_parent(
      descriptor,
      std::move(callback),
      registered_parent->wayland_window().toplevel());
}

} // namespace cgpui
