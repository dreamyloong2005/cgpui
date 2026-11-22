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
      "src/platform/win32/win32_file_dialog_internal.hpp");
  const std::string source = read_source(
      "src/platform/win32/win32_file_dialog.cpp");
  const std::string state = read_source("src/platform/win32/win32_native.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_file_open_dialog_plan_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&header, &source, &state, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "struct Win32FileDialogPlan") ||
      !contains(header, "win32_file_dialog_plan(") ||
      !contains(header, "show_win32_native_file_dialog(")) return 2;
  if (!contains(source, "CoInitializeEx(") ||
      !contains(source, "CLSID_FileOpenDialog") ||
      !contains(source, "FOS_ALLOWMULTISELECT") ||
      !contains(source, "SetFileTypes(") || !contains(source, "Show(nullptr)") ||
      !contains(source, "ERROR_CANCELLED") || !contains(source, "GetResults(") ||
      !contains(source, "SIGDN_FILESYSPATH")) return 3;
  if (!contains(state, "show_win32_native_file_dialog(options_)") ||
      contains(state, ".supported = false,\n      .accepted = false,\n      .backend = \"win32\"")) return 4;
  if (!contains(behavior, "FOS_FILEMUSTEXIST") ||
      !contains(behavior, "FOS_ALLOWMULTISELECT") ||
      !contains(behavior, "L\"*.cpp;*.hpp;*.cxx\"")) return 5;
  if (line_count(header) > 55 || line_count(source) > 180 ||
      line_count(state) > 80 || line_count(behavior) > 45) return 6;
  if (!contains(manifests, "\"win32_file_open_dialog_plan_test.cpp\"") ||
      !contains(manifests, "\"phase_f_win32_file_open_dialog_structure_test.cpp\"")) return 7;
  return 0;
}
