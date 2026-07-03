#include "wayland_internal.hpp"

int xdg_toplevel_add_listener(
    xdg_toplevel* toplevel,
    const xdg_toplevel_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(toplevel),
      reinterpret_cast<void (**)(void)>(
          const_cast<xdg_toplevel_listener*>(listener)),
      data);
}

void xdg_toplevel_destroy(xdg_toplevel* toplevel) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)),
      WL_MARSHAL_FLAG_DESTROY);
}

void xdg_toplevel_set_title(
    xdg_toplevel* toplevel,
    const char* title) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel),
      2,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)),
      0,
      title);
}
