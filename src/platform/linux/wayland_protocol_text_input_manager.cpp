#include "wayland_internal.hpp"

void zwp_text_input_manager_v3_destroy(zwp_text_input_manager_v3* manager) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(manager),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(manager)),
      WL_MARSHAL_FLAG_DESTROY);
}

zwp_text_input_v3* zwp_text_input_manager_v3_get_text_input(
    zwp_text_input_manager_v3* manager,
    wl_seat* seat) {
  wl_proxy* proxy = wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(manager),
      1,
      &zwp_text_input_v3_interface,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(manager)),
      0,
      nullptr,
      seat);
  return reinterpret_cast<zwp_text_input_v3*>(proxy);
}
