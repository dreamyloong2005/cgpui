#include "wayland_internal.hpp"

int xdg_wm_base_add_listener(
    xdg_wm_base* shell,
    const xdg_wm_base_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(shell),
      reinterpret_cast<void (**)(void)>(
          const_cast<xdg_wm_base_listener*>(listener)),
      data);
}

void xdg_wm_base_destroy(xdg_wm_base* shell) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(shell),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(shell)),
      WL_MARSHAL_FLAG_DESTROY);
}

xdg_surface* xdg_wm_base_get_xdg_surface(
    xdg_wm_base* shell,
    wl_surface* surface) {
  wl_proxy* proxy = wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(shell),
      2,
      &xdg_surface_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(shell)),
      0,
      nullptr,
      surface);
  return reinterpret_cast<xdg_surface*>(proxy);
}

void xdg_wm_base_pong(xdg_wm_base* shell, std::uint32_t serial) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(shell),
      3,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(shell)),
      0,
      serial);
}
