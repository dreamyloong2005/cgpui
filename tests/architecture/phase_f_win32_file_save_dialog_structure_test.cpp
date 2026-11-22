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
  const std::string header = read_source("src/platform/win32/win32_file_dialog_internal.hpp");
  const std::string plan = read_source("src/platform/win32/win32_file_dialog.cpp");
  const std::string save = read_source("src/platform/win32/win32_file_dialog_save.cpp");
  const std::string behavior = read_source("tests/platform/win32_file_save_dialog_plan_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&header, &plan, &save, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "show_win32_native_save_file_dialog(") ||
      !contains(header, "suggested_name")) return 2;
  if (!contains(plan, "FOS_OVERWRITEPROMPT") ||
      !contains(plan, "show_win32_native_save_file_dialog(options, *plan)")) return 3;
  if (!contains(save, "CLSID_FileSaveDialog") ||
      !contains(save, "SetFileName(") || !contains(save, "SetFileTypes(") ||
      !contains(save, "ERROR_CANCELLED") || !contains(save, "GetResult(") ||
      !contains(save, "result.paths.size() == 1")) return 4;
  if (!contains(behavior, "FOS_OVERWRITEPROMPT") ||
      !contains(behavior, "project.cgpui") ||
      !contains(behavior, "L\"*.cgpui\"")) return 5;
  if (line_count(header) > 55 || line_count(plan) > 160 ||
      line_count(save) > 90 || line_count(behavior) > 35) return 6;
  if (!contains(manifests, "\"win32_file_save_dialog_plan_test.cpp\"") ||
      !contains(manifests, "\"phase_f_win32_file_save_dialog_structure_test.cpp\"")) return 7;
  return 0;
}
