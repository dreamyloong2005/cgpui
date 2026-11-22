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
} // namespace

int main() {
  const std::string wayland = read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string empty = read_source("src/platform/empty.cpp");
  const std::string result_source = read_source("src/ui/runtime_platform_service_system_results.cpp");
  const std::string diagnostics = read_source("src/ui/runtime_platform_services.cpp");
  const std::string manifests = read_source("mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&wayland, &empty, &result_source, &diagnostics, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(wayland, "message dialog unsupported by wayland") ||
      !contains(wayland, "open URL unsupported by wayland") ||
      !contains(empty, "NativeFileDialogResult{") ||
      !contains(empty, ".backend = \"unsupported\"") ||
      !contains(empty, "message dialog unsupported") ||
      !contains(empty, "open URL unsupported") ||
      !contains(empty, "reopen unsupported")) return 3;
  if (!contains(result_source, "ErrorCode::unsupported_platform") ||
      !contains(result_source, "native_message_dialog_result_") ||
      !contains(result_source, "open_url_result_") ||
      !contains(result_source, "reopen_result_") ||
      !contains(diagnostics, "platform_diagnostic_limit = 32")) return 4;
  const char* targets[]{"phase_f_win32_file_open_dialog_structure_test.cpp",
      "phase_f_win32_file_save_dialog_structure_test.cpp",
      "phase_f_native_directory_picker_structure_test.cpp",
      "phase_f_native_message_dialog_structure_test.cpp",
      "phase_f_native_open_url_structure_test.cpp",
      "phase_f_quit_reopen_lifecycle_structure_test.cpp",
      "phase_f_platform_service_result_policy_structure_test.cpp",
      "phase_f_platform_services_closeout_structure_test.cpp"};
  for (const char* target : targets) if (!contains(manifests, target)) return 5;
  return 0;
}
