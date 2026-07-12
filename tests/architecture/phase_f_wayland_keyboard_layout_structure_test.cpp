#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) {
    source.open(path);
  }
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) {
    lines += value == '\n' ? 1U : 0U;
  }
  return lines;
}

} // namespace

int main() {
  const std::string input =
      read_source("src/platform/linux/wayland_input_internal.hpp");
  const std::string keyboard =
      read_source("src/platform/linux/wayland_keyboard.cpp");
  const std::string application_keyboard =
      read_source("src/platform/linux/wayland_application_keyboard.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_keyboard_layout_test.cpp");
  const std::string compositor_header =
      read_source("tests/platform/wayland_test_compositor.hpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string source_inventory =
      read_source("tests/architecture/wayland_window_source_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &input, &keyboard, &application_keyboard, &behavior, &compositor_header,
      &compositor, &source_inventory, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(input, "mods_depressed = 0") ||
      !contains(input, "mods_latched = 0") ||
      !contains(input, "mods_locked = 0") ||
      !contains(input, "layout_group = 0") ||
      !contains(keyboard, "release_xkb_resources(") ||
      !contains(keyboard, "apply_modifier_state(") ||
      !contains(keyboard, "keyboard.layout_group = group") ||
      !contains(keyboard, "xkb_state_update_mask(")) {
    return 2;
  }
  if (!contains(application_keyboard, "wayland_keyboard_update_modifiers(") ||
      contains(application_keyboard, "xkb_state_update_mask(") ||
      !contains(source_inventory, "release_xkb_resources(") ||
      !contains(source_inventory, "apply_modifier_state(")) {
    return 3;
  }
  if (!contains(behavior, "request_keyboard_modifier_masks(") ||
      !contains(behavior, "request_keyboard_keymap_reload(") ||
      !contains(behavior, "texts[0] != \"Q\"") ||
      !contains(compositor_header, "request_keyboard_modifier_masks(") ||
      !contains(compositor, "name[Group2]") ||
      !contains(compositor, "dispatch_pending_keyboard_keymap(") ||
      !contains(xmake, "target(\"wayland_keyboard_layout_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_keyboard_layout_structure_test\")")) {
    return 4;
  }
  if (line_count(input) > 115 || line_count(keyboard) > 145 ||
      line_count(behavior) > 165) {
    return 5;
  }
  constexpr const char* completion =
      "Phase F Step 556 preserves depressed, latched, and locked Wayland "
      "modifiers plus the active layout group across XKB keymap reloads, "
      "with real multi-layout text and modifier coverage. Step 557 Wayland "
      "pointer enter, leave, and motion production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 6;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_556_remaining_gap\": \"Step 557 Wayland")) {
    return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 640 async timer integration production behavior")) {
    return 8;
  }
  return 0;
}
