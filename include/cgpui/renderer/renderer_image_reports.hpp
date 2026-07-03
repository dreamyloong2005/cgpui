#pragma once

#include <cstddef>

namespace cgpui {

struct RendererImageRenderReport {
  std::size_t image_draw_count = 0;
  std::size_t image_upload_plan_count = 0;
  std::size_t image_upload_byte_count = 0;
};

} // namespace cgpui
