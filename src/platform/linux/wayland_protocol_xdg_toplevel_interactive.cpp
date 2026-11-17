#include "wayland_internal.hpp"
#include "wayland_protocol_xdg_toplevel_interactive_internal.hpp"

namespace {

// Opcodes from xdg-shell.xml: show_window_menu 4, move 5, resize 6.
wl_proxy* proxy(xdg_toplevel* toplevel) {
  return reinterpret_cast<wl_proxy*>(toplevel);
}

} // namespace

void xdg_toplevel_show_window_menu(
    xdg_toplevel* toplevel, wl_seat* seat, std::uint32_t serial,
    std::int32_t x, std::int32_t y) {
  wl_proxy_marshal_flags(
      proxy(toplevel), 4, nullptr, wl_proxy_get_version(proxy(toplevel)), 0,
      seat, serial, x, y);
}

void xdg_toplevel_move(
    xdg_toplevel* toplevel, wl_seat* seat, std::uint32_t serial) {
  wl_proxy_marshal_flags(
      proxy(toplevel), 5, nullptr, wl_proxy_get_version(proxy(toplevel)), 0,
      seat, serial);
}

void xdg_toplevel_resize(
    xdg_toplevel* toplevel, wl_seat* seat, std::uint32_t serial,
    std::uint32_t edges) {
  wl_proxy_marshal_flags(
      proxy(toplevel), 6, nullptr, wl_proxy_get_version(proxy(toplevel)), 0,
      seat, serial, edges);
}
