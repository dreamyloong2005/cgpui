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

  if (!contains(
          xmake,
          "target(\"phase_e_clip_composition_integration_closeout_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/"
          "phase_e_clip_composition_integration_closeout_test.cpp")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "target(\"vulkan_rounded_rect_geometry_test\")",
      "target(\"vulkan_rounded_rect_buffer_test\")",
      "target(\"vulkan_rounded_rect_pipeline_test\")",
      "target(\"vulkan_rounded_rect_draw_recording_test\")",
      "target(\"vulkan_rounded_rect_antialiasing_test\")",
      "target(\"vulkan_rounded_rect_radii_test\")",
      "target(\"vulkan_rounded_rect_stroke_test\")",
      "target(\"vulkan_rounded_rect_fill_variants_test\")",
      "target(\"vulkan_clip_stack_recording_test\")",
      "target(\"vulkan_nested_opacity_test\")",
      "target(\"vulkan_solid_rect_geometry_test\")",
      "target(\"vulkan_composed_transform_test\")",
      "target(\"vulkan_transform_clip_interaction_test\")",
      "target(\"vulkan_stable_draw_order_test\")",
      "target(\"vulkan_layer_ordering_test\")",
  };
  for (std::size_t index = 0; index < target_evidence.size(); ++index) {
    if (!contains(xmake, target_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase E Step 490 clip/composition integration closeout",
      "tests/api_parity/"
      "phase_e_clip_composition_integration_closeout_test.cpp",
      "Steps 475-489",
      "contiguous rounded geometry",
      "allocation-free dynamic scissor",
      "single-application composed opacity",
      "blend-capable solid geometry",
      "push-time framebuffer AABB",
      "stable authored interleaving",
      "explicit z/layer command ordering",
      "non-rectangular clip",
      "Step 491 image texture resources",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  if (!contains(
          roadmap,
          "- [x] Phase E Step 490 closes the clip/composition integration band") ||
      contains(roadmap, "- [ ] Phase E Step 490:")) {
    return 60;
  }

  constexpr std::array module_evidence{
      SourceEvidence{
          "src/renderer/vulkan/vulkan_rounded_rect_geometry_internal.hpp",
          "struct VulkanRoundedRectGeometry"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_rounded_rect_buffers_internal.hpp",
          "struct VulkanRoundedRectBufferResources"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_rounded_rect_pipeline_internal.hpp",
          "struct VulkanRoundedRectPipelineResources"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_rounded_rect_draw_recording.cpp",
          "vkCmdDrawIndexed"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_rounded_rect_antialiasing_internal.hpp",
          "struct VulkanRoundedRectAntialiasingPolicy"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_rounded_rect_radii_internal.hpp",
          "struct VulkanRoundedRectRadiiResolution"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_rounded_rect_stroke_internal.hpp",
          "struct VulkanRoundedRectStrokeResolution"},
      SourceEvidence{
          "src/ui/paint_rounded_rect.cpp",
          "PaintList::stroke_rounded_rect("},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_clip_scissor_internal.hpp",
          "vulkan_resolve_clip_stack_scissor("},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_composition_opacity_internal.hpp",
          "vulkan_apply_composed_opacity("},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_solid_rect_geometry_internal.hpp",
          "vulkan_build_solid_rect_geometry("},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_composition_transform_internal.hpp",
          "vulkan_apply_composed_transform("},
      SourceEvidence{
          "src/ui/paint_clip_transform.hpp",
          "transform_clip_rect_to_framebuffer_aabb("},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_frame_draw_order_internal.hpp",
          "class VulkanFrameDrawOrderCursor"},
      SourceEvidence{
          "include/cgpui/ui/element_core.hpp",
          "explicit_z_index != 0"},
  };
  for (std::size_t index = 0; index < module_evidence.size(); ++index) {
    const std::string source = read_source(module_evidence[index].path);
    if (source.empty() || !contains(source, module_evidence[index].needle)) {
      return 70 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_geometry_test.cpp",
          "test_rounded_rect_builds_contiguous_geometry"},
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_buffer_test.cpp",
          "test_rounded_rect_buffer_resource_contract"},
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_pipeline_test.cpp",
          "test_rounded_rect_vertex_and_fixed_state"},
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_draw_recording_test.cpp",
          "test_uploaded_draw_ranges_are_validated"},
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_antialiasing_test.cpp",
          "test_geometry_builds_inner_and_outer_coverage_rings"},
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_radii_test.cpp",
          "test_oversized_radii_share_one_normalization_scale"},
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_stroke_test.cpp",
          "test_stroke_geometry_builds_fill_border_and_fringe_rings"},
      SourceEvidence{
          "tests/renderer/vulkan_rounded_rect_fill_variants_test.cpp",
          "test_vulkan_stroke_only_geometry_skips_fill_vertices"},
      SourceEvidence{
          "tests/renderer/vulkan_clip_stack_recording_test.cpp",
          "test_full_frame_and_nested_clip_scissors"},
      SourceEvidence{
          "tests/renderer/vulkan_nested_opacity_test.cpp",
          "test_precomposed_opacity_is_applied_once"},
      SourceEvidence{
          "tests/renderer/vulkan_solid_rect_geometry_test.cpp",
          "test_solid_rects_build_compact_blended_geometry"},
      SourceEvidence{
          "tests/renderer/vulkan_composed_transform_test.cpp",
          "test_solid_and_rounded_vertices_transform"},
      SourceEvidence{
          "tests/renderer/vulkan_transform_clip_interaction_test.cpp",
          "test_push_clip_captures_current_precomposed_transform"},
      SourceEvidence{
          "tests/renderer/vulkan_stable_draw_order_test.cpp",
          "test_compact_cursor_preserves_authored_interleaving"},
      SourceEvidence{
          "tests/renderer/vulkan_layer_ordering_test.cpp",
          "test_ui_z_order_reaches_vulkan_cursor"},
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    const std::string source = read_source(behavior_evidence[index].path);
    if (source.empty() || !contains(source, behavior_evidence[index].needle)) {
      return 100 + static_cast<int>(index);
    }
  }

  return 0;
}
