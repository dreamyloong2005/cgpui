#include "cgpui/renderer/svg_image_upload.hpp"

#include "cgpui/renderer/renderer_reports.hpp"
#include "cgpui/renderer/renderer_frame.hpp"

namespace cgpui {

bool SvgImageUploadResult::ready() const {
  return status == SvgRasterizationStatus::ready && uploaded;
}

SvgImageUploadResult upload_svg_image(
    RenderFrame& frame,
    SvgRasterCache& cache,
    const SvgRasterizationRequest& request) {
  const SvgRasterCacheLookup lookup = cache.rasterize(request);
  SvgImageUploadResult result{
      .status = lookup.status,
      .cache_hit = lookup.cache_hit,
  };
  if (!lookup.ready()) {
    return result;
  }

  frame.upload_image(lookup.result->image);
  result.image = lookup.result->plan.descriptor;
  result.uploaded = true;
  return result;
}

} // namespace cgpui
