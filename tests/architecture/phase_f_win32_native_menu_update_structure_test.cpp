#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines;
}
} // namespace

int main() {
  const std::string tree = read_source(
      "src/platform/win32/win32_native_menu_tree.cpp");
  const std::string state = read_source("src/platform/win32/win32_native.cpp");
  const std::string application = read_source(
      "src/platform/win32/win32_application.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_native_menu_update_test.cpp");
  const std::string state_structure = read_source(
      "tests/architecture/phase_f_win32_native_menu_state_structure_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &tree, &state, &application, &behavior, &state_structure, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(tree, "if (model.items.empty()) return Win32NativeMenuTree{}") ||
      !contains(tree, "std::optional<Win32NativeMenuTree>") ||
      !contains(state, "menu_tree_ = std::move(*menu_tree)")) return 2;
  if (!contains(application, "win32_apply_native_menu(window_handle(window), nullptr)") ||
      !contains(application, "windows_.visit(") ||
      !contains(application, "apply_native_menu(window)") ||
      !contains(application, "apply_native_menu(window->get())")) return 3;
  if (!contains(behavior, "first_window") ||
      !contains(behavior, "second_window") ||
      !contains(behavior, "updated_root == initial_root") ||
      !contains(behavior, "install_native_menu({})") ||
      !contains(behavior, "GetMenu(first_hwnd) != nullptr") ||
      !contains(behavior, "future_window")) return 4;
  if (!contains(state_structure, "MFT_RADIOCHECK") ||
      !contains(manifests, "\"win32_native_menu_update_test.cpp\"") ||
      !contains(manifests, "\"phase_f_win32_native_menu_update_structure_test.cpp\"")) return 5;
  if (line_count(tree) > 125 || line_count(state) > 70 ||
      line_count(application) > 190 || line_count(behavior) > 130) return 6;
  return 0;
}
