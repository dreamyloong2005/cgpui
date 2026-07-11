#include "wayland_internal.hpp"

void wp_viewporter_destroy(wp_viewporter* viewporter) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(viewporter), 0, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(viewporter)),
      WL_MARSHAL_FLAG_DESTROY);
}

wp_viewport* wp_viewporter_get_viewport(
    wp_viewporter* viewporter,
    wl_surface* surface) {
  return reinterpret_cast<wp_viewport*>(wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(viewporter), 1, &wp_viewport_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(viewporter)), 0,
      nullptr, surface));
}

void wp_viewport_destroy(wp_viewport* viewport) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(viewport), 0, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(viewport)),
      WL_MARSHAL_FLAG_DESTROY);
}

void wp_viewport_set_destination(
    wp_viewport* viewport,
    std::int32_t width,
    std::int32_t height) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(viewport), 2, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(viewport)), 0,
      width, height);
}
