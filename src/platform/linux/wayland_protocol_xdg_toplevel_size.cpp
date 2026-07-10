#include "wayland_internal.hpp"

namespace {

void set_toplevel_size(
    xdg_toplevel* toplevel,
    std::uint32_t opcode,
    std::int32_t width,
    std::int32_t height) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(toplevel), opcode, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(toplevel)), 0,
      width, height);
}

} // namespace

void xdg_toplevel_set_max_size(
    xdg_toplevel* toplevel,
    std::int32_t width,
    std::int32_t height) {
  set_toplevel_size(toplevel, 7, width, height);
}

void xdg_toplevel_set_min_size(
    xdg_toplevel* toplevel,
    std::int32_t width,
    std::int32_t height) {
  set_toplevel_size(toplevel, 8, width, height);
}
