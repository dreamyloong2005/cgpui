#include "wayland_internal.hpp"

void zxdg_decoration_manager_v1_destroy(
    zxdg_decoration_manager_v1* manager) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(manager), 0, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(manager)),
      WL_MARSHAL_FLAG_DESTROY);
}

zxdg_toplevel_decoration_v1*
zxdg_decoration_manager_v1_get_toplevel_decoration(
    zxdg_decoration_manager_v1* manager,
    xdg_toplevel* toplevel) {
  return reinterpret_cast<zxdg_toplevel_decoration_v1*>(wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(manager), 1,
      &zxdg_toplevel_decoration_v1_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(manager)), 0,
      nullptr, toplevel));
}

int zxdg_toplevel_decoration_v1_add_listener(
    zxdg_toplevel_decoration_v1* decoration,
    const zxdg_toplevel_decoration_v1_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(decoration),
      reinterpret_cast<void (**)(void)>(
          const_cast<zxdg_toplevel_decoration_v1_listener*>(listener)),
      data);
}

void zxdg_toplevel_decoration_v1_destroy(
    zxdg_toplevel_decoration_v1* decoration) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(decoration), 0, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(decoration)),
      WL_MARSHAL_FLAG_DESTROY);
}

void zxdg_toplevel_decoration_v1_set_mode(
    zxdg_toplevel_decoration_v1* decoration,
    std::uint32_t mode) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(decoration), 1, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(decoration)), 0, mode);
}

void zxdg_toplevel_decoration_v1_unset_mode(
    zxdg_toplevel_decoration_v1* decoration) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(decoration), 2, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(decoration)), 0);
}
