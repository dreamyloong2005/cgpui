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
  const std::string xmake = read_source("xmake.lua");
  const std::string module =
      read_source("build/xmake/phase_j_audit_targets.lua");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  if (self.empty() || behavior.empty() || xmake.empty() || module.empty() ||
      manifest.empty()) {
    return 1;
  }
  if (!contains(xmake, "includes(\"build/xmake/phase_j_audit_targets.lua\")") ||
      contains(xmake, "target(\"phase_j_upstream_audit_test\")") ||
      !contains(module, "target(\"phase_j_upstream_audit_test\")") ||
      !contains(module,
                "target(\"phase_j_upstream_audit_structure_test\")") ||
      !contains(module, "CGPUI_SOURCE_ROOT")) {
    return 2;
  }
  if (!contains(manifest,
                "phase_j_upstream_audit_structure_test|tests/architecture/"
                "phase_j_upstream_audit_structure_test.cpp") ||
      line_count(self) > 90 || line_count(behavior) > 140 ||
      line_count(module) > 30) {
    return 3;
  }
  return 0;
}
