#include "cgpui/prelude.hpp"
#include "cgpui/ui/image_source.hpp"

#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(path);
  if (!source) {
    source.open((std::string("../../../../") + path).c_str());
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

cgpui::ImageAsset make_front_end_asset() {
  return cgpui::ImageAsset{
      .id = cgpui::ImageAssetId{361},
      .logical_size = {.width = 18.0F, .height = 10.0F},
      .bitmap =
          cgpui::DecodedImageBitmap{
              .width = 6,
              .height = 4,
              .stride = 24,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .pixels = std::vector<std::uint8_t>(96, 255),
          },
  };
}

int test_svg_source_builds_svg_image_element() {
  constexpr std::string_view svg_markup =
      "<svg viewBox=\"0 0 24 12\"><path d=\"M0 0h24v12H0z\"/></svg>";
  cgpui::ImageSource source = cgpui::svg_image_source(
      cgpui::ImageAssetId{362},
      {.width = 24.0F, .height = 12.0F},
      svg_markup);

  if (source.kind() != cgpui::ImageSourceKind::svg ||
      source.asset().id.value != 362 ||
      source.asset().logical_size.width != 24.0F ||
      source.asset().logical_size.height != 12.0F ||
      source.asset().byte_size != svg_markup.size() ||
      source.svg_source() != svg_markup) {
    return 1;
  }

  cgpui::AnyElement element =
      cgpui::svg(source)
          .alt("Vector badge")
          .style(cgpui::Style{}.with_preferred_size({.width = 48.0F,
                                                     .height = 24.0F}))
          .build();
  auto* image = dynamic_cast<cgpui::ImageElement*>(element.get());
  if (image == nullptr ||
      image->kind() != cgpui::ImageElementKind::svg ||
      image->source().kind() != cgpui::ImageSourceKind::svg ||
      image->asset().id.value != 362 ||
      image->accessibility_name() != "Vector badge") {
    return 2;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 48.0F || output.size.height != 24.0F) {
    return 3;
  }

  cgpui::PaintList paint_list;
  image->paint(paint_list);
  if (paint_list.commands().size() != 1 ||
      paint_list.commands()[0].kind != cgpui::PaintCommandKind::image) {
    return 4;
  }
  const cgpui::ImagePaint& command = paint_list.commands()[0].image;
  return command.asset.id.value == 362 &&
                 command.asset.logical_size.width == 24.0F &&
                 command.bounds.size.width == 48.0F
             ? 0
             : 5;
}

int test_image_source_preserves_raster_asset_front_end() {
  const cgpui::ImageAsset asset = make_front_end_asset();
  cgpui::ImageSource source = cgpui::image_source(asset);
  if (source.kind() != cgpui::ImageSourceKind::image ||
      source.asset().id.value != 361 ||
      source.asset().pixel_width != 6 ||
      source.asset().byte_size != asset.bitmap.pixels.size() ||
      !source.svg_source().empty()) {
    return 10;
  }

  cgpui::AnyElement element =
      cgpui::image(source).alt("Raster preview").build();
  auto* image = dynamic_cast<cgpui::ImageElement*>(element.get());
  return image != nullptr &&
                 image->kind() == cgpui::ImageElementKind::image &&
                 image->source().kind() == cgpui::ImageSourceKind::image &&
                 image->asset().id.value == 361 &&
                 image->accessibility_name() == "Raster preview"
             ? 0
             : 11;
}

int test_step_361_docs_and_structure_are_recorded() {
  const std::string xmake = read_source("xmake.lua");
  const std::string image_source_header =
      read_source("include/cgpui/ui/image_source.hpp");
  const std::string image_source_source =
      read_source("src/ui/image_source.cpp");
  const std::string image_builder_header =
      read_source("include/cgpui/ui/image_builder.hpp");
  const std::string widget_source =
      read_source("src/ui/widgets/image_builder.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");

  if (!contains(xmake, "phase_c_svg_image_front_end_test") ||
      !contains(image_source_header, "enum class ImageSourceKind") ||
      !contains(image_source_header, "class ImageSource") ||
      !contains(image_source_source, "svg_image_source(") ||
      !contains(image_builder_header, "svg(ImageSource source)") ||
      !contains(widget_source, "ImageElementKind::svg")) {
    return 20;
  }
  if (!contains(roadmap,
                "Phase C Step 361 SVG/image front-end source APIs") ||
      !contains(ledger_md, "ImageSource") ||
      !contains(ledger_json, "\"step_361\"") ||
      !contains(vocabulary, "`svg(...)`") ||
      !contains(vocabulary, "`svg_image_source(...)`") ||
      !contains(roadmap, "Phase C Step 362 SVG/image asset registration")) {
    return 21;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_svg_source_builds_svg_image_element();
      result != 0) {
    return result;
  }
  if (const int result = test_image_source_preserves_raster_asset_front_end();
      result != 0) {
    return result;
  }
  return test_step_361_docs_and_structure_are_recorded();
}
