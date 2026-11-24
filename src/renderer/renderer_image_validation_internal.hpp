#pragma once
#include "cgpui/core/error.hpp"
#include "cgpui/renderer/planning/image_geometry.hpp"

namespace cgpui {
struct RendererValidatedImageDraw {
  std::vector<ImageVertex> vertices;
  std::optional<Rect> clip_rect;
};
bool renderer_same_image_allocation(const ImageAssetDescriptor& lhs,
                                    const ImageAssetDescriptor& rhs);
Result<RendererValidatedImageDraw> renderer_validate_image_draw(const ImageDraw& draw);
} // namespace cgpui
