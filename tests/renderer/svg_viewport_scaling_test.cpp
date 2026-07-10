#include "cgpui/renderer/svg_raster_cache.hpp"
#include "cgpui/renderer/svg_viewport_scaling.hpp"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>
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

bool near(float lhs, float rhs) {
  return std::abs(lhs - rhs) < 0.0001F;
}

constexpr std::string_view source =
    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"4\" height=\"2\" "
    "viewBox=\"0 0 4 2\" preserveAspectRatio=\"none\">"
    "<rect width=\"4\" height=\"2\" fill=\"#ff0000\"/></svg>";

cgpui::SvgRasterizationRequest request_for(cgpui::Size viewport = {}) {
  return cgpui::SvgRasterizationRequest{
      .asset_id = cgpui::ImageAssetId{71},
      .logical_size = {.width = 4.0F, .height = 2.0F},
      .svg_source = source,
      .scale = cgpui::DpiScale{2.0F},
      .viewport_size = viewport,
  };
}

int test_empty_viewport_uses_intrinsic_size() {
  const cgpui::SvgViewportScalingPlan plan =
      cgpui::plan_svg_viewport_scaling(cgpui::SvgViewportScalingRequest{
          .logical_size = {.width = 4.0F, .height = 2.0F},
          .scale = cgpui::DpiScale{1.5F},
      });
  return plan.ready() && plan.logical_size.width == 4.0F &&
                 plan.logical_size.height == 2.0F &&
                 plan.viewport_size.width == 4.0F &&
                 plan.viewport_size.height == 2.0F && plan.pixel_width == 6 &&
                 plan.pixel_height == 3 && near(plan.effective_scale_x, 1.5F) &&
                 near(plan.effective_scale_y, 1.5F)
             ? 0
             : 10;
}

int test_explicit_viewport_drives_raster_plan() {
  const cgpui::SvgRasterizationPlan plan = cgpui::plan_svg_rasterization(
      cgpui::SvgRasterizationRequest{
          .asset_id = cgpui::ImageAssetId{72},
          .logical_size = {.width = 4.0F, .height = 2.0F},
          .svg_source = source,
          .scale = cgpui::DpiScale{1.5F},
          .viewport_size = {.width = 5.0F, .height = 2.5F},
      });
  return plan.ready() && plan.viewport.ready() &&
                 plan.viewport.logical_size.width == 4.0F &&
                 plan.viewport.viewport_size.width == 5.0F &&
                 plan.viewport.viewport_size.height == 2.5F &&
                 plan.viewport.pixel_width == 8 &&
                 plan.viewport.pixel_height == 4 &&
                 near(plan.viewport.effective_scale_x, 1.6F) &&
                 near(plan.viewport.effective_scale_y, 1.6F) &&
                 plan.descriptor.logical_size.width == 5.0F &&
                 plan.descriptor.logical_size.height == 2.5F &&
                 plan.descriptor.pixel_width == 8 &&
                 plan.descriptor.pixel_height == 4
             ? 0
             : 20;
}

int test_invalid_and_oversized_viewports_report_status() {
  const cgpui::SvgViewportScalingPlan partial =
      cgpui::plan_svg_viewport_scaling(cgpui::SvgViewportScalingRequest{
          .logical_size = {.width = 4.0F, .height = 2.0F},
          .viewport_size = {.width = 8.0F},
      });
  const cgpui::SvgViewportScalingPlan oversized =
      cgpui::plan_svg_viewport_scaling(cgpui::SvgViewportScalingRequest{
          .logical_size = {.width = 4.0F, .height = 2.0F},
          .viewport_size = {.width = 20000.0F, .height = 20000.0F},
      });
  return partial.status ==
                     cgpui::SvgViewportScalingStatus::invalid_viewport_size &&
                 oversized.status ==
                     cgpui::SvgViewportScalingStatus::exceeds_limits
             ? 0
             : 30;
}

int test_lunasvg_renders_explicit_viewport_pixels() {
  cgpui::SvgRasterizationRequest request = request_for(
      {.width = 6.0F, .height = 3.0F});
  const cgpui::SvgRasterizationResult result = cgpui::rasterize_svg(request);
  if (!result.ready() || result.image.logical_size.width != 6.0F ||
      result.image.logical_size.height != 3.0F ||
      result.image.bitmap.width != 12 || result.image.bitmap.height != 6 ||
      result.image.bitmap.pixels.size() != 288) {
    return 40;
  }
  for (std::size_t index = 0; index < result.image.bitmap.pixels.size();
       index += 4) {
    if (result.image.bitmap.pixels[index] != 255 ||
        result.image.bitmap.pixels[index + 1] != 0 ||
        result.image.bitmap.pixels[index + 2] != 0 ||
        result.image.bitmap.pixels[index + 3] != 255) {
      return 41;
    }
  }
  return 0;
}

int test_cache_normalizes_default_viewport() {
  cgpui::SvgRasterCache cache;
  const cgpui::SvgRasterCacheLookup first = cache.rasterize(request_for());
  const cgpui::SvgRasterCacheLookup equivalent =
      cache.rasterize(request_for({.width = 4.0F, .height = 2.0F}));
  const cgpui::SvgRasterCacheLookup larger =
      cache.rasterize(request_for({.width = 8.0F, .height = 4.0F}));
  return first.ready() && equivalent.ready() && larger.ready() &&
                 !first.cache_hit && equivalent.cache_hit && !larger.cache_hit &&
                 equivalent.result == first.result && cache.entry_count() == 2 &&
                 cache.lookup_count() == 3 && cache.hit_count() == 1 &&
                 cache.miss_count() == 2
             ? 0
             : 50;
}

int test_structure_and_documentation() {
  const std::string header =
      read_source("include/cgpui/renderer/svg_viewport_scaling.hpp");
  const std::string source_file =
      read_source("src/renderer/svg_viewport_scaling.cpp");
  const std::string rasterization =
      read_source("include/cgpui/renderer/svg_rasterization.hpp");
  const std::string rasterization_source =
      read_source("src/renderer/svg_rasterization.cpp");
  const std::string cache_source =
      read_source("src/renderer/svg_raster_cache.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  if (header.empty() || source_file.empty() || rasterization.empty() ||
      rasterization_source.empty() || cache_source.empty() || structure.empty() ||
      xmake.empty()) {
    return 60;
  }
  if (!contains(header, "struct SvgViewportScalingPlan") ||
      !contains(source_file, "plan_svg_viewport_scaling(") ||
      !contains(rasterization, "Size viewport_size") ||
      !contains(rasterization_source, "plan_svg_viewport_scaling(") ||
      !contains(cache_source, "viewport_size") ||
      !contains(structure, "src/renderer/svg_viewport_scaling.cpp") ||
      !contains(xmake, "target(\"svg_viewport_scaling_test\")")) {
    return 61;
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
      "Phase E Step 502",
      "SvgViewportScalingPlan",
      "optional raster viewport",
      "ceil-rounded viewport device pixels",
      "effective x/y raster scales",
      "Step 503 SVG recolor/tint",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 70;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_empty_viewport_uses_intrinsic_size(); result != 0) {
    return result;
  }
  if (const int result = test_explicit_viewport_drives_raster_plan();
      result != 0) {
    return result;
  }
  if (const int result = test_invalid_and_oversized_viewports_report_status();
      result != 0) {
    return result;
  }
  if (const int result = test_lunasvg_renders_explicit_viewport_pixels();
      result != 0) {
    return result;
  }
  if (const int result = test_cache_normalizes_default_viewport(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
