#include <array>
#include <cstddef>
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

struct SourceEvidence {
  const char* path;
  const char* needle;
};

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty()) {
    return 1;
  }

  if (!contains(xmake,
                "target(\"phase_e_svg_integration_closeout_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_e_svg_integration_closeout_test.cpp")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "target(\"svg_rasterization_plan_test\")",
      "target(\"svg_lunasvg_rasterization_test\")",
      "target(\"svg_raster_cache_test\")",
      "target(\"svg_viewport_scaling_test\")",
      "target(\"svg_recolor_tint_test\")",
      "target(\"svg_image_upload_integration_test\")",
      "target(\"phase_e_svg_public_example_test\")",
      "target(\"api_parity_public_svg_raster_upload\")",
  };
  for (std::size_t index = 0; index < target_evidence.size(); ++index) {
    if (!contains(xmake, target_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase E Step 506 SVG integration closeout",
      "tests/api_parity/phase_e_svg_integration_closeout_test.cpp",
      "Steps 499-505",
      "bounded RGBA raster planning",
      "LunaSVG raster backend",
      "cache-owned raster results",
      "viewport-aware scaling",
      "currentColor recolor",
      "RenderFrame image upload",
      "prelude-only public example",
      "Step 507 batching and frame scheduling",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  if (!contains(
          roadmap,
          "- [x] Phase E Step 506 closes the SVG integration band") ||
      contains(roadmap, "- [ ] Step 506:")) {
    return 50;
  }

  constexpr std::array module_evidence{
      SourceEvidence{"include/cgpui/renderer/svg_rasterization.hpp",
                     "struct SvgRasterizationPlan"},
      SourceEvidence{"src/renderer/svg_rasterization_lunasvg.cpp",
                     "lunasvg::Document::loadFromData("},
      SourceEvidence{"include/cgpui/renderer/svg_raster_cache.hpp",
                     "class SvgRasterCache"},
      SourceEvidence{"include/cgpui/renderer/svg_viewport_scaling.hpp",
                     "struct SvgViewportScalingPlan"},
      SourceEvidence{"include/cgpui/renderer/svg_raster_colorization.hpp",
                     "struct SvgRasterColorizationPlan"},
      SourceEvidence{"include/cgpui/renderer/svg_image_upload.hpp",
                     "struct SvgImageUploadResult"},
      SourceEvidence{
          "examples/api_parity/public_svg_raster_upload/main.cpp",
          "cgpui::upload_svg_image("},
  };
  for (std::size_t index = 0; index < module_evidence.size(); ++index) {
    const std::string source = read_source(module_evidence[index].path);
    if (source.empty() || !contains(source, module_evidence[index].needle)) {
      return 60 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      SourceEvidence{"tests/renderer/svg_rasterization_plan_test.cpp",
                     "test_valid_plan_uses_rgba8_device_pixels"},
      SourceEvidence{"tests/renderer/svg_lunasvg_rasterization_test.cpp",
                     "test_lunasvg_rasterizes_plain_rgba_pixels"},
      SourceEvidence{"tests/renderer/svg_raster_cache_test.cpp",
                     "test_identical_request_reuses_cache_owned_result"},
      SourceEvidence{"tests/renderer/svg_viewport_scaling_test.cpp",
                     "test_explicit_viewport_drives_raster_plan"},
      SourceEvidence{"tests/renderer/svg_recolor_tint_test.cpp",
                     "test_current_color_recolors_lunasvg_output"},
      SourceEvidence{"tests/renderer/svg_image_upload_integration_test.cpp",
                     "test_rasterized_svg_is_submitted_to_render_frame"},
      SourceEvidence{"tests/api_parity/phase_e_svg_public_example_test.cpp",
                     "public_svg_raster_upload"},
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    const std::string source = read_source(behavior_evidence[index].path);
    if (source.empty() || !contains(source, behavior_evidence[index].needle)) {
      return 80 + static_cast<int>(index);
    }
  }
  return 0;
}
