#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, std::string_view value) {
  return text.find(value) != std::string::npos;
}
std::size_t line_count(const std::string& text) {
  std::size_t result = 0;
  for (const char value : text) result += value == '\n' ? 1U : 0U;
  return result + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}
}  // namespace

int main() {
  const std::string module =
      read_source("build/xmake/phase_j_documentation_targets.lua");
  const std::string root = read_source("xmake.lua");
  const std::string manifest = read_source("scripts/ci/architecture-header-targets.txt");
  if (module.empty() || root.empty() || manifest.empty()) return 1;
  if (!contains(root, "includes(\"build/xmake/phase_j_documentation_targets.lua\")") ||
      !contains(module, "target(\"phase_j_documentation_audit_test\")") ||
      !contains(module, "target(\"phase_j_documentation_structure_test\")") ||
      !contains(manifest, "phase_j_documentation_structure_test")) return 2;
  const std::array paths{
      "README.md", "docs/getting-started.md", "docs/architecture.md",
      "docs/platform-dependencies.md", "docs/examples.md",
      "docs/gpui-to-cpp23-migration.md", "docs/non-goals.md"};
  for (const auto* path : paths) {
    const std::string source = read_source(path);
    if (source.empty() || line_count(source) > 180) return 3;
  }
  if (line_count(module) > 30) return 4;
  return 0;
}
