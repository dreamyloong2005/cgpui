#include "cgpui/renderer/svg_asset_decode.hpp"

#include <lunasvg.h>

#include <cmath>
#include <utility>

namespace cgpui {

bool SvgAssetDecodeResult::ready() const {
  return status == SvgAssetDecodeStatus::ready;
}

SvgAssetDecodeResult decode_svg_asset(
    std::span<const std::uint8_t> encoded,
    SvgAssetDecodeLimits limits) {
  SvgAssetDecodeResult result;
  if (encoded.empty()) return result;
  if (encoded.size() > limits.max_encoded_bytes) {
    result.status = SvgAssetDecodeStatus::exceeds_limits;
    return result;
  }

  std::string source(
      reinterpret_cast<const char*>(encoded.data()), encoded.size());
  const auto document =
      lunasvg::Document::loadFromData(source.data(), source.size());
  if (!document) {
    result.status = SvgAssetDecodeStatus::invalid_svg;
    return result;
  }
  const float width = document->width();
  const float height = document->height();
  if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0F ||
      height <= 0.0F) {
    result.status = SvgAssetDecodeStatus::invalid_intrinsic_size;
    return result;
  }
  const double pixels = static_cast<double>(width) * height;
  if (width > limits.max_intrinsic_dimension ||
      height > limits.max_intrinsic_dimension ||
      pixels > static_cast<double>(limits.max_intrinsic_pixels)) {
    result.status = SvgAssetDecodeStatus::exceeds_limits;
    return result;
  }

  result.status = SvgAssetDecodeStatus::ready;
  result.asset = DecodedSvgAsset{
      .source = std::move(source),
      .intrinsic_size = {.width = width, .height = height},
  };
  return result;
}

} // namespace cgpui
