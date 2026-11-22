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
  const std::string header = read_source("src/platform/win32/win32_ole_file_data_object_internal.hpp");
  const std::string source = read_source("src/platform/win32/win32_ole_file_data_object.cpp");
  const std::string behavior = read_source("tests/platform/win32_ole_file_data_object_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&header, &source, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "class Win32OleFileDataObject final : public IDataObject") ||
      !contains(header, "run_win32_ole_file_drag(")) return 2;
  if (!contains(source, "MB_ERR_INVALID_CHARS") ||
      !contains(source, "CF_HDROP") || !contains(source, "DROPFILES") ||
      !contains(source, "drop->fWide = TRUE") ||
      !contains(source, "SHCreateStdEnumFmtEtc") ||
      !contains(source, "run_win32_ole_drag(data_object")) return 3;
  if (!contains(behavior, "DragQueryFileW") ||
      !contains(behavior, "IID_IDataObject") ||
      !contains(behavior, "empty_path") ||
      !contains(behavior, "embedded_nul")) return 4;
  if (line_count(header) > 60 || line_count(source) > 170 ||
      line_count(behavior) > 100) return 5;
  if (!contains(manifests, "\"win32_ole_file_data_object_test.cpp\"") ||
      !contains(manifests, "\"phase_f_win32_ole_file_payload_structure_test.cpp\"")) return 6;
  return 0;
}
