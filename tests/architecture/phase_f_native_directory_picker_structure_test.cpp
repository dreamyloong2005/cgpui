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
  const std::string public_header = read_source("include/cgpui/platform/platform_file_dialog.hpp");
  const std::string plan = read_source("src/platform/win32/win32_file_dialog.cpp");
  const std::string behavior = read_source("tests/platform/win32_directory_dialog_plan_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&public_header, &plan, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "pick_directory,")) return 2;
  if (!contains(plan, "NativeFileDialogKind::pick_directory") ||
      !contains(plan, "FOS_PICKFOLDERS | FOS_PATHMUSTEXIST") ||
      !contains(plan, "if (options.kind == NativeFileDialogKind::pick_directory) break")) return 3;
  if (!contains(behavior, "FOS_PICKFOLDERS") ||
      !contains(behavior, "FOS_PATHMUSTEXIST") ||
      !contains(behavior, "!directory->filters.empty()")) return 4;
  if (line_count(public_header) > 45 || line_count(plan) > 160 ||
      line_count(behavior) > 35) return 5;
  if (!contains(manifests, "\"win32_directory_dialog_plan_test.cpp\"") ||
      !contains(manifests, "\"phase_f_native_directory_picker_structure_test.cpp\"")) return 6;
  return 0;
}
