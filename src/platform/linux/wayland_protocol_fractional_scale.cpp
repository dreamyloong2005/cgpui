#include "wayland_internal.hpp"

void wp_fractional_scale_manager_v1_destroy(
    wp_fractional_scale_manager_v1* manager) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(manager), 0, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(manager)),
      WL_MARSHAL_FLAG_DESTROY);
}

wp_fractional_scale_v1* wp_fractional_scale_manager_v1_get_fractional_scale(
    wp_fractional_scale_manager_v1* manager,
    wl_surface* surface) {
  return reinterpret_cast<wp_fractional_scale_v1*>(wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(manager), 1,
      &wp_fractional_scale_v1_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(manager)), 0,
      nullptr, surface));
}

int wp_fractional_scale_v1_add_listener(
    wp_fractional_scale_v1* fractional_scale,
    const wp_fractional_scale_v1_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(fractional_scale),
      reinterpret_cast<void (**)(void)>(
          const_cast<wp_fractional_scale_v1_listener*>(listener)),
      data);
}

void wp_fractional_scale_v1_destroy(
    wp_fractional_scale_v1* fractional_scale) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(fractional_scale), 0, nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(fractional_scale)),
      WL_MARSHAL_FLAG_DESTROY);
}
