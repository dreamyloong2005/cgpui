#include "cgpui/renderer/svg_rasterization.hpp"

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

int test_valid_plan_uses_rgba8_device_pixels() {
  const cgpui::SvgRasterizationPlan plan = cgpui::plan_svg_rasterization(
      cgpui::SvgRasterizationRequest{
          .asset_id = cgpui::ImageAssetId{42},
          .logical_size = {.width = 12.5F, .height = 8.25F},
          .svg_source = "<svg viewBox=\"0 0 10 10\"/>",
          .scale = cgpui::DpiScale{2.0F},
      });
  return plan.ready() &&
                 plan.strategy ==
                     cgpui::SvgRasterizationStrategy::rgba8_bitmap &&
                 plan.descriptor.id.value == 42 &&
                 plan.descriptor.logical_size.width == 12.5F &&
                 plan.descriptor.logical_size.height == 8.25F &&
                 plan.descriptor.pixel_width == 25 &&
                 plan.descriptor.pixel_height == 17 &&
                 plan.descriptor.stride == 100 &&
                 plan.descriptor.byte_size == 1700
             ? 0
             : 10;
}

int test_invalid_requests_report_explicit_status() {
  const cgpui::SvgRasterizationPlan missing_id =
      cgpui::plan_svg_rasterization(cgpui::SvgRasterizationRequest{
          .logical_size = {.width = 16.0F, .height = 16.0F},
          .svg_source = "<svg/>",
      });
  const cgpui::SvgRasterizationPlan empty_source =
      cgpui::plan_svg_rasterization(cgpui::SvgRasterizationRequest{
          .asset_id = cgpui::ImageAssetId{1},
          .logical_size = {.width = 16.0F, .height = 16.0F},
      });
  const cgpui::SvgRasterizationPlan invalid_size =
      cgpui::plan_svg_rasterization(cgpui::SvgRasterizationRequest{
          .asset_id = cgpui::ImageAssetId{1},
          .logical_size = {.width = -1.0F, .height = 16.0F},
          .svg_source = "<svg/>",
      });
  const cgpui::SvgRasterizationPlan invalid_scale =
      cgpui::plan_svg_rasterization(cgpui::SvgRasterizationRequest{
          .asset_id = cgpui::ImageAssetId{1},
          .logical_size = {.width = 16.0F, .height = 16.0F},
          .svg_source = "<svg/>",
          .scale = cgpui::DpiScale{0.0F},
      });
  return missing_id.status ==
                     cgpui::SvgRasterizationPlanStatus::invalid_asset_id &&
                 empty_source.status ==
                     cgpui::SvgRasterizationPlanStatus::empty_source &&
                 invalid_size.status ==
                     cgpui::SvgRasterizationPlanStatus::invalid_logical_size &&
                 invalid_scale.status ==
                     cgpui::SvgRasterizationPlanStatus::invalid_scale
             ? 0
             : 20;
}

int test_budget_limits_are_bounded() {
  const cgpui::SvgRasterizationPlan too_large =
      cgpui::plan_svg_rasterization(cgpui::SvgRasterizationRequest{
          .asset_id = cgpui::ImageAssetId{7},
          .logical_size = {.width = 20000.0F, .height = 20000.0F},
          .svg_source = "<svg/>",
      });
  return too_large.status ==
                     cgpui::SvgRasterizationPlanStatus::exceeds_limits &&
                 cgpui::svg_rasterization_max_dimension == 16384 &&
                 cgpui::svg_rasterization_max_bytes == 512U * 1024U * 1024U
             ? 0
             : 30;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "include/cgpui/renderer/svg_rasterization.hpp");
  const std::string source =
      read_source("src/renderer/svg_rasterization.cpp");
  const std::string aggregate =
      read_source("include/cgpui/renderer/renderer.hpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  if (header.empty() || source.empty() || aggregate.empty() ||
      structure.empty() || xmake.empty()) {
    return 40;
  }
  if (!contains(header, "struct SvgRasterizationRequest") ||
      !contains(header, "struct SvgRasterizationPlan") ||
      !contains(source, "plan_svg_rasterization(") ||
      !contains(aggregate, "cgpui/renderer/svg_rasterization.hpp") ||
      contains(aggregate, "plan_svg_rasterization(") ||
      !contains(structure, "src/renderer/svg_rasterization.cpp") ||
      !contains(xmake, "target(\"svg_rasterization_plan_test\")")) {
    return 41;
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
      "Phase E Step 499",
      "SvgRasterizationRequest",
      "SvgRasterizationPlan",
      "explicit RGBA8 output plan",
      "bounded raster byte budget",
      "Step 500 LunaSVG raster backend",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 50;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_valid_plan_uses_rgba8_device_pixels();
      result != 0) {
    return result;
  }
  if (const int result = test_invalid_requests_report_explicit_status();
      result != 0) {
    return result;
  }
  if (const int result = test_budget_limits_are_bounded(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
