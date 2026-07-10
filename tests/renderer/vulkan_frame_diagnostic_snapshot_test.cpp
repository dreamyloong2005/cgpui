#include "cgpui/renderer/renderer.hpp"
#include "vulkan_frame_diagnostic_snapshot_internal.hpp"

#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

class DefaultDiagnosticRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unique_ptr<cgpui::RenderFrame>{};
  }
};

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
  return source ? std::string{
                      std::istreambuf_iterator<char>(source),
                      std::istreambuf_iterator<char>()}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::vector<cgpui::RendererCommandBatch> command_batches() {
  using Kind = cgpui::RendererPrimitiveKind;
  return {
      {.key = {.primitive_kind = Kind::solid_rect}, .command_indices = {0}},
      {.key = {.primitive_kind = Kind::rounded_rect}, .command_indices = {0}},
      {.key = {.primitive_kind = Kind::text}, .command_indices = {0}},
      {.key = {.primitive_kind = Kind::text_selection}, .command_indices = {0}},
      {.key = {.primitive_kind = Kind::text_caret}, .command_indices = {0}},
      {.key = {.primitive_kind = Kind::image}, .command_indices = {0}},
  };
}

int test_builds_live_resource_accounting() {
  using Kind = cgpui::RendererPrimitiveKind;
  const std::array draw_order{
      cgpui::VulkanFrameDrawOrderEntry{.primitive_kind = Kind::solid_rect},
      cgpui::VulkanFrameDrawOrderEntry{.primitive_kind = Kind::rounded_rect},
      cgpui::VulkanFrameDrawOrderEntry{.primitive_kind = Kind::text},
      cgpui::VulkanFrameDrawOrderEntry{.primitive_kind = Kind::text_selection},
      cgpui::VulkanFrameDrawOrderEntry{.primitive_kind = Kind::text_caret},
      cgpui::VulkanFrameDrawOrderEntry{.primitive_kind = Kind::image},
  };
  const std::array solid_draws{
      cgpui::VulkanRoundedRectDrawRange{.source_index = 0}};
  const std::array rounded_draws{
      cgpui::VulkanRoundedRectDrawRange{.source_index = 0}};
  const std::array text_bindings{
      cgpui::VulkanGlyphAtlasDrawBinding{.text_draw_index = 0},
      cgpui::VulkanGlyphAtlasDrawBinding{.text_draw_index = 0},
  };
  const std::array image_draws{cgpui::ImageDraw{}};
  const std::vector batches = command_batches();
  const cgpui::VulkanFrameDiagnosticResources resources =
      cgpui::vulkan_build_frame_diagnostic_resources(
          draw_order,
          solid_draws,
          rounded_draws,
          text_bindings,
          image_draws,
          batches);
  return resources.planned_resources.size() == 7 &&
                 resources.submitted_resources.size() == 5 &&
                 resources.planned_draws.total_count == 7 &&
                 resources.submitted_draws.total_count == 5 &&
                 resources.submitted_command_count == 4 &&
                 resources.submitted_batch_count == 4 &&
                 resources.dropped_resources.resources.size() == 2 &&
                 resources.dropped_resources.counts.text_selection_count == 1 &&
                 resources.dropped_resources.counts.text_caret_count == 1
             ? 0
             : 10;
}

int test_builds_frame_snapshot() {
  using Kind = cgpui::RendererPrimitiveKind;
  const std::array draw_order{
      cgpui::VulkanFrameDrawOrderEntry{.primitive_kind = Kind::solid_rect},
      cgpui::VulkanFrameDrawOrderEntry{.primitive_kind = Kind::text_selection},
  };
  const std::array solid_draws{
      cgpui::VulkanRoundedRectDrawRange{.source_index = 0}};
  const std::vector batches{
      cgpui::RendererCommandBatch{
          .key = {.primitive_kind = Kind::solid_rect},
          .command_indices = {0},
      },
      cgpui::RendererCommandBatch{
          .key = {.primitive_kind = Kind::text_selection},
          .command_indices = {0},
      },
  };
  const cgpui::VulkanFrameDiagnosticResources resources =
      cgpui::vulkan_build_frame_diagnostic_resources(
          draw_order, solid_draws, {}, {}, {}, batches);
  const cgpui::RendererFrameDiagnosticSnapshot snapshot =
      cgpui::vulkan_build_frame_diagnostic_snapshot(
          draw_order.size(),
          batches.size(),
          resources,
          cgpui::RendererUploadByteCounts{
              .glyph_atlas_byte_count = 3,
              .image_byte_count = 5,
              .total_byte_count = 8,
          },
          cgpui::RendererFrameTimings{.total_nanoseconds = 13});
  return snapshot.work.planned_work.command_count == 2 &&
                 snapshot.work.submitted_work.command_count == 1 &&
                 snapshot.work.pending_command_count == 1 &&
                 snapshot.work.pending_batch_count == 1 &&
                 snapshot.work.pending_draw_count == 1 &&
                 snapshot.upload_bytes.total_byte_count == 8 &&
                 snapshot.timings.total_nanoseconds == 13 &&
                 snapshot.dropped_resources.resources.size() == 1
             ? 0
             : 20;
}

int test_default_renderer_has_no_snapshot() {
  const DefaultDiagnosticRenderer renderer;
  return renderer.last_frame_diagnostic_snapshot() == nullptr ? 0 : 30;
}

int test_step_520_structure_and_documentation() {
  const std::string public_header = read_source(
      "include/cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp");
  const std::string private_header = read_source(
      "src/renderer/vulkan/vulkan_frame_diagnostic_snapshot_internal.hpp");
  const std::string resources = read_source(
      "src/renderer/vulkan/vulkan_frame_diagnostic_resources.cpp");
  const std::string timing = read_source(
      "src/renderer/vulkan/vulkan_frame_diagnostic_timing.cpp");
  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string lifetime =
      read_source("tests/renderer/vulkan_frame_lifetime_test.cpp");
  if (!contains(public_header, "struct RendererFrameDiagnosticSnapshot") ||
      !contains(private_header, "struct VulkanFrameDiagnosticResources") ||
      !contains(resources, "vulkan_build_frame_diagnostic_resources(") ||
      !contains(timing, "VulkanFrameDiagnosticTimer::finish_stage(") ||
      !contains(presentation, "last_frame_diagnostic_snapshot_") ||
      !contains(lifetime, "last_frame_diagnostic_snapshot()")) {
    return 50;
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
      "Phase E Step 520",
      "RendererFrameDiagnosticSnapshot",
      "live Vulkan planned and submitted work",
      "dropped selection and caret resources",
      "Step 521 runtime diagnostic propagation",
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
  if (const int result = test_builds_live_resource_accounting(); result != 0) {
    return result;
  }
  if (const int result = test_builds_frame_snapshot(); result != 0) {
    return result;
  }
  if (const int result = test_default_renderer_has_no_snapshot(); result != 0) {
    return result;
  }
  return test_step_520_structure_and_documentation();
}
