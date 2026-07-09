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
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
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

std::string line_containing(const std::string& text, const char* anchor) {
  const std::size_t anchor_index = text.find(anchor);
  if (anchor_index == std::string::npos) {
    return {};
  }
  const std::size_t line_start = text.rfind('\n', anchor_index);
  const std::size_t line_end = text.find('\n', anchor_index);
  const std::size_t start =
      line_start == std::string::npos ? 0 : line_start + 1U;
  const std::size_t end =
      line_end == std::string::npos ? text.size() : line_end;
  return text.substr(start, end - start);
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string text_hit_testing =
      read_source("include/cgpui/ui/text_hit_testing.hpp");
  const std::string event_pointer =
      read_source("include/cgpui/core/event_pointer.hpp");
  const std::string text_model_header =
      read_source("include/cgpui/ui/text_model.hpp");
  const std::string scroll_header = read_source("include/cgpui/ui/scroll.hpp");
  const std::string text_element_header =
      read_source("include/cgpui/ui/element_text_nodes.hpp");
  const std::string runtime_selection =
      read_source("src/ui/runtime_text_pointer_selection.cpp");
  const std::string runtime_focus =
      read_source("src/ui/runtime_text_focus.cpp");
  const std::string runtime_route =
      read_source("src/ui/runtime_event_route_dispatch.cpp");
  const std::string scroll_source = read_source("src/ui/scroll.cpp");
  const std::string element_geometry =
      read_source("src/ui/element_text_geometry.cpp");
  const std::string selection_paint =
      read_source("src/ui/text_selection_paint_geometry.cpp");
  const std::string text_model_test =
      read_source("tests/ui/text_model_test.cpp");
  const std::string element_test = read_source("tests/ui/element_test.cpp");
  const std::string scroll_test = read_source("tests/ui/scroll_test.cpp");
  const std::string window_text_test =
      read_source("tests/ui/window_runtime_text_test.cpp");
  const std::string window_input_test =
      read_source("tests/ui/window_runtime_input_test.cpp");
  const std::string structure_test =
      read_source("tests/architecture/ui_source_structure_test.cpp");

  if (xmake.empty() || roadmap.empty() || ledger.empty() ||
      task_plan.empty() || findings.empty() || text_hit_testing.empty() ||
      event_pointer.empty() || text_model_header.empty() ||
      scroll_header.empty() || text_element_header.empty() ||
      runtime_selection.empty() || runtime_focus.empty() ||
      runtime_route.empty() || scroll_source.empty() ||
      element_geometry.empty() || selection_paint.empty() ||
      text_model_test.empty() || element_test.empty() || scroll_test.empty() ||
      window_text_test.empty() || window_input_test.empty() ||
      structure_test.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_selection_caret_audit_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_d_selection_caret_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 411-418: Complete selection and caret behavior") ||
      contains(roadmap, "- [ ] Steps 411-418") ||
      !contains(roadmap,
                "Step 418 closes the selection/caret band through") ||
      !contains(roadmap,
                "tests/api_parity/phase_d_selection_caret_audit_test.cpp") ||
      !contains(roadmap, "Steps 419-422 deepen") ||
      !contains(roadmap, "redo invalidation diagnostics")) {
    return 3;
  }

  if (!contains(task_plan,
                "Step 418 closes the selection/caret band") ||
      !contains(task_plan, "TextElement::scroll_caret_into_view(...)") ||
      !contains(findings, "Step 418 closes the selection/caret band")) {
    return 4;
  }

  const std::string text_row = line_containing(ledger, "| gpui text system |");
  if (text_row.empty() || !contains(text_row, "Phase D Steps 379-") ||
      !contains(text_row, "TextSelectionDrag") ||
      !contains(text_row, "word_selection_range_at") ||
      !contains(text_row, "line_selection_range_at") ||
      !contains(text_row, "PointerButton::click_count") ||
      !contains(text_row, "runtime double-click word selection") ||
      !contains(text_row, "runtime triple-click line selection") ||
      !contains(text_row, "text_selection_paint_geometry") ||
      !contains(text_row, "preferred-column vertical navigation") ||
      !contains(text_row, "ScrollModel::scroll_rect_into_view") ||
      !contains(text_row, "TextElement::scroll_caret_into_view") ||
      !contains(text_row, "text_caret_rect") ||
      !contains(text_row, "copy_selection_to_clipboard") ||
      !contains(text_row, "phase_d_selection_caret_audit_test") ||
      !contains(text_row, "edit transaction diagnostics") ||
      !contains(text_row, "IME")) {
    return 5;
  }

  constexpr std::array public_evidence{
      "struct TextSelectionDrag",
      "enum class TextSelectionGranularity",
      "TextSelectionRange word_selection_range_at(",
      "TextSelectionRange line_selection_range_at(",
      "preferred_line_column_",
      "std::uint8_t click_count = 1",
      "scroll_rect_into_view(Rect rect)",
      "std::optional<Rect> caret_rect(",
      "bool scroll_caret_into_view(",
  };
  const std::array public_sources{
      text_hit_testing,
      text_hit_testing,
      text_model_header,
      text_model_header,
      text_model_header,
      event_pointer,
      scroll_header,
      text_element_header,
      text_element_header,
  };
  for (std::size_t index = 0; index < public_evidence.size(); ++index) {
    if (!contains(public_sources[index], public_evidence[index])) {
      return 20 + static_cast<int>(index);
    }
  }

  constexpr std::array source_evidence{
      "text_selection_granularity_for_click_count(",
      "word_selection_range_at(",
      "line_selection_range_at(",
      "preferred_line_column_for_vertical_navigation(",
      "paint_text_selection_ranges(",
      "text_caret_rect(",
      "ScrollModel::scroll_rect_into_view(",
      "TextElement::scroll_caret_into_view(",
      "dynamic_cast<ScrollableListElement*>(",
      "text_element->caret_rect(scale_)",
  };
  const std::array source_files{
      runtime_selection,
      runtime_selection,
      runtime_selection,
      read_source("src/ui/text_model_navigation.cpp"),
      selection_paint,
      selection_paint,
      scroll_source,
      element_geometry,
      runtime_route,
      runtime_focus,
  };
  for (std::size_t index = 0; index < source_evidence.size(); ++index) {
    if (!contains(source_files[index], source_evidence[index]) ||
        !contains(structure_test, source_evidence[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      "test_text_model_reports_word_selection_range_at_offset",
      "test_text_model_reports_line_selection_range_at_offset",
      "test_text_selection_granularity_maps_click_counts",
      "test_runtime_double_click_selects_text_input_word",
      "test_runtime_triple_click_selects_text_input_line",
      "test_text_element_paints_multiline_selection_and_caret_geometry",
      "test_text_model_preserves_preferred_column_for_vertical_navigation",
      "test_scroll_model_scrolls_rect_into_view",
      "test_text_element_scrolls_caret_into_view",
      "test_runtime_scrolls_scrollable_list_route",
      "test_runtime_pastes_clipboard_text_into_focused_text_model",
      "test_runtime_copies_focused_text_selection_to_clipboard",
      "test_runtime_cuts_focused_text_selection_to_clipboard",
  };
  const std::array behavior_sources{
      text_model_test,
      text_model_test,
      text_model_test,
      window_text_test,
      window_text_test,
      element_test,
      text_model_test,
      scroll_test,
      element_test,
      window_input_test,
      window_text_test,
      window_text_test,
      window_text_test,
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    if (!contains(behavior_sources[index], behavior_evidence[index])) {
      return 80 + static_cast<int>(index);
    }
  }

  return 0;
}
