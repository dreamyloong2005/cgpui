#include "cgpui/renderer/svg_raster_cache.hpp"
#include "cgpui/renderer/svg_raster_colorization.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
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

constexpr std::string_view current_color_svg =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"2\" height=\"2\">"
    "<rect width=\"2\" height=\"2\" fill=\"currentColor\"/></svg>";

cgpui::SvgRasterizationRequest request_for(
    std::optional<cgpui::Color> current_color = std::nullopt) {
  return cgpui::SvgRasterizationRequest{
      .asset_id = cgpui::ImageAssetId{81},
      .logical_size = {.width = 2.0F, .height = 2.0F},
      .svg_source = current_color_svg,
      .scale = cgpui::DpiScale{1.0F},
      .current_color = current_color,
  };
}

bool all_pixels_are(
    const cgpui::SvgRasterizationResult& result,
    std::uint8_t red,
    std::uint8_t green,
    std::uint8_t blue,
    std::uint8_t alpha) {
  for (std::size_t index = 0; index < result.image.bitmap.pixels.size();
       index += 4) {
    if (result.image.bitmap.pixels[index] != red ||
        result.image.bitmap.pixels[index + 1] != green ||
        result.image.bitmap.pixels[index + 2] != blue ||
        result.image.bitmap.pixels[index + 3] != alpha) {
      return false;
    }
  }
  return true;
}

int test_current_color_recolors_lunasvg_output() {
  const cgpui::SvgRasterizationResult result = cgpui::rasterize_svg(
      request_for(cgpui::Color{.r = 0.0F, .g = 0.5F, .b = 1.0F, .a = 1.0F}));
  return result.ready() && result.plan.colorization.ready() &&
                 result.plan.colorization.recolors_current_color &&
                 result.image.bitmap.width == 2 &&
                 result.image.bitmap.height == 2 &&
                 all_pixels_are(result, 0, 128, 255, 255)
             ? 0
             : 10;
}

int test_invalid_current_color_is_rejected() {
  const cgpui::Color invalid{
      .r = std::numeric_limits<float>::quiet_NaN(),
      .g = 0.0F,
      .b = 0.0F,
      .a = 1.0F,
  };
  const cgpui::SvgRasterColorizationPlan colorization =
      cgpui::plan_svg_raster_colorization(
          cgpui::SvgRasterColorizationRequest{.current_color = invalid});
  const cgpui::SvgRasterizationPlan plan =
      cgpui::plan_svg_rasterization(request_for(invalid));
  const cgpui::SvgRasterizationResult result =
      cgpui::rasterize_svg(request_for(invalid));
  return colorization.status ==
                     cgpui::SvgRasterColorizationStatus::invalid_color &&
                 plan.status ==
                     cgpui::SvgRasterizationPlanStatus::invalid_color &&
                 result.status ==
                     cgpui::SvgRasterizationStatus::invalid_request
             ? 0
             : 20;
}

int test_cache_identity_includes_current_color() {
  cgpui::SvgRasterCache cache;
  const cgpui::Color blue{.r = 0.0F, .g = 0.0F, .b = 1.0F, .a = 1.0F};
  const cgpui::Color red{.r = 1.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F};
  const cgpui::SvgRasterCacheLookup first =
      cache.rasterize(request_for(blue));
  const cgpui::SvgRasterCacheLookup repeated =
      cache.rasterize(request_for(blue));
  const cgpui::SvgRasterCacheLookup recolored =
      cache.rasterize(request_for(red));
  return first.ready() && repeated.ready() && recolored.ready() &&
                 !first.cache_hit && repeated.cache_hit && !recolored.cache_hit &&
                 repeated.result == first.result && cache.entry_count() == 2 &&
                 cache.lookup_count() == 3 && cache.hit_count() == 1 &&
                 cache.miss_count() == 2 &&
                 all_pixels_are(*first.result, 0, 0, 255, 255) &&
                 all_pixels_are(*recolored.result, 255, 0, 0, 255)
             ? 0
             : 30;
}

int test_tint_remains_draw_time() {
  const std::string color_source =
      read_source("src/renderer/vulkan/vulkan_image_color.cpp");
  const std::string fragment_shader =
      read_source("src/renderer/vulkan/shaders/image.frag.glsl");
  const std::string cache_header =
      read_source("include/cgpui/renderer/svg_raster_cache.hpp");
  return contains(color_source, "vulkan_resolve_image_tint(") &&
                 contains(fragment_shader,
                          "texture(image_texture, in_image_uv) * in_color") &&
                 !contains(cache_header, "tint")
             ? 0
             : 40;
}

int test_structure_and_documentation() {
  const std::string header =
      read_source("include/cgpui/renderer/svg_raster_colorization.hpp");
  const std::string source =
      read_source("src/renderer/svg_raster_colorization.cpp");
  const std::string internal = read_source(
      "src/renderer/svg_raster_colorization_internal.hpp");
  const std::string backend =
      read_source("src/renderer/svg_rasterization_lunasvg.cpp");
  const std::string rasterization =
      read_source("include/cgpui/renderer/svg_rasterization.hpp");
  const std::string cache_source =
      read_source("src/renderer/svg_raster_cache.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  if (header.empty() || source.empty() || internal.empty() || backend.empty() ||
      rasterization.empty() || cache_source.empty() || structure.empty() ||
      xmake.empty()) {
    return 50;
  }
  if (!contains(header, "struct SvgRasterColorizationPlan") ||
      !contains(source, "plan_svg_raster_colorization(") ||
      !contains(internal, "svg_current_color_css(") ||
      !contains(backend, "documentElement().setAttribute(") ||
      !contains(rasterization, "current_color") ||
      !contains(cache_source, "colorization") ||
      !contains(structure, "src/renderer/svg_raster_colorization.cpp") ||
      !contains(xmake, "target(\"svg_recolor_tint_test\")")) {
    return 51;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 503",
      "SvgRasterColorizationPlan",
      "LunaSVG currentColor recolor",
      "validated RGBA current color",
      "draw-time multiplicative tint",
      "Step 504 SVG image upload integration",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 60;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_current_color_recolors_lunasvg_output();
      result != 0) {
    return result;
  }
  if (const int result = test_invalid_current_color_is_rejected(); result != 0) {
    return result;
  }
  if (const int result = test_cache_identity_includes_current_color();
      result != 0) {
    return result;
  }
  if (const int result = test_tint_remains_draw_time(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
