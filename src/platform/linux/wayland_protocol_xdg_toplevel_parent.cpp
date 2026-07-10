#include "wayland_internal.hpp"

void xdg_toplevel_set_parent(
    xdg_toplevel* toplevel,
    xdg_toplevel* parent) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel), 1, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)), 0, parent);
}
