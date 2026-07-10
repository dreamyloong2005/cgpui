#include "cgpui/prelude.hpp"

#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

class PublicSvgUploadFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  void upload_image(const cgpui::ImageAsset& image) override {
    uploads.push_back(image);
  }
  void draw_image(const cgpui::ImageDraw& image) override {
    draws.push_back(image);
  }
  cgpui::Result<void> present() override { return {}; }

  std::vector<cgpui::ImageAsset> uploads;
  std::vector<cgpui::ImageDraw> draws;
};

using UploadResult = decltype(cgpui::upload_svg_image(
    std::declval<cgpui::RenderFrame&>(),
    std::declval<cgpui::SvgRasterCache&>(),
    std::declval<const cgpui::SvgRasterizationRequest&>()));

static_assert(std::is_same_v<UploadResult, cgpui::SvgImageUploadResult>);

} // namespace

int main() {
  constexpr std::string_view svg_source =
      "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 48 32\">"
      "<rect width=\"48\" height=\"32\" rx=\"6\" fill=\"currentColor\"/>"
      "<path d=\"M8 22L18 10l8 8 6-6 8 10\" stroke=\"white\" "
      "stroke-width=\"3\" fill=\"none\"/></svg>";

  cgpui::ImageAssetRegistry registry(cgpui::ImageAssetId{5050});
  const cgpui::RegisteredImageAsset registration =
      registry.register_svg({.width = 48.0F, .height = 32.0F}, svg_source);
  const cgpui::SvgRasterizationRequest request{
      .asset_id = registration.id(),
      .logical_size = registration.asset().logical_size,
      .svg_source = registration.svg_source(),
      .scale = cgpui::DpiScale{1.5F},
      .viewport_size = {.width = 96.0F, .height = 64.0F},
      .current_color = cgpui::rgb(47, 132, 201),
  };

  cgpui::SvgRasterCache cache;
  PublicSvgUploadFrame frame;
  const cgpui::SvgImageUploadResult first =
      cgpui::upload_svg_image(frame, cache, request);
  const cgpui::SvgImageUploadResult repeated =
      cgpui::upload_svg_image(frame, cache, request);
  if (!repeated.ready()) {
    return 1;
  }

  frame.draw_image(cgpui::ImageDraw{
      .bounds = {
          .origin = {.x = 12.0F, .y = 16.0F},
          .size = request.viewport_size,
      },
      .asset = repeated.image,
      .sampling = cgpui::ImageSamplingMode::linear,
  });

  return first.ready() && !first.cache_hit && repeated.cache_hit &&
                 frame.uploads.size() == 2 && frame.draws.size() == 1 &&
                 repeated.image.pixel_width == 144 &&
                 repeated.image.pixel_height == 96 &&
                 cache.entry_count() == 1 && cache.hit_count() == 1
             ? 0
             : 2;
}
