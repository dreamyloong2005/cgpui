#include "cgpui/prelude.hpp"

#include <cstdint>
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

cgpui::ImageAsset make_unregistered_asset() {
  return cgpui::ImageAsset{
      .id = {},
      .logical_size = {.width = 16.0F, .height = 8.0F},
      .bitmap =
          cgpui::DecodedImageBitmap{
              .width = 4,
              .height = 2,
              .stride = 16,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .pixels = std::vector<std::uint8_t>(32, 192),
          },
  };
}

int test_registry_registers_raster_sources_for_image_elements() {
  cgpui::ImageAssetRegistry registry(cgpui::ImageAssetId{620});
  const cgpui::RegisteredImageAsset registration =
      registry.register_image(make_unregistered_asset());

  if (registration.id().value != 620 ||
      registration.kind() != cgpui::ImageSourceKind::image ||
      registration.asset().pixel_width != 4 ||
      registration.asset().byte_size != 32 ||
      !registration.svg_source().empty()) {
    return 1;
  }

  const cgpui::RegisteredImageAsset* found =
      registry.find(cgpui::ImageAssetId{620});
  if (found == nullptr ||
      found->asset().id.value != registration.asset().id.value ||
      found->asset().pixel_width != registration.asset().pixel_width ||
      found->asset().byte_size != registration.asset().byte_size ||
      registry.registrations().size() != 1 ||
      registry.raster_assets().size() != 1 ||
      registry.raster_assets()[0].id.value != 620) {
    return 2;
  }

  cgpui::AnyElement element =
      cgpui::image(registration.source()).alt("Registered raster").build();
  auto* image = dynamic_cast<cgpui::ImageElement*>(element.get());
  return image != nullptr && image->asset().id.value == 620 &&
                 image->source().kind() == cgpui::ImageSourceKind::image &&
                 image->accessibility_name() == "Registered raster"
             ? 0
             : 3;
}

int test_registry_registers_svg_sources_without_decoding() {
  constexpr std::string_view svg_markup =
      "<svg viewBox=\"0 0 20 10\"><circle cx=\"5\" cy=\"5\" r=\"4\"/></svg>";
  cgpui::ImageAssetRegistry registry(cgpui::ImageAssetId{700});

  const cgpui::RegisteredImageAsset svg_registration =
      registry.register_svg({.width = 20.0F, .height = 10.0F}, svg_markup);
  const cgpui::RegisteredImageAsset raster_registration =
      registry.register_image(make_unregistered_asset());

  if (svg_registration.id().value != 700 ||
      svg_registration.kind() != cgpui::ImageSourceKind::svg ||
      svg_registration.asset().byte_size != svg_markup.size() ||
      svg_registration.svg_source() != svg_markup ||
      raster_registration.id().value != 701 ||
      registry.raster_assets().size() != 1) {
    return 10;
  }

  cgpui::AnyElement element =
      cgpui::svg(svg_registration.source()).alt("Registered vector").build();
  auto* image = dynamic_cast<cgpui::ImageElement*>(element.get());
  if (image == nullptr ||
      image->source().kind() != cgpui::ImageSourceKind::svg ||
      image->source().svg_source() != svg_markup ||
      image->asset().id.value != 700 ||
      image->accessibility_name() != "Registered vector") {
    return 11;
  }

  return registry.find(cgpui::ImageAssetId{999}) == nullptr ? 0 : 12;
}

int test_step_362_docs_and_structure_are_recorded() {
  const std::string xmake = read_source("xmake.lua");
  const std::string registry_header =
      read_source("include/cgpui/ui/image_asset_registry.hpp");
  const std::string registry_source =
      read_source("src/ui/image_asset_registry.cpp");
  const std::string widget_aggregate =
      read_source("include/cgpui/ui/widget_builders.hpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");

  if (!contains(xmake, "phase_c_svg_image_asset_registration_test") ||
      !contains(registry_header, "class ImageAssetRegistry") ||
      !contains(registry_header, "class RegisteredImageAsset") ||
      !contains(registry_source, "ImageAssetRegistry::register_svg(") ||
      !contains(widget_aggregate,
                "#include \"cgpui/ui/image_asset_registry.hpp\"")) {
    return 20;
  }
  if (!contains(roadmap,
                "Phase C Step 362 SVG/image asset registration") ||
      !contains(ledger_md, "ImageAssetRegistry") ||
      !contains(ledger_json, "\"step_362\"") ||
      !contains(vocabulary, "`ImageAssetRegistry`") ||
      !contains(vocabulary, "`register_svg(...)`")) {
    return 21;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result =
          test_registry_registers_raster_sources_for_image_elements();
      result != 0) {
    return result;
  }
  if (const int result = test_registry_registers_svg_sources_without_decoding();
      result != 0) {
    return result;
  }
  return test_step_362_docs_and_structure_are_recorded();
}
