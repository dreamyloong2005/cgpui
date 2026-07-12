#pragma once

#include "cgpui/renderer/svg_rasterization.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace cgpui {

enum class SvgAssetDecodeStatus {
  ready,
  empty_input,
  invalid_svg,
  invalid_intrinsic_size,
  exceeds_limits,
};

struct SvgAssetDecodeLimits {
  std::size_t max_encoded_bytes = 8U * 1024U * 1024U;
  float max_intrinsic_dimension = 16384.0F;
  std::size_t max_intrinsic_pixels = 64U * 1024U * 1024U;
};

struct DecodedSvgAsset {
  std::string source;
  Size intrinsic_size;
};

struct SvgAssetDecodeResult {
  SvgAssetDecodeStatus status = SvgAssetDecodeStatus::empty_input;
  DecodedSvgAsset asset;

  [[nodiscard]] bool ready() const;
};

struct SvgAssetRasterizationOptions {
  ImageAssetId asset_id;
  DpiScale scale{1.0F};
  Size viewport_size;
  std::optional<Color> current_color;
};

[[nodiscard]] SvgAssetDecodeResult decode_svg_asset(
    std::span<const std::uint8_t> encoded,
    SvgAssetDecodeLimits limits = {});
[[nodiscard]] SvgRasterizationResult rasterize_svg_asset(
    const DecodedSvgAsset& asset,
    const SvgAssetRasterizationOptions& options);

} // namespace cgpui
