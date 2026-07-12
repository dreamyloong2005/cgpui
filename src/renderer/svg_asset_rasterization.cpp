#include "cgpui/renderer/svg_asset_decode.hpp"

namespace cgpui {

SvgRasterizationResult rasterize_svg_asset(
    const DecodedSvgAsset& asset,
    const SvgAssetRasterizationOptions& options) {
  return rasterize_svg(SvgRasterizationRequest{
      .asset_id = options.asset_id,
      .logical_size = asset.intrinsic_size,
      .svg_source = asset.source,
      .scale = options.scale,
      .viewport_size = options.viewport_size,
      .current_color = options.current_color,
  });
}

} // namespace cgpui
