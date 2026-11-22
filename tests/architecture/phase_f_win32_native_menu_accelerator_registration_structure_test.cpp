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
  const std::string header = read_source(
      "src/platform/win32/win32_native_menu_accelerator_table_internal.hpp");
  const std::string source = read_source(
      "src/platform/win32/win32_native_menu_accelerator_table.cpp");
  const std::string native_header = read_source(
      "src/platform/win32/win32_native_internal.hpp");
  const std::string native_source = read_source(
      "src/platform/win32/win32_native.cpp");
  const std::string application = read_source(
      "src/platform/win32/win32_application.cpp");
  const std::string tree = read_source(
      "src/platform/win32/win32_native_menu_tree.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_native_menu_accelerator_registration_test.cpp");
  const std::string win32_structure = read_source(
      "tests/architecture/win32_window_source_test.cpp");
  const std::string platform_structure = read_source(
      "tests/architecture/platform_source_structure_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &header, &source, &native_header, &native_source, &application, &tree,
      &behavior, &win32_structure, &platform_structure, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "class Win32NativeMenuAcceleratorTable") ||
      !contains(header, "win32_native_menu_first_command_id = 0x1000U") ||
      !contains(header, "Win32NativeMenuAcceleratorTable&& other") ||
      !contains(header, "bool translate(MSG& message) const")) return 2;
  if (!contains(source, "CreateAcceleratorTableW(") ||
      !contains(source, "DestroyAcceleratorTable(table_)") ||
      !contains(source, "TranslateAcceleratorW(message.hwnd, table_, &message)") ||
      !contains(source, "flags = FVIRTKEY") ||
      !contains(source, "flags |= FCONTROL") ||
      !contains(source, "flags |= FALT") ||
      !contains(source, "flags |= FSHIFT") ||
      !contains(source, "modifiers.super") ||
      !contains(source, "KeyAction::pressed")) return 3;
  if (!contains(native_header, "Win32NativeMenuAcceleratorTable accelerator_table_") ||
      !contains(native_header, "translate_accelerator(MSG& message) const") ||
      !contains(native_source, "Win32NativeMenuAcceleratorTable::build(menu)") ||
      !contains(native_source, "menu_tree.has_value() && accelerator_table.has_value()") ||
      !contains(native_source, "accelerator_table_ = std::move(*accelerator_table)")) return 4;
  const auto accelerator = application.find(
      "native_menu_state_.translate_accelerator(message)");
  const auto ordinary = application.find("TranslateMessage(&message)", accelerator);
  if (accelerator == std::string::npos || ordinary == std::string::npos ||
      !contains(tree, "win32_native_menu_first_command_id")) return 5;
  if (!contains(behavior, "WM_KEYDOWN") ||
      !contains(behavior, "WM_COMMAND") ||
      !contains(behavior, "observed_command_id.load() != 0x1000U")) return 6;
  if (!contains(win32_structure, "win32_native_menu_accelerator_table_internal.hpp") ||
      !contains(win32_structure, "win32_native_menu_accelerator_table.cpp") ||
      !contains(platform_structure, "win32_native_menu_accelerator_table_internal.hpp") ||
      !contains(platform_structure, "win32_native_menu_accelerator_table.cpp")) return 7;
  if (line_count(header) > 60 || line_count(source) > 130 ||
      line_count(application) > 170 || line_count(tree) > 130 ||
      line_count(behavior) > 130) return 8;
  if (!contains(manifests, "\"win32_native_menu_accelerator_registration_test.cpp\"") ||
      !contains(manifests, "\"phase_f_win32_native_menu_accelerator_registration_structure_test.cpp\"")) return 9;
  return 0;
}
