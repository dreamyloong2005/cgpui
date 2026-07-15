#include "macos_text_services_internal.hpp"

namespace cgpui {

Rect macos_candidate_rect_to_screen(
    Rect rect,
    Size framebuffer_size,
    DpiScale scale,
    Point window_origin) {
  const float factor = scale.value > 0.0F ? scale.value : 1.0F;
  const float framebuffer_height = framebuffer_size.height / factor;
  return Rect{
      .origin = Point{
          window_origin.x + rect.origin.x,
          window_origin.y + framebuffer_height - rect.origin.y - rect.size.height},
      .size = rect.size};
}

}  // namespace cgpui
