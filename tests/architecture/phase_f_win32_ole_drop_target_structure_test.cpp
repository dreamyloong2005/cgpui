#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
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
  const std::string header =
      read_source("src/platform/win32/win32_drag_drop_internal.hpp");
  const std::string target =
      read_source("src/platform/win32/win32_ole_drop_target.cpp");
  const std::string behavior =
      read_source("tests/platform/win32_ole_drop_target_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &header, &target, &behavior, &manifests};
  for (const std::string* value : required) if (value->empty()) return 1;
  if (!contains(header, "class Win32OleDropTarget final : public IDropTarget") ||
      !contains(target, "Win32OleDropTarget::QueryInterface(") ||
      !contains(target, "Win32OleDropTarget::AddRef()") ||
      !contains(target, "Win32OleDropTarget::Release()")) return 2;
  if (!contains(target, "if (effect == nullptr) return E_INVALIDARG") ||
      !contains(target, "if (data_object == nullptr)") ||
      !contains(target, "*effect = DROPEFFECT_NONE") ||
      !contains(target, "owner_->ole_drag_entered(") ||
      !contains(target, "owner_->ole_drag_dropped(")) return 3;
  if (!contains(behavior, "IID_IClassFactory") ||
      !contains(behavior, "target.Release() != 1") ||
      !contains(behavior, "E_INVALIDARG") ||
      !contains(behavior, "DROPEFFECT_NONE") ||
      !contains(behavior, "owner.updated != 1")) return 4;
  if (!contains(manifests, "\"win32_ole_drop_target_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_win32_ole_drop_target_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(header) > 120 || line_count(target) > 100 ||
      line_count(behavior) > 90) return 6;
  return 0;
}
