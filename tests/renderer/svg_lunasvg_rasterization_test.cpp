#include "cgpui/renderer/svg_rasterization.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = CGPUI_SOURCE_ROOT; root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
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

int test_lunasvg_rasterizes_plain_rgba_pixels() {
  constexpr std::string_view source =
      "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"4\" height=\"2\" "
      "viewBox=\"0 0 4 2\"><rect width=\"4\" height=\"2\" "
      "fill=\"#ff0000\"/></svg>";
  const cgpui::SvgRasterizationResult result = cgpui::rasterize_svg(
      cgpui::SvgRasterizationRequest{
          .asset_id = cgpui::ImageAssetId{51},
          .logical_size = {.width = 4.0F, .height = 2.0F},
          .svg_source = source,
          .scale = cgpui::DpiScale{2.0F},
      });
  if (!result.ready() || result.image.id.value != 51 ||
      result.image.bitmap.width != 8 || result.image.bitmap.height != 4 ||
      result.image.bitmap.stride != 32 ||
      result.image.bitmap.format != cgpui::ImageFormat::rgba8_unorm ||
      result.image.bitmap.pixels.size() != 128) {
    return 10;
  }
  for (std::size_t index = 0; index < result.image.bitmap.pixels.size();
       index += 4) {
    if (result.image.bitmap.pixels[index] != 255 ||
        result.image.bitmap.pixels[index + 1] != 0 ||
        result.image.bitmap.pixels[index + 2] != 0 ||
        result.image.bitmap.pixels[index + 3] != 255) {
      return 11;
    }
  }
  return 0;
}

int test_invalid_svg_reports_parse_failure() {
  const cgpui::SvgRasterizationResult invalid_plan = cgpui::rasterize_svg(
      cgpui::SvgRasterizationRequest{});
  const cgpui::SvgRasterizationResult invalid_svg = cgpui::rasterize_svg(
      cgpui::SvgRasterizationRequest{
          .asset_id = cgpui::ImageAssetId{52},
          .logical_size = {.width = 4.0F, .height = 4.0F},
          .svg_source = "<svg><",
      });
  return invalid_plan.status ==
                     cgpui::SvgRasterizationStatus::invalid_request &&
                 invalid_svg.status ==
                     cgpui::SvgRasterizationStatus::invalid_svg
             ? 0
             : 20;
}

int test_structure() {
  const std::string header = read_source(
      "include/cgpui/renderer/svg_rasterization.hpp");
  const std::string backend =
      read_source("src/renderer/svg_rasterization_lunasvg.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string manifests = read_source("packages/renderer/mcpp.toml") +
      read_source("tests/renderer/mcpp.toml");
  if (header.empty() || backend.empty() || structure.empty() || manifests.empty()) {
    return 30;
  }
  if (!contains(header, "struct SvgRasterizationResult") ||
      !contains(header, "rasterize_svg(") ||
      !contains(backend, "lunasvg::Document::loadFromData(") ||
      !contains(backend, "convertToRGBA()") ||
      !contains(structure, "src/renderer/svg_rasterization_lunasvg.cpp") ||
      !contains(manifests, "vendor.lunasvg = { path = \"../../third_party/lunasvg\" }") ||
      !contains(manifests, "\"svg_lunasvg_rasterization_test.cpp\"")) {
    return 31;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_lunasvg_rasterizes_plain_rgba_pixels();
      result != 0) {
    return result;
  }
  if (const int result = test_invalid_svg_reports_parse_failure(); result != 0) {
    return result;
  }
  return test_structure();
}
