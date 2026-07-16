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
  const std::string module = read_source("build/xmake/phase_j_stress_targets.lua");
  const std::string root = read_source("xmake.lua");
  const std::string runner = read_source("tests/stress/phase_j_stress_runner.cpp");
  const std::string header = read_source("include/cgpui/ui/test_app.hpp");
  const std::string input = read_source("src/ui/test_app_input.cpp");
  const std::string manifest = read_source("scripts/ci/architecture-header-targets.txt");
  if (module.empty() || root.empty() || runner.empty() || header.empty() ||
      input.empty() || manifest.empty()) return 1;
  if (!contains(root, "includes(\"build/xmake/phase_j_stress_targets.lua\")") ||
      !contains(module, "target(\"phase_j_stress_runner\")") ||
      !contains(module, "target(\"phase_j_stress_audit_test\")") ||
      !contains(module, "target(\"phase_j_stress_structure_test\")")) return 2;
  if (!contains(module, "tests/stress/phase_j_stress_runner.cpp") ||
      !contains(module, "cgpui_ui") ||
      !contains(manifest, "phase_j_stress_structure_test")) return 3;
  if (line_count(runner) > 360 || line_count(module) > 40 ||
      line_count(header) > 145 || line_count(input) > 110) return 4;
  return 0;
}
