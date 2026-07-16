#include <cstdlib>
#include <filesystem>
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
      read_source("build/xmake/phase_j_performance_targets.lua");
  const std::string root = read_source("xmake.lua");
  const std::string runner =
      read_source("tests/performance/phase_j_performance_runner.cpp");
  const std::string audit = read_source(
      "tests/api_parity/phase_j_performance_baseline_audit_test.cpp");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  if (module.empty() || root.empty() || runner.empty() || audit.empty() ||
      manifest.empty()) return 1;
  if (!contains(root, "includes(\"build/xmake/phase_j_performance_targets.lua\")") ||
      !contains(module, "target(\"phase_j_performance_runner\")") ||
      !contains(module, "target(\"phase_j_performance_baseline_audit_test\")") ||
      !contains(module, "target(\"phase_j_performance_baseline_structure_test\")")) {
    return 2;
  }
  if (!contains(module, "tests/performance/phase_j_performance_runner.cpp") ||
      !contains(module, "cgpui_ui") || !contains(module, "add_tests(\"default\")") ||
      !contains(manifest, "phase_j_performance_baseline_structure_test")) {
    return 3;
  }
  if (line_count(runner) > 320 || line_count(module) > 40 ||
      line_count(audit) > 100) return 4;
  return 0;
}
