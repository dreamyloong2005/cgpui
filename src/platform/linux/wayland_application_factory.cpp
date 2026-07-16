#include "wayland_application_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<PlatformApplication>> create_wayland_application() {
  return std::make_unique<WaylandApplication>();
}

} // namespace cgpui
