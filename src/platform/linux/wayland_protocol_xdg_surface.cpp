#include "wayland_internal.hpp"

int xdg_surface_add_listener(
    xdg_surface* surface,
    const xdg_surface_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(surface),
      reinterpret_cast<void (**)(void)>(
          const_cast<xdg_surface_listener*>(listener)),
      data);
}

void xdg_surface_destroy(xdg_surface* surface) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(surface),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(surface)),
      WL_MARSHAL_FLAG_DESTROY);
}

xdg_toplevel* xdg_surface_get_toplevel(xdg_surface* surface) {
  wl_proxy* proxy = wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(surface),
      1,
      &xdg_toplevel_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(surface)),
      0,
      nullptr);
  return reinterpret_cast<xdg_toplevel*>(proxy);
}

void xdg_surface_ack_configure(
    xdg_surface* surface,
    std::uint32_t serial) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(surface),
      4,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(surface)),
      0,
      serial);
}
