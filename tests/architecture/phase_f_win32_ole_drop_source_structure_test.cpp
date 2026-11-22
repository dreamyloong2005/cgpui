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
  const std::string header = read_source("src/platform/win32/win32_drag_drop_internal.hpp");
  const std::string source = read_source("src/platform/win32/win32_ole_drop_source.cpp");
  const std::string behavior = read_source("tests/platform/win32_ole_drop_source_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&header, &source, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(header, "class Win32OleDropSource final : public IDropSource") ||
      !contains(header, "run_win32_ole_drag(")) return 2;
  if (!contains(source, "DRAGDROP_S_CANCEL") || !contains(source, "DRAGDROP_S_DROP") ||
      !contains(source, "DRAGDROP_S_USEDEFAULTCURSORS") || !contains(source, "runner(&data_object")) return 3;
  if (!contains(behavior, "QueryContinueDrag(TRUE") || !contains(behavior, "observed_allowed") ||
      !contains(manifests, "\"win32_ole_drop_source_test.cpp\"") ||
      !contains(manifests, "\"phase_f_win32_ole_drop_source_structure_test.cpp\"")) return 4;
  return 0;
}
