#include "cgpui/core/asset_source.hpp"
#include "cgpui/renderer/svg_asset_decode.hpp"

#include <string_view>

namespace {

cgpui::AssetBytes svg_bytes(std::string_view source) {
  return cgpui::AssetBytes{.bytes = std::vector<std::uint8_t>(
                               source.begin(), source.end())};
}

int test_decodes_asset_bytes_and_rasterizes() {
  constexpr std::string_view source =
      "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"2\" height=\"1\">"
      "<rect width=\"2\" height=\"1\" fill=\"#ff0000\"/></svg>";
  const cgpui::SvgAssetDecodeResult decoded =
      cgpui::decode_svg_asset(svg_bytes(source).bytes);
  if (!decoded.ready() || decoded.asset.intrinsic_size.width != 2.0F ||
      decoded.asset.intrinsic_size.height != 1.0F ||
      decoded.asset.source != source) {
    return 10;
  }
  const cgpui::SvgRasterizationResult raster = cgpui::rasterize_svg_asset(
      decoded.asset,
      cgpui::SvgAssetRasterizationOptions{
          .asset_id = cgpui::ImageAssetId{73},
          .scale = cgpui::DpiScale{2.0F},
      });
  if (!raster.ready() || raster.image.id.value != 73 ||
      raster.image.bitmap.width != 4 || raster.image.bitmap.height != 2 ||
      raster.image.bitmap.stride != 16 ||
      raster.image.bitmap.pixels.size() != 32) {
    return 11;
  }
  for (std::size_t index = 0; index < raster.image.bitmap.pixels.size();
       index += 4) {
    if (raster.image.bitmap.pixels[index] != 255 ||
        raster.image.bitmap.pixels[index + 1] != 0 ||
        raster.image.bitmap.pixels[index + 2] != 0 ||
        raster.image.bitmap.pixels[index + 3] != 255) {
      return 12;
    }
  }
  return 0;
}

int test_rejects_invalid_and_oversized_assets() {
  const auto empty = cgpui::decode_svg_asset({});
  const auto invalid = cgpui::decode_svg_asset(svg_bytes("<svg><").bytes);
  const auto encoded = cgpui::decode_svg_asset(
      svg_bytes("<svg width=\"1\" height=\"1\"/>").bytes,
      cgpui::SvgAssetDecodeLimits{.max_encoded_bytes = 8});
  const auto intrinsic = cgpui::decode_svg_asset(
      svg_bytes("<svg width=\"4\" height=\"2\"/>").bytes,
      cgpui::SvgAssetDecodeLimits{.max_intrinsic_dimension = 3.0F});
  const cgpui::SvgRasterizationResult absent = cgpui::rasterize_svg_asset(
      cgpui::DecodedSvgAsset{}, cgpui::SvgAssetRasterizationOptions{});
  return empty.status == cgpui::SvgAssetDecodeStatus::empty_input &&
                 invalid.status == cgpui::SvgAssetDecodeStatus::invalid_svg &&
                 encoded.status ==
                     cgpui::SvgAssetDecodeStatus::exceeds_limits &&
                 intrinsic.status ==
                     cgpui::SvgAssetDecodeStatus::exceeds_limits &&
                 absent.status ==
                     cgpui::SvgRasterizationStatus::invalid_request
             ? 0
             : 20;
}

} // namespace

int main() {
  if (const int result = test_decodes_asset_bytes_and_rasterizes()) {
    return result;
  }
  return test_rejects_invalid_and_oversized_assets();
}
