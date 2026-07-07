#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

struct ToggleCatalogAction {
  static constexpr std::string_view name = "window_examples.catalog.toggle";
};

struct VolumeCatalogAction {
  static constexpr std::string_view name = "window_examples.catalog.volume";
};

struct OpenCatalogItemAction {
  static constexpr std::string_view name = "window_examples.catalog.open";
};

struct InspectCatalogImageAction {
  static constexpr std::string_view name = "window_examples.catalog.inspect";
};

cgpui::ImageAsset make_catalog_asset() {
  std::vector<std::uint8_t> pixels(4U * 4U * 4U, 255U);
  for (std::size_t index = 0; index < pixels.size(); index += 4U) {
    pixels[index] = 48U;
    pixels[index + 1U] = 132U;
    pixels[index + 2U] = 196U;
    pixels[index + 3U] = 255U;
  }

  return cgpui::ImageAsset{
      .id = cgpui::ImageAssetId{357},
      .logical_size = {.width = 48.0F, .height = 32.0F},
      .bitmap =
          cgpui::DecodedImageBitmap{
              .width = 4,
              .height = 4,
              .stride = 16,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .pixels = std::move(pixels),
          },
  };
}

class PublicWindowExamplesWidgetCatalogView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicWindowExamplesWidgetCatalogView>& context) override {
    const cgpui::ImageAsset asset = make_catalog_asset();
    const cgpui::ImageAssetDescriptor descriptor =
        cgpui::describe_image_asset(asset);

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .bg(cgpui::rgb(19, 24, 30))
            .p(22.0F)
            .child(
                cgpui::v_stack()
                    .size_pct(100.0F, 100.0F)
                    .gap(12.0F)
                    .p(18.0F)
                    .rounded(8.0F)
                    .bg(cgpui::rgb(38, 47, 57))
                    .shadow(catalog_shadow())
                    .child(cgpui::h_stack()
                               .gap(10.0F)
                               .child(cgpui::icon(asset)
                                          .alt("Catalog")
                                          .size(20.0F)
                                          .tint(cgpui::rgb(104, 187, 255))
                                          .build())
                               .child(cgpui::label(
                                          "window/examples widget catalog")
                                          .font_size(19.0F)
                                          .foreground(cgpui::rgb(242, 246, 250))
                                          .build()))
                    .child(cgpui::label(
                               "checkbox/radio/switch, slider, list/menu, image/icon, and container widgets")
                               .font_size(12.0F)
                               .foreground(cgpui::rgb(181, 194, 207))
                               .build())
                    .child(cgpui::h_stack()
                               .gap(10.0F)
                               .child(cgpui::checkbox(
                                          ToggleCatalogAction::name)
                                          .label("Enable catalog")
                                          .checked(true)
                                          .key("catalog-checkbox")
                                          .build())
                               .child(cgpui::radio(ToggleCatalogAction::name)
                                          .label("Preview mode")
                                          .selected(true)
                                          .key("catalog-radio")
                                          .build())
                               .child(cgpui::toggle_switch(
                                          ToggleCatalogAction::name)
                                          .label("Live widgets")
                                          .on(true)
                                          .key("catalog-switch")
                                          .build()))
                    .child(cgpui::h_stack()
                               .gap(10.0F)
                               .child(cgpui::slider(VolumeCatalogAction::name)
                                          .style(slider_style())
                                          .range(0.0F, 100.0F)
                                          .value(64.0F)
                                          .step(4.0F)
                                          .key("catalog-slider")
                                          .build())
                               .child(cgpui::button(
                                          InspectCatalogImageAction::name)
                                          .label("Inspect image")
                                          .build()))
                    .child(cgpui::h_stack()
                               .gap(10.0F)
                               .child(cgpui::list_item(
                                          OpenCatalogItemAction::name)
                                          .label("Window widget example")
                                          .selected(true)
                                          .key("catalog-list-item")
                                          .build())
                               .child(cgpui::menu_item(
                                          OpenCatalogItemAction::name)
                                          .label("Open catalog item")
                                          .key("catalog-menu-item")
                                          .build()))
                    .child(cgpui::h_stack()
                               .gap(10.0F)
                               .child(cgpui::image(descriptor)
                                          .alt("Catalog preview")
                                          .source_rect(cgpui::Rect{
                                              .origin = {.x = 0.0F,
                                                         .y = 0.0F},
                                              .size = {.width = 2.0F,
                                                       .height = 2.0F},
                                          })
                                          .size(cgpui::Size{72.0F, 48.0F})
                                          .key("catalog-image")
                                          .build())
                               .child(cgpui::text_input(input_model_)
                                          .key("catalog-input")
                                          .style(input_style())
                                          .foreground(cgpui::rgb(245, 248, 251))
                                          .font_size(14.0F)
                                          .build()))));
  }

 private:
  static cgpui::BoxShadow catalog_shadow() {
    return cgpui::BoxShadow{
        .color = cgpui::rgba(5, 10, 15, 0.30F),
        .offset = cgpui::Point{0.0F, 8.0F},
        .blur_radius = 22.0F,
        .spread_radius = 1.0F,
    };
  }

  static cgpui::Style slider_style() {
    return cgpui::Style{}.with_preferred_size(
        cgpui::Size{180.0F, 22.0F});
  }

  static cgpui::Style input_style() {
    return cgpui::Style{}
        .with_preferred_size(cgpui::Size{260.0F, 40.0F})
        .with_background_color(cgpui::rgb(49, 60, 70))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(95, 113, 132))
        .with_border_radius(cgpui::BorderRadii::all(6.0F))
        .with_padding(cgpui::edges(12.0F, 8.0F));
  }

  cgpui::TextModel input_model_{"Catalog input example"};
};

using WindowOptionsRef = cgpui::WindowOptions&;

static_assert(cgpui::Action<ToggleCatalogAction>);
static_assert(cgpui::Action<VolumeCatalogAction>);
static_assert(cgpui::Action<OpenCatalogItemAction>);
static_assert(cgpui::Action<InspectCatalogImageAction>);
static_assert(cgpui::Render<PublicWindowExamplesWidgetCatalogView>);
static_assert(
    std::is_same_v<decltype(std::declval<WindowOptionsRef>().resizable(true)),
                   cgpui::WindowOptions&>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Window Examples Widget Catalog")
                       .size(760.0F, 500.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_WINDOW_EXAMPLES_WIDGET_CATALOG") ==
      nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicWindowExamplesWidgetCatalogView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}
