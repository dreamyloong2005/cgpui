#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>

namespace {
std::string read(const std::filesystem::path& path) {
  std::ifstream stream(path);
  return {std::istreambuf_iterator<char>(stream), {}};
}

int fail(const std::filesystem::path& path, const char* reason) {
  std::cerr << path.generic_string() << ": " << reason << '\n';
  return 1;
}
} // namespace

int main() {
  namespace fs = std::filesystem;
  const fs::path root{CGPUI_SOURCE_ROOT};
  // Assert maintainable test inputs, not the wording of a completed phase.
  const std::array historical_inputs{
      "\"task_plan.md\"", "\"findings.md\"", "\"progress.md\""};
  const std::regex phase_prose{"\"Phase [A-Z] Steps? [0-9]"};
  for (const auto* directory : {"tests/architecture", "tests/renderer"}) {
    for (const auto& entry : fs::recursive_directory_iterator(root / directory)) {
      const auto extension = entry.path().extension();
      if ((extension != ".cpp" && extension != ".mm") ||
          entry.path().filename() == "structural_assertion_policy_test.cpp") {
        continue;
      }
      const auto source = read(entry.path());
      if (source.empty()) return fail(entry.path(), "test source is unreadable");
      for (const auto* input : historical_inputs) {
        if (source.find(input) != std::string::npos) {
          return fail(entry.path(), "structure checks must not depend on work logs");
        }
      }
      if (std::regex_search(source, phase_prose) ||
          source.find("current_handoff") != std::string::npos) {
        return fail(entry.path(), "assert current contracts, not historical phase prose");
      }
    }
  }
  for (const auto* directory : {"docs/migration", "scripts/migration"}) {
    if (fs::exists(root / directory)) {
      return fail(directory, "retired build-system migration artifacts returned");
    }
  }
  const std::array active_documents{
      "README.md", "docs/architecture.md", "docs/examples.md",
      "docs/platform-dependencies.md", "docs/platform-mac-readiness.md",
      "scripts/ci/macos-example-smoke.sh"};
  for (const auto* path : active_documents) {
    const auto source = read(root / path);
    if (source.empty() || source.find("migration/README") != std::string::npos) {
      return fail(path, "active navigation references retired migration documentation");
    }
  }
  if (read(root / "docs/macos-validation.md").empty()) {
    return fail("docs/macos-validation.md", "native verification evidence is required");
  }
  return 0;
}
