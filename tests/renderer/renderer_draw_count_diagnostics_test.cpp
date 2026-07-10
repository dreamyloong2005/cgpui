#include "cgpui/renderer/renderer_frame_diagnostics.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
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

int test_counts_every_primitive_kind() {
  cgpui::RendererDrawCounts counts;
  cgpui::renderer_add_draw_count(
      counts, cgpui::RendererPrimitiveKind::solid_rect);
  cgpui::renderer_add_draw_count(
      counts, cgpui::RendererPrimitiveKind::rounded_rect, 2);
  cgpui::renderer_add_draw_count(
      counts, cgpui::RendererPrimitiveKind::text, 3);
  cgpui::renderer_add_draw_count(
      counts, cgpui::RendererPrimitiveKind::text_selection, 4);
  cgpui::renderer_add_draw_count(
      counts, cgpui::RendererPrimitiveKind::text_caret, 5);
  cgpui::renderer_add_draw_count(
      counts, cgpui::RendererPrimitiveKind::image, 6);
  return counts.solid_rect_count == 1 && counts.rounded_rect_count == 2 &&
                 counts.text_count == 3 &&
                 counts.text_selection_count == 4 &&
                 counts.text_caret_count == 5 && counts.image_count == 6 &&
                 counts.total_count == 21 && !counts.saturated
             ? 0
             : 10;
}

int test_zero_draw_add_is_a_noop() {
  cgpui::RendererDrawCounts counts;
  cgpui::renderer_add_draw_count(
      counts, cgpui::RendererPrimitiveKind::image, 0);
  return counts.image_count == 0 && counts.total_count == 0 &&
                 !counts.saturated
             ? 0
             : 20;
}

int test_draw_count_saturates() {
  cgpui::RendererDrawCounts counts;
  cgpui::renderer_add_draw_count(
      counts,
      cgpui::RendererPrimitiveKind::text,
      std::numeric_limits<std::size_t>::max());
  cgpui::renderer_add_draw_count(
      counts, cgpui::RendererPrimitiveKind::text, 1);
  return counts.text_count == std::numeric_limits<std::size_t>::max() &&
                 counts.total_count ==
                     std::numeric_limits<std::size_t>::max() &&
                 counts.saturated
             ? 0
             : 30;
}

int test_draw_count_comparison() {
  const cgpui::RendererFrameDiagnostics pending =
      cgpui::compare_renderer_frame_work(
          {.draw_count = 11},
          {.draw_count = 7});
  if (pending.exact_match() || pending.pending_draw_count != 4 ||
      pending.unexpected_draw_count != 0) {
    return 40;
  }
  const cgpui::RendererFrameDiagnostics unexpected =
      cgpui::compare_renderer_frame_work(
          {.draw_count = 3},
          {.draw_count = 8});
  return !unexpected.exact_match() && unexpected.pending_draw_count == 0 &&
                 unexpected.unexpected_draw_count == 5
             ? 0
             : 41;
}

int test_step_517_structure_and_documentation() {
  const std::string header = read_source(
      "include/cgpui/renderer/renderer_frame_diagnostics.hpp");
  const std::string source = read_source(
      "src/renderer/renderer_frame_draw_diagnostics.cpp");
  const std::string structure =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  if (!contains(header, "struct RendererDrawCounts") ||
      !contains(header, "renderer_add_draw_count(") ||
      !contains(source, "renderer_add_draw_count(") ||
      !contains(structure, "renderer_frame_draw_diagnostics.cpp")) {
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
      "Phase E Step 517",
      "RendererDrawCounts",
      "primitive-aware GPU draw counts",
      "saturation-safe draw-count accounting",
      "Step 518 dropped-resource accounting",
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
  if (const int result = test_counts_every_primitive_kind(); result != 0) {
    return result;
  }
  if (const int result = test_zero_draw_add_is_a_noop(); result != 0) {
    return result;
  }
  if (const int result = test_draw_count_saturates(); result != 0) {
    return result;
  }
  if (const int result = test_draw_count_comparison(); result != 0) {
    return result;
  }
  return test_step_517_structure_and_documentation();
}
