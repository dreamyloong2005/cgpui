#pragma once

#include "cgpui/renderer/svg_raster_cache.hpp"

namespace cgpui {

class RenderFrame;

struct SvgImageUploadResult {
  SvgRasterizationStatus status = SvgRasterizationStatus::invalid_request;
  ImageAssetDescriptor image;
  bool cache_hit = false;
  bool uploaded = false;

  [[nodiscard]] bool ready() const;
};

[[nodiscard]] SvgImageUploadResult upload_svg_image(
    RenderFrame& frame,
    SvgRasterCache& cache,
    const SvgRasterizationRequest& request);

} // namespace cgpui
