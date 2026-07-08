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

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string widget_structure =
      read_source("tests/architecture/widget_source_structure_test.cpp");
  const std::string builtin_widget =
      read_source("tests/ui/builtin_widget_test.cpp");
  const std::string element_test = read_source("tests/ui/element_test.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");

  if (xmake.empty() || widget_structure.empty() || builtin_widget.empty() ||
      element_test.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || vocabulary.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_c_widget_family_structure_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_c_widget_family_structure_test.cpp")) {
    return 2;
  }

  if (!contains(widget_structure, "struct WidgetFamilyBoundary") ||
      !contains(widget_structure, "widget_families") ||
      !contains(widget_structure, "behavior_test_path") ||
      !contains(widget_structure, "behavior_test_signature")) {
    return 3;
  }

  constexpr std::array families{
      "label",
      "button",
      "text_input",
      "toggle_controls",
      "slider",
      "list_menu_items",
      "image_icon_svg",
      "container_primitives",
      "scrollable_list",
  };
  for (std::size_t index = 0; index < families.size(); ++index) {
    if (!contains(widget_structure, families[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      "test_label_widget_builder_builds_accessible_text",
      "test_button_label_convenience_builds_accessible_button",
      "test_text_input_widget_builder_keeps_text_model_boundary",
      "test_checkbox_widget_builder_tracks_checked_state",
      "test_slider_widget_builder_tracks_range_value_and_click_updates",
      "test_list_item_widget_builder_tracks_selection_and_clicks",
      "test_svg_image_source_builds_front_end_element",
      "test_container_primitive_builder_exposes_horizontal_stack",
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    if (!contains(builtin_widget, behavior_evidence[index]) ||
        !contains(widget_structure, behavior_evidence[index])) {
      return 30 + static_cast<int>(index);
    }
  }
  if (!contains(element_test,
                "test_scrollable_list_container_keys_clip_and_scroll_offset") ||
      !contains(widget_structure,
                "test_scrollable_list_container_keys_clip_and_scroll_offset")) {
    return 50;
  }

  constexpr std::array doc_evidence{
      "Phase C Step 367 widget family structure tests",
      "tests/api_parity/phase_c_widget_family_structure_test.cpp",
      "tests/architecture/widget_source_structure_test.cpp",
      "public leaf header",
      "focused source file",
      "focused behavior tests",
      "Phase C Step 373 final element/style/widget ledger audit",
  };
  for (std::size_t index = 0; index < doc_evidence.size(); ++index) {
    if (!contains(roadmap, doc_evidence[index]) ||
        !contains(ledger_md, doc_evidence[index]) ||
        !contains(ledger_json, doc_evidence[index]) ||
        !contains(vocabulary, doc_evidence[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  if (!contains(roadmap,
                "- [x] Steps 367-372: Add structure tests requiring every "
                "widget family") ||
      !contains(ledger_json,
                "\"widget_family_structure_status\": \"phase_c_guarded\"") ||
      !contains(ledger_json,
                "\"widget_family_structure_next\": \"Phase C Step 373 final "
                "element/style/widget ledger audit\"")) {
    return 80;
  }

  return 0;
}
