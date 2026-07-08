#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

cgpui::ImageAsset make_registered_raster_asset() {
  std::vector<std::uint8_t> pixels(6U * 4U * 4U, 255U);
  for (std::size_t index = 0; index < pixels.size(); index += 4U) {
    pixels[index] = 52U;
    pixels[index + 1U] = 132U;
    pixels[index + 2U] = 210U;
    pixels[index + 3U] = 255U;
  }

  return cgpui::ImageAsset{
      .id = {},
      .logical_size = {.width = 72.0F, .height = 48.0F},
      .bitmap =
          cgpui::DecodedImageBitmap{
              .width = 6,
              .height = 4,
              .stride = 24,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .pixels = std::move(pixels),
          },
  };
}

class PublicSvgImageSourcesView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicSvgImageSourcesView>& context) override {
    cgpui::ImageAssetRegistry registry(cgpui::ImageAssetId{3630});
    const cgpui::RegisteredImageAsset raster_registration =
        registry.register_image(make_registered_raster_asset());
    const cgpui::RegisteredImageAsset svg_registration = registry.register_svg(
        {.width = 72.0F, .height = 48.0F},
        "<svg viewBox=\"0 0 72 48\"><rect width=\"72\" height=\"48\" rx=\"8\" "
        "fill=\"#223041\"/><path d=\"M10 34L28 14l12 14 8-8 14 14\" "
        "stroke=\"#7fd0ff\" stroke-width=\"5\" fill=\"none\"/></svg>");

    const bool has_expected_sources =
        raster_registration.kind() == cgpui::ImageSourceKind::image &&
        svg_registration.kind() == cgpui::ImageSourceKind::svg &&
        registry.raster_assets().size() == 1U;

    return cgpui::into_element(
        cgpui::v_stack()
            .size(context.viewport_size)
            .p(22.0F)
            .gap(12.0F)
            .bg(cgpui::rgb(18, 24, 32))
            .child(cgpui::label("SVG/image registered sources")
                       .font_size(18.0F)
                       .foreground(cgpui::rgb(241, 246, 250))
                       .build())
            .child(cgpui::label(has_expected_sources
                                    ? "ImageAssetRegistry feeds image/svg"
                                    : "Unexpected registration state")
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(175, 190, 205))
                       .build())
            .child(cgpui::h_stack()
                       .gap(14.0F)
                       .child(cgpui::image(raster_registration.source())
                                  .alt("Registered raster source")
                                  .size(cgpui::Size{96.0F, 64.0F})
                                  .key("registered-raster-source")
                                  .build())
                       .child(cgpui::svg(svg_registration.source())
                                  .alt("Registered SVG source")
                                  .size(cgpui::Size{96.0F, 64.0F})
                                  .key("registered-svg-source")
                                  .build())));
  }
};

using RegistryRef = cgpui::ImageAssetRegistry&;

static_assert(cgpui::Render<PublicSvgImageSourcesView>);
static_assert(std::is_same_v<
              decltype(std::declval<RegistryRef>().register_image(
                  std::declval<cgpui::ImageAsset>())),
              cgpui::RegisteredImageAsset>);
static_assert(std::is_same_v<
              decltype(std::declval<RegistryRef>().register_svg(
                  std::declval<cgpui::Size>(),
                  std::declval<std::string_view>())),
              cgpui::RegisteredImageAsset>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI SVG/Image Sources")
                       .size(440.0F, 260.0F)
                       .decorations(true)
                       .resizable(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_SVG_IMAGE_SOURCES") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicSvgImageSourcesView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}
