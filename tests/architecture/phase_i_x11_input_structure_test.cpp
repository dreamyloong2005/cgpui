#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}} : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
}  // namespace

int main() {
  const std::string keyboard =
      read_source("src/platform/linux/x11/x11_keyboard.cpp");
  const std::string keyboard_internal =
      read_source("src/platform/linux/x11/x11_keyboard_internal.hpp");
  const std::string pointer =
      read_source("src/platform/linux/x11/x11_pointer.cpp");
  const std::string cursor =
      read_source("src/platform/linux/x11/x11_cursor_capture.cpp");
  if (keyboard.empty() || keyboard_internal.empty() || pointer.empty() ||
      cursor.empty()) return 1;
  if (!contains(keyboard, "xkb_x11_keymap_new_from_device") ||
      !contains(keyboard, "xkb_state_key_get_utf8") ||
      !contains(pointer, "PointerScrolled") ||
      !contains(pointer, "MouseButton::back") ||
      !contains(cursor, "xcb_grab_pointer") ||
      !contains(cursor, "xcb_cursor_load_cursor")) return 2;
  const std::string module = read_source("build/xmake/phase_i_linux_backends.lua");
  if (!contains(module, "target(\"x11_input_test\")") ||
      !contains(module, "xkbcommon-x11") || !contains(module, "xcb-cursor")) {
    return 3;
  }
  return 0;
}
