#include "cgpui/renderer/svg_rasterization.hpp"

#include "svg_raster_colorization_internal.hpp"

#include <lunasvg.h>

#include <cstring>
#include <limits>

namespace cgpui {

SvgRasterizationResult rasterize_svg(
    const SvgRasterizationRequest& request) {
  SvgRasterizationResult result;
  result.plan = plan_svg_rasterization(request);
  if (!result.plan.ready()) {
    return result;
  }

  const auto document = lunasvg::Document::loadFromData(
      request.svg_source.data(), request.svg_source.size());
  if (!document) {
    result.status = SvgRasterizationStatus::invalid_svg;
    return result;
  }
  if (result.plan.colorization.recolors_current_color) {
    document->documentElement().setAttribute(
        "color", detail::svg_current_color_css(result.plan.colorization));
  }

  const ImageAssetDescriptor& descriptor = result.plan.descriptor;
  if (descriptor.pixel_width >
          static_cast<std::uint32_t>(std::numeric_limits<int>::max()) ||
      descriptor.pixel_height >
          static_cast<std::uint32_t>(std::numeric_limits<int>::max())) {
    result.status = SvgRasterizationStatus::rasterization_failed;
    return result;
  }
  lunasvg::Bitmap bitmap = document->renderToBitmap(
      static_cast<int>(descriptor.pixel_width),
      static_cast<int>(descriptor.pixel_height));
  if (bitmap.isNull() || bitmap.data() == nullptr ||
      bitmap.width() != static_cast<int>(descriptor.pixel_width) ||
      bitmap.height() != static_cast<int>(descriptor.pixel_height) ||
      bitmap.stride() < static_cast<int>(descriptor.stride)) {
    result.status = SvgRasterizationStatus::rasterization_failed;
    return result;
  }

  bitmap.convertToRGBA();
  std::vector<std::uint8_t> pixels(descriptor.byte_size);
  for (std::uint32_t row = 0; row < descriptor.pixel_height; ++row) {
    std::memcpy(
        pixels.data() + static_cast<std::size_t>(row) * descriptor.stride,
        bitmap.data() + static_cast<std::size_t>(row) * bitmap.stride(),
        descriptor.stride);
  }

  result.status = SvgRasterizationStatus::ready;
  result.image = ImageAsset{
      .id = descriptor.id,
      .logical_size = descriptor.logical_size,
      .bitmap = DecodedImageBitmap{
          .width = descriptor.pixel_width,
          .height = descriptor.pixel_height,
          .stride = descriptor.stride,
          .format = descriptor.format,
          .pixels = std::move(pixels),
      },
  };
  return result;
}

} // namespace cgpui
