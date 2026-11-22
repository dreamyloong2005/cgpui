#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &input, &keyboard, &application_keyboard, &behavior, &compositor_header,
      &compositor, &source_inventory, &manifests};
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
      !contains(manifests, "\"wayland_keyboard_layout_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_wayland_keyboard_layout_structure_test.cpp\"")) {
    return 4;
  }
  if (line_count(input) > 117 || line_count(keyboard) > 145 ||
      line_count(behavior) > 165) {
    return 5;
  }
  return 0;
}
