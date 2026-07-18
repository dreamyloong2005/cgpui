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

std::size_t count(const std::string& text, std::string_view value) {
  std::size_t result = 0;
  for (std::size_t offset = text.find(value); offset != std::string::npos;
       offset = text.find(value, offset + value.size())) {
    ++result;
  }
  return result;
}

std::size_t line_count(const std::string& text) {
  std::size_t result = 0;
  for (const char value : text) result += value == '\n' ? 1U : 0U;
  return result + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

}  // namespace

int main() {
  const std::string self = read_source(
      "tests/architecture/phase_j_macos_completion_structure_test.cpp");
  const std::string behavior =
      read_source("tests/api_parity/phase_j_macos_completion_test.cpp");
  const std::string module =
      read_source("build/xmake/phase_j_macos_targets.lua");
  const std::string root = read_source("xmake.lua");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  if (self.empty() || behavior.empty() || module.empty() || root.empty() ||
      manifest.empty()) {
    return 1;
  }

  constexpr std::array required_paths{
      "scripts/ci/macos-dependencies.sh",
      "scripts/ci/macos-debug.sh",
      "scripts/ci/macos-example-smoke.sh",
      "scripts/ci/macos-package.sh",
      "scripts/ci/macos-phase-j-release.sh",
      "scripts/ci/macos-performance-baseline.sh",
      "scripts/ci/macos-stress-matrix.sh",
      "scripts/ci/macos-architecture-header.sh",
      "scripts/ci/run-with-github-annotation.sh",
      "tests/ci/run_with_github_annotation_test.sh",
      ".github/workflows/phase-j-desktop.yml"};
  for (const auto* path : required_paths) {
    if (read_source(path).empty()) return 10;
  }
  constexpr std::array target_minver_scripts{
      "scripts/ci/macos-debug.sh", "scripts/ci/macos-example-smoke.sh",
      "scripts/ci/macos-package.sh",
      "scripts/ci/macos-performance-baseline.sh",
      "scripts/ci/macos-stress-matrix.sh",
      "scripts/ci/macos-architecture-header.sh"};
  for (const auto* path : target_minver_scripts) {
    if (!contains(read_source(path),
                  "--target_minver=\"$MACOSX_DEPLOYMENT_TARGET\"")) return 15;
  }
  if (!contains(root,
                "includes(\"build/xmake/phase_j_macos_targets.lua\")") ||
      contains(root, "target(\"phase_j_macos_completion_test\")") ||
      !contains(module, "target(\"phase_j_macos_completion_test\")") ||
      !contains(
          module,
          "target(\"phase_j_macos_completion_structure_test\")")) {
    return 20;
  }
  if (!contains(
          manifest,
          "phase_j_macos_completion_structure_test|tests/architecture/"
          "phase_j_macos_completion_structure_test.cpp")) {
    return 30;
  }
  const std::string annotation =
      read_source("scripts/ci/run-with-github-annotation.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-j-desktop.yml");
  if (!contains(annotation, "CI command failed (exit %s)") ||
      count(workflow, "run-with-github-annotation.sh") != 6) {
    return 35;
  }
  if (line_count(module) > 30 || line_count(behavior) > 130 ||
      line_count(self) > 120 ||
      line_count(read_source("scripts/ci/macos-dependencies.sh")) > 150 ||
      line_count(read_source("scripts/ci/macos-package.sh")) > 230 ||
      line_count(read_source("scripts/ci/macos-phase-j-release.sh")) > 150 ||
      line_count(read_source("scripts/ci/macos-performance-baseline.sh")) >
          100 ||
      line_count(read_source("scripts/ci/macos-stress-matrix.sh")) > 100 ||
      line_count(read_source("scripts/ci/macos-architecture-header.sh")) >
          100 ||
      line_count(annotation) > 80 ||
      line_count(read_source("tests/ci/run_with_github_annotation_test.sh")) >
          80 ||
      line_count(workflow) > 360) {
    return 40;
  }
  return 0;
}
