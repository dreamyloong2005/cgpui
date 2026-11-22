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
  const std::string public_header = read_source("include/cgpui/platform/platform_message_dialog.hpp");
  const std::string application = read_source("include/cgpui/platform/platform_application.hpp");
  const std::string empty = read_source("src/platform/empty.cpp");
  const std::string wayland = read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string win32_header = read_source("src/platform/win32/win32_message_dialog_internal.hpp");
  const std::string win32_source = read_source("src/platform/win32/win32_message_dialog.cpp");
  const std::string win32_application = read_source("src/platform/win32/win32_application.cpp");
  const std::string behavior = read_source("tests/platform/win32_message_dialog_plan_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&public_header, &application, &empty, &wayland,
      &win32_header, &win32_source, &win32_application, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "enum class NativeMessageDialogKind") ||
      !contains(public_header, "enum class NativeMessageDialogButtons") ||
      !contains(public_header, "enum class NativeMessageDialogResponse") ||
      !contains(public_header, "struct NativeMessageDialogResult")) return 2;
  if (!contains(application, "show_native_message_dialog(") ||
      !contains(empty, ".backend = \"unsupported\"") ||
      !contains(wayland, ".backend = \"wayland\"") ||
      !contains(wayland, "message dialog unsupported by wayland")) return 3;
  if (!contains(win32_header, "struct Win32MessageDialogPlan") ||
      !contains(win32_source, "MB_ICONQUESTION") ||
      !contains(win32_source, "MB_YESNO") ||
      !contains(win32_source, "MessageBoxW(") ||
      !contains(win32_source, "NativeMessageDialogResponse::cancel") ||
      !contains(win32_application, "show_win32_native_message_dialog(options)")) return 4;
  if (!contains(behavior, "MB_ICONQUESTION") ||
      !contains(behavior, "NativeMessageDialogResponse::yes") ||
      !contains(behavior, "NativeMessageDialogResponse::none")) return 5;
  if (line_count(public_header) > 40 || line_count(win32_header) > 35 ||
      line_count(win32_source) > 75 || line_count(behavior) > 35 ||
      line_count(wayland) > 40 || line_count(win32_application) > 220) return 6;
  if (!contains(manifests, "\"win32_message_dialog_plan_test.cpp\"") ||
      !contains(manifests, "\"phase_f_native_message_dialog_structure_test.cpp\"")) return 7;
  return 0;
}
