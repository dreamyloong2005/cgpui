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
  const std::string wayland = read_source("src/platform/linux/wayland_native.cpp");
  const std::string manifests = read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&wayland, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(wayland, ".supported = false") ||
      !contains(wayland, ".backend = \"wayland\"") ||
      !contains(wayland, ".skipped_accelerator_count = accelerator_count")) return 3;
  if (!contains(manifests, "\"phase_f_native_menu_closeout_structure_test.cpp\"") ||
      !contains(manifests, "\"phase_f_native_menu_dispatch_diagnostics_structure_test.cpp\"") ||
      !contains(manifests, "\"phase_f_win32_native_menu_tree_structure_test.cpp\"")) return 4;
  return 0;
}
