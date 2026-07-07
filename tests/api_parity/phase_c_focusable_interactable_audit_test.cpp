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
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string xmake = read_source("xmake.lua");

  if (roadmap.empty() || ledger_md.empty() || ledger_json.empty() ||
      xmake.empty()) {
    return 1;
  }

  if (!contains(xmake,
                "target(\"phase_c_focusable_interactable_audit_test\")") ||
      !contains(xmake,
                "tests/api_parity/"
                "phase_c_focusable_interactable_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 337-342: Complete focusable/interactable element "
                "semantics") ||
      !contains(roadmap,
                "Step 342 closes the focusable/interactable band") ||
      !contains(roadmap,
                "tests/api_parity/"
                "phase_c_focusable_interactable_audit_test.cpp") ||
      !contains(roadmap, "Step 343 starts built-in widget expansion")) {
    return 3;
  }
  if (contains(roadmap, "Step 342 should close")) {
    return 4;
  }

  constexpr std::array roadmap_evidence{
      "Step 337 starts the focusable/interactable band",
      "Step 338 adds tab-order/focus-ring metadata",
      "Step 339 adds focused click/drag gesture synthesis",
      "Step 340 adds keyboard activation semantics",
      "Step 341 adds disabled interaction semantics",
  };
  for (std::size_t index = 0; index < roadmap_evidence.size(); ++index) {
    if (!contains(roadmap, roadmap_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array ledger_evidence{
      "Phase C Step 337 pointer-active input semantics",
      "ViewInputState::active_element_id",
      "runtime_active_state.cpp",
      "Phase C Step 338 tab-order/focus-ring metadata",
      "FocusMetadata",
      "runtime_focus_order.cpp",
      "Phase C Step 339 click/drag gesture synthesis",
      "ElementGestureKind::click",
      "runtime_gesture_synthesis.cpp",
      "Phase C Step 340 keyboard activation semantics",
      "should_dispatch_synthesized_keyboard_activation_event",
      "test_runtime_enter_space_activate_focused_button",
      "Phase C Step 341 disabled interaction semantics",
      "WindowRuntime::refresh_disabled_interaction_state",
      "runtime_disabled_interaction.cpp",
      "test_runtime_clears_disabled_interaction_state",
  };
  for (std::size_t index = 0; index < ledger_evidence.size(); ++index) {
    if (!contains(ledger_md, ledger_evidence[index]) ||
        !contains(ledger_json, ledger_evidence[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_md,
                "Phase C Step 342 focusable/interactable band closeout") ||
      !contains(ledger_json,
                "Phase C Step 342 focusable/interactable band closeout") ||
      !contains(ledger_md,
                "phase_c_focusable_interactable_audit_test.cpp") ||
      !contains(ledger_json,
                "phase_c_focusable_interactable_audit_test.cpp") ||
      !contains(ledger_md, "Phase C Step 343 built-in widget expansion") ||
      !contains(ledger_json, "Phase C Step 343 built-in widget expansion") ||
      !contains(ledger_md, "ButtonBuilder::label") ||
      !contains(ledger_json, "ButtonBuilder::label") ||
      !contains(ledger_md, "widget_source_structure_test.cpp") ||
      !contains(ledger_json, "widget_source_structure_test.cpp") ||
      !contains(ledger_md,
                "Phase C Step 344 checkbox/radio/switch widgets") ||
      !contains(ledger_json,
                "Phase C Step 344 checkbox/radio/switch widgets") ||
      !contains(ledger_md, "ToggleBuilder") ||
      !contains(ledger_json, "ToggleBuilder") ||
      !contains(ledger_md, "toggle_switch(...)") ||
      !contains(ledger_json, "toggle_switch(...)") ||
      !contains(ledger_md, "src/ui/widgets/toggle_builder.cpp") ||
      !contains(ledger_json, "src/ui/widgets/toggle_builder.cpp") ||
      !contains(ledger_md, "Phase C Step 345 slider widget") ||
      !contains(ledger_json, "Phase C Step 345 slider widget") ||
      !contains(ledger_md, "SliderBuilder") ||
      !contains(ledger_json, "SliderBuilder") ||
      !contains(ledger_md, "slider(...)") ||
      !contains(ledger_json, "slider(...)") ||
      !contains(ledger_md, "src/ui/widgets/slider_builder.cpp") ||
      !contains(ledger_json, "src/ui/widgets/slider_builder.cpp") ||
      !contains(ledger_md, "src/ui/element_slider_nodes.cpp") ||
      !contains(ledger_json, "src/ui/element_slider_nodes.cpp") ||
      !contains(ledger_md, "PlatformAccessibilityRole::slider") ||
      !contains(ledger_json, "PlatformAccessibilityRole::slider") ||
      !contains(ledger_md, "Phase C Step 346 list/menu widget follow-up") ||
      !contains(ledger_json, "Phase C Step 346 list/menu widget follow-up") ||
      !contains(ledger_md, "ItemBuilder") ||
      !contains(ledger_json, "ItemBuilder") ||
      !contains(ledger_md, "list_item(...)") ||
      !contains(ledger_json, "list_item(...)") ||
      !contains(ledger_md, "menu_item(...)") ||
      !contains(ledger_json, "menu_item(...)") ||
      !contains(ledger_md, "src/ui/widgets/item_builder.cpp") ||
      !contains(ledger_json, "src/ui/widgets/item_builder.cpp") ||
      !contains(ledger_md, "src/ui/element_item_nodes.cpp") ||
      !contains(ledger_json, "src/ui/element_item_nodes.cpp") ||
      !contains(ledger_md, "PlatformAccessibilityRole::list_item") ||
      !contains(ledger_json, "PlatformAccessibilityRole::list_item") ||
      !contains(ledger_md, "Phase C Step 347 icon/image widget follow-up") ||
      !contains(ledger_json, "Phase C Step 347 icon/image widget follow-up") ||
      !contains(ledger_md, "ImageBuilder") ||
      !contains(ledger_json, "ImageBuilder") ||
      !contains(ledger_md, "image(...)") ||
      !contains(ledger_json, "image(...)") ||
      !contains(ledger_md, "icon(...)") ||
      !contains(ledger_json, "icon(...)") ||
      !contains(ledger_md, "src/ui/widgets/image_builder.cpp") ||
      !contains(ledger_json, "src/ui/widgets/image_builder.cpp") ||
      !contains(ledger_md, "src/ui/element_image_nodes.cpp") ||
      !contains(ledger_json, "src/ui/element_image_nodes.cpp") ||
      !contains(ledger_md, "PlatformAccessibilityRole::image") ||
      !contains(ledger_json, "PlatformAccessibilityRole::image") ||
      !contains(ledger_md, "Phase C Step 348 container primitive follow-up") ||
      !contains(ledger_json, "Phase C Step 348 container primitive follow-up") ||
      !contains(ledger_md, "container_builder.hpp") ||
      !contains(ledger_json, "container_builder.hpp") ||
      !contains(ledger_md, "src/ui/widgets/container_builder.cpp") ||
      !contains(ledger_json, "src/ui/widgets/container_builder.cpp") ||
      !contains(ledger_md, "h_stack()") ||
      !contains(ledger_json, "h_stack()") ||
      !contains(ledger_md, "Phase C Step 349 uniform list parity") ||
      !contains(ledger_json, "Phase C Step 349 uniform list parity") ||
      !contains(ledger_md, "UniformListVisibleRange") ||
      !contains(ledger_json, "UniformListVisibleRange") ||
      !contains(ledger_md, "UniformListItemIdentity") ||
      !contains(ledger_json, "UniformListItemIdentity") ||
      !contains(ledger_md, "UniformListItemMeasurement") ||
      !contains(ledger_json, "UniformListItemMeasurement") ||
      !contains(ledger_md, "UniformListItemMeasurementResult") ||
      !contains(ledger_json, "UniformListItemMeasurementResult") ||
      !contains(ledger_md, "UniformListRecyclingWindow") ||
      !contains(ledger_json, "UniformListRecyclingWindow") ||
      !contains(ledger_md, "UniformListLayoutSnapshot") ||
      !contains(ledger_json, "UniformListLayoutSnapshot") ||
      !contains(ledger_md, "UniformListScrollAnchor") ||
      !contains(ledger_json, "UniformListScrollAnchor") ||
      !contains(ledger_md, "UniformListItemMeasurementCache") ||
      !contains(ledger_json, "UniformListItemMeasurementCache") ||
      !contains(ledger_md, "calculate_uniform_list_visible_range") ||
      !contains(ledger_json, "calculate_uniform_list_visible_range") ||
      !contains(ledger_md, "measure_uniform_list_items") ||
      !contains(ledger_json, "measure_uniform_list_items") ||
      !contains(ledger_md, "calculate_uniform_list_recycling_window") ||
      !contains(ledger_json, "calculate_uniform_list_recycling_window") ||
      !contains(ledger_md, "capture_uniform_list_scroll_anchor") ||
      !contains(ledger_json, "capture_uniform_list_scroll_anchor") ||
      !contains(ledger_md, "apply_uniform_list_scroll_anchor") ||
      !contains(ledger_json, "apply_uniform_list_scroll_anchor") ||
      !contains(ledger_md, "ScrollableListElement::layout_snapshot") ||
      !contains(ledger_json, "ScrollableListElement::layout_snapshot") ||
      !contains(ledger_md, "ScrollableListElement::measurement_cache") ||
      !contains(ledger_json, "ScrollableListElement::measurement_cache") ||
      !contains(ledger_md, "src/ui/uniform_list.cpp") ||
      !contains(ledger_json, "src/ui/uniform_list.cpp") ||
      !contains(ledger_md, "src/ui/uniform_list_measurement.cpp") ||
      !contains(ledger_json, "src/ui/uniform_list_measurement.cpp") ||
      !contains(ledger_md, "src/ui/uniform_list_recycling.cpp") ||
      !contains(ledger_json, "src/ui/uniform_list_recycling.cpp") ||
      !contains(ledger_md, "src/ui/element_scroll_layout.cpp") ||
      !contains(ledger_json, "src/ui/element_scroll_layout.cpp") ||
      !contains(ledger_md, "Phase C Step 350 scroll anchoring") ||
      !contains(ledger_json, "Phase C Step 350 scroll anchoring") ||
      !contains(ledger_md, "Phase C Step 351 item measurement cache") ||
      !contains(ledger_json, "Phase C Step 351 item measurement cache") ||
      !contains(ledger_md, "Phase C Step 352 large-list recycling") ||
      !contains(ledger_json, "Phase C Step 352 large-list recycling") ||
      !contains(ledger_json,
                "\"next_step\": \"Phase C Step 353 keyboard/pointer selection\"")) {
    return 50;
  }

  constexpr std::array closeout_exclusions{
      "broad widget behavior",
      "runtime theme switching",
      "broad resolved-style",
      "layout/paint rewrites",
      "ClipboardItem",
      "gpui::test",
      "action macro payloads",
      "task priorities",
      "structured task groups",
  };
  for (std::size_t index = 0; index < closeout_exclusions.size(); ++index) {
    if (!contains(roadmap, closeout_exclusions[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  return 0;
}
