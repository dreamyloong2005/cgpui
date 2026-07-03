#include "wayland_internal.hpp"

int zwp_text_input_v3_add_listener(
    zwp_text_input_v3* text_input,
    const zwp_text_input_v3_listener* listener,
    void* data) {
  return wl_proxy_add_listener(
      reinterpret_cast<wl_proxy*>(text_input),
      reinterpret_cast<void (**)(void)>(
          const_cast<zwp_text_input_v3_listener*>(listener)),
      data);
}

void zwp_text_input_v3_destroy(zwp_text_input_v3* text_input) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      0,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      WL_MARSHAL_FLAG_DESTROY);
}

void zwp_text_input_v3_enable(zwp_text_input_v3* text_input) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      1,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0);
}

void zwp_text_input_v3_disable(zwp_text_input_v3* text_input) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      2,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0);
}

void zwp_text_input_v3_set_surrounding_text(
    zwp_text_input_v3* text_input,
    const char* text,
    std::int32_t cursor,
    std::int32_t anchor) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      3,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0,
      text,
      cursor,
      anchor);
}

void zwp_text_input_v3_set_content_type(
    zwp_text_input_v3* text_input,
    std::uint32_t hint,
    std::uint32_t purpose) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      5,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0,
      hint,
      purpose);
}

void zwp_text_input_v3_set_cursor_rectangle(
    zwp_text_input_v3* text_input,
    std::int32_t x,
    std::int32_t y,
    std::int32_t width,
    std::int32_t height) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      6,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0,
      x,
      y,
      width,
      height);
}

void zwp_text_input_v3_commit(zwp_text_input_v3* text_input) {
  wl_proxy_marshal_flags(
      reinterpret_cast<wl_proxy*>(text_input),
      7,
      nullptr,
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(text_input)),
      0);
}
