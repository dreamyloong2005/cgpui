#include "vulkan_image_color_internal.hpp"
#include "cgpui/renderer/planning/image_color.hpp"

namespace cgpui {

Color vulkan_resolve_image_tint(const ImageDraw& draw) {
  return resolve_image_tint(draw);
}

} // namespace cgpui
