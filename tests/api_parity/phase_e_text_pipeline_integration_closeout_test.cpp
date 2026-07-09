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
  const std::string pipeline = read_source(
      "src/renderer/vulkan/vulkan_text_pipeline_internal.hpp");
  const std::string resources = read_source(
      "src/renderer/vulkan/vulkan_text_pipeline_resources_internal.hpp");
  const std::string shaders =
      read_source("src/renderer/vulkan/vulkan_text_shader_binaries.cpp");
  const std::string vertex_buffer = read_source(
      "src/renderer/vulkan/vulkan_text_vertex_buffer_internal.hpp");
  const std::string draw =
      read_source("src/renderer/vulkan/vulkan_text_draw_recording.cpp");
  const std::string positioning = read_source(
      "src/renderer/vulkan/vulkan_text_positioning_internal.hpp");
  const std::string coverage = read_source(
      "src/renderer/vulkan/vulkan_text_coverage_internal.hpp");
  const std::string fragment =
      read_source("src/renderer/vulkan/shaders/text.frag.glsl");
  const std::string frame_test =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty() ||
      pipeline.empty() || resources.empty() || shaders.empty() ||
      vertex_buffer.empty() || draw.empty() || positioning.empty() ||
      coverage.empty() || fragment.empty() || frame_test.empty()) {
    return 1;
  }

  if (!contains(
          xmake,
          "target(\"phase_e_text_pipeline_integration_closeout_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/"
          "phase_e_text_pipeline_integration_closeout_test.cpp")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "target(\"vulkan_text_pipeline_state_test\")",
      "target(\"vulkan_text_shader_module_test\")",
      "target(\"vulkan_text_pipeline_resources_test\")",
      "target(\"vulkan_text_vertex_buffer_test\")",
      "target(\"vulkan_text_draw_recording_test\")",
      "target(\"vulkan_text_positioning_test\")",
      "target(\"vulkan_text_coverage_test\")",
  };
  for (std::size_t index = 0; index < target_evidence.size(); ++index) {
    if (!contains(xmake, target_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase E Step 474 text pipeline integration closeout",
      "tests/api_parity/phase_e_text_pipeline_integration_closeout_test.cpp",
      "Steps 467-473",
      "descriptor-bound textured glyph draws",
      "preserve_subpixel",
      "straight_color_coverage_alpha",
      "Step 475 rounded rectangle geometry",
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
          "- [x] Phase E Step 474 closes text pipeline integration") ||
      contains(roadmap, "- [ ] Step 474:")) {
    return 50;
  }

  const char* module_evidence[] = {
      "struct VulkanTextVertex",
      "struct VulkanTextPipelineResources",
      "vulkan_text_fragment_shader_spirv(",
      "struct VulkanTextVertexBufferResources",
      "vkCmdDraw",
      "enum class VulkanTextPositioningPolicy",
      "struct VulkanTextCoveragePolicy",
      "coverage_gamma",
  };
  const std::string* module_sources[] = {
      &pipeline,
      &resources,
      &shaders,
      &vertex_buffer,
      &draw,
      &positioning,
      &coverage,
      &fragment,
  };
  for (std::size_t index = 0; index < std::size(module_evidence); ++index) {
    if (!contains(*module_sources[index], module_evidence[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  if (!contains(frame_test, "test_multi_page_glyph_atlas_frame") ||
      !contains(frame_test, "test_incremental_glyph_atlas_frame")) {
    return 80;
  }

  return 0;
}
