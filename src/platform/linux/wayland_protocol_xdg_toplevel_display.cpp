#include "wayland_internal.hpp"

namespace {

void xdg_toplevel_request(xdg_toplevel* toplevel, std::uint32_t opcode) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel),
      opcode,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)),
      0);
}

} // namespace

void xdg_toplevel_set_maximized(xdg_toplevel* toplevel) {
  xdg_toplevel_request(toplevel, 9);
}

void xdg_toplevel_unset_maximized(xdg_toplevel* toplevel) {
  xdg_toplevel_request(toplevel, 10);
}

void xdg_toplevel_set_fullscreen(
    xdg_toplevel* toplevel,
    wl_output* output) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel),
      11,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)),
      0,
      output);
}

void xdg_toplevel_unset_fullscreen(xdg_toplevel* toplevel) {
  xdg_toplevel_request(toplevel, 12);
}

void xdg_toplevel_set_minimized(xdg_toplevel* toplevel) {
  xdg_toplevel_request(toplevel, 13);
}

void xdg_toplevel_set_max_size(
    xdg_toplevel* toplevel,
    std::int32_t width,
    std::int32_t height) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel), 7, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)), 0,
      width, height);
}

void xdg_toplevel_set_min_size(
    xdg_toplevel* toplevel,
    std::int32_t width,
    std::int32_t height) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel), 8, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)), 0,
      width, height);
}
