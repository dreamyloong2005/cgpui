#include "cgpui/renderer/renderer_dropped_resource_diagnostics.hpp"

#include <array>
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

int test_exact_resources_are_not_dropped() {
  const std::array<cgpui::RendererFrameResource, 2> resources{
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 0,
          .resource_index = 0,
      },
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::image,
          .command_index = 0,
          .resource_index = 0,
      },
  };
  const std::array supported{
      cgpui::RendererPrimitiveKind::solid_rect,
      cgpui::RendererPrimitiveKind::image,
  };
  const cgpui::RendererDroppedResourceDiagnostics diagnostics =
      cgpui::classify_renderer_dropped_resources(
          resources, resources, supported);
  return diagnostics.empty() && diagnostics.resources.empty() &&
                 diagnostics.counts.total_count == 0
             ? 0
             : 10;
}

int test_unsupported_resources_are_classified() {
  const std::array<cgpui::RendererFrameResource, 4> planned{
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 0,
      },
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::text_selection,
          .command_index = 0,
      },
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::text_caret,
          .command_index = 0,
      },
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::image,
          .command_index = 0,
      },
  };
  const std::array submitted{planned[0], planned[3]};
  const std::array supported{
      cgpui::RendererPrimitiveKind::solid_rect,
      cgpui::RendererPrimitiveKind::image,
  };
  const cgpui::RendererDroppedResourceDiagnostics diagnostics =
      cgpui::classify_renderer_dropped_resources(
          planned, submitted, supported);
  return diagnostics.resources.size() == 2 &&
                 diagnostics.unsupported_primitive_count == 2 &&
                 diagnostics.missing_submission_resource_count == 0 &&
                 diagnostics.counts.text_selection_count == 1 &&
                 diagnostics.counts.text_caret_count == 1 &&
                 diagnostics.counts.total_count == 2 &&
                 diagnostics.resources[0].reason ==
                     cgpui::RendererDroppedResourceReason::unsupported_primitive &&
                 diagnostics.resources[1].resource == planned[2]
             ? 0
             : 20;
}

int test_missing_supported_resource_is_classified() {
  const std::array<cgpui::RendererFrameResource, 2> planned{
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::text,
          .command_index = 3,
          .resource_index = 0,
      },
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::text,
          .command_index = 3,
          .resource_index = 1,
      },
  };
  const std::array submitted{planned[1]};
  const std::array supported{cgpui::RendererPrimitiveKind::text};
  const cgpui::RendererDroppedResourceDiagnostics diagnostics =
      cgpui::classify_renderer_dropped_resources(
          planned, submitted, supported);
  return diagnostics.resources.size() == 1 &&
                 diagnostics.unsupported_primitive_count == 0 &&
                 diagnostics.missing_submission_resource_count == 1 &&
                 diagnostics.counts.text_count == 1 &&
                 diagnostics.resources[0].resource == planned[0] &&
                 diagnostics.resources[0].reason ==
                     cgpui::RendererDroppedResourceReason::
                         missing_submission_resource
             ? 0
             : 30;
}

int test_unexpected_submission_does_not_hide_later_match() {
  const cgpui::RendererFrameResource planned{
      .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
      .command_index = 4,
  };
  const std::array submitted{
      cgpui::RendererFrameResource{
          .primitive_kind = cgpui::RendererPrimitiveKind::image,
          .command_index = 8,
      },
      planned,
  };
  const std::array supported{
      cgpui::RendererPrimitiveKind::solid_rect,
      cgpui::RendererPrimitiveKind::image,
  };
  const cgpui::RendererDroppedResourceDiagnostics diagnostics =
      cgpui::classify_renderer_dropped_resources(
          std::span(&planned, 1), submitted, supported);
  return diagnostics.empty() ? 0 : 40;
}

int test_step_518_structure_and_documentation() {
  const std::string header = read_source(
      "include/cgpui/renderer/renderer_dropped_resource_diagnostics.hpp");
  const std::string source = read_source(
      "src/renderer/renderer_dropped_resource_diagnostics.cpp");
  const std::string aggregate =
      read_source("include/cgpui/renderer/renderer_reports.hpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (!contains(header, "struct RendererDroppedResourceDiagnostics") ||
      !contains(header, "classify_renderer_dropped_resources(") ||
      !contains(source, "classify_renderer_dropped_resources(") ||
      !contains(aggregate, "renderer_dropped_resource_diagnostics.hpp") ||
      !contains(structure, "renderer_dropped_resource_diagnostics.cpp")) {
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
      "Phase E Step 518",
      "RendererDroppedResourceDiagnostics",
      "ordered planned-resource submission gaps",
      "unsupported and missing submission resources",
      "Step 519 frame-timing diagnostics",
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
  if (const int result = test_exact_resources_are_not_dropped(); result != 0) {
    return result;
  }
  if (const int result = test_unsupported_resources_are_classified();
      result != 0) {
    return result;
  }
  if (const int result = test_missing_supported_resource_is_classified();
      result != 0) {
    return result;
  }
  if (const int result = test_unexpected_submission_does_not_hide_later_match();
      result != 0) {
    return result;
  }
  return test_step_518_structure_and_documentation();
}
