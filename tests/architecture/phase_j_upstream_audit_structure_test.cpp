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
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

}  // namespace

int main() {
  const std::string self = read_source(
      "tests/architecture/phase_j_upstream_audit_structure_test.cpp");
  const std::string behavior =
      read_source("tests/api_parity/phase_j_upstream_audit_test.cpp");
  const std::string manifests = read_source("mcpp.toml") +
      read_source("tests/api_parity/mcpp.toml");
  const std::string module =
      read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/architecture/build.mcpp");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  if (self.empty() || behavior.empty() || manifests.empty() || module.empty() ||
      manifest.empty()) {
    return 1;
  }
  if (!contains(manifests, "\"tests/architecture\",") ||
      contains(read_source("mcpp.toml"), "\"phase_j_upstream_audit_test.cpp\"") ||
      !contains(module, "\"phase_j_upstream_audit_test.cpp\"") ||
      !contains(module,
                "\"phase_j_upstream_audit_structure_test.cpp\"") ||
      !contains(module, "source_root_define(2)")) {
    return 2;
  }
  if (!contains(manifest,
                "phase_j_upstream_audit_structure_test|tests/architecture/"
                "phase_j_upstream_audit_structure_test.cpp") ||
      line_count(self) > 90 || line_count(behavior) > 140 ||
      line_count(read_source("tests/architecture/mcpp.toml")) > 240) {
    return 3;
  }
  return 0;
}
