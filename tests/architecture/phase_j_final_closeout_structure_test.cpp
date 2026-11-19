#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string self = read_source(
      "tests/architecture/phase_j_final_closeout_structure_test.cpp");
  const std::string audit =
      read_source("tests/api_parity/phase_j_final_closeout_test.cpp");
  const std::string module = read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/architecture/build.mcpp");
  const std::string root = read_source("mcpp.toml") +
      read_source("tests/api_parity/mcpp.toml");
  const std::string manifest = read_source("scripts/ci/architecture-header-targets.txt");
  const std::string drift = read_source("docs/gpui-upstream-drift-policy.md");
  const std::string verification =
      read_source("docs/gpui-phase-j-final-verification.json");
  if (self.empty() || audit.empty() || module.empty() || root.empty() ||
      manifest.empty() || drift.empty() || verification.empty()) return 1;
  if (!contains(root, "\"tests/architecture\",") ||
      contains(read_source("mcpp.toml"), "\"phase_j_final_closeout_test.cpp\"") ||
      !contains(module, "\"phase_j_final_closeout_test.cpp\"") ||
      !contains(module, "\"phase_j_final_closeout_structure_test.cpp\"") ||
      !contains(module, "source_root_define(2)")) return 2;
  if (!contains(manifest,
                "phase_j_final_closeout_structure_test|tests/architecture/"
                "phase_j_final_closeout_structure_test.cpp")) return 3;
  if (line_count(self) > 90 || line_count(audit) > 130 ||
      line_count(read_source("tests/architecture/mcpp.toml")) > 240 || line_count(drift) > 120) return 4;
  return 0;
}
