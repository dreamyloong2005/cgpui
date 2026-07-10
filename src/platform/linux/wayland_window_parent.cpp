#include "wayland_window_internal.hpp"

namespace cgpui {

xdg_toplevel* WaylandWindow::toplevel() const {
  return toplevel_;
}

} // namespace cgpui
