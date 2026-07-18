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
  const std::string module = read_source("build/xmake/phase_j_release_targets.lua");
  const std::string root = read_source("xmake.lua");
  const std::string manifest = read_source("scripts/ci/architecture-header-targets.txt");
  if (module.empty() || root.empty() || manifest.empty()) return 1;
  if (!contains(root, "includes(\"build/xmake/phase_j_release_targets.lua\")") ||
      !contains(module, "target(\"phase_j_release_audit_test\")") ||
      !contains(module, "target(\"phase_j_release_structure_test\")") ||
      !contains(manifest, "phase_j_release_structure_test")) return 2;
  const std::array paths{
      "scripts/ci/create-release-artifact.py",
      "scripts/ci/windows-phase-j-release.ps1",
      "scripts/ci/linux-phase-j-release.sh",
      ".github/actions/setup-phase-j-dependencies/action.yml",
      ".github/workflows/phase-j-windows-linux.yml"};
  for (const auto* path : paths) if (read_source(path).empty()) return 3;
  if (line_count(module) > 30 ||
      line_count(read_source("scripts/ci/create-release-artifact.py")) > 190 ||
      line_count(read_source("scripts/ci/windows-phase-j-release.ps1")) > 100 ||
      line_count(read_source("scripts/ci/linux-phase-j-release.sh")) > 100 ||
      line_count(read_source(".github/workflows/phase-j-windows-linux.yml")) > 260) return 4;
  const std::string macos_package = read_source("scripts/ci/macos-package.sh");
  if (macos_package.empty() || !contains(macos_package, "#!/usr/bin/env bash") ||
      !contains(macos_package, "MACOSX_DEPLOYMENT_TARGET") ||
      line_count(macos_package) > 230) return 5;
  const std::string macos_release =
      read_source("scripts/ci/macos-phase-j-release.sh");
  if (macos_release.empty() ||
      !contains(macos_release, "#!/usr/bin/env bash") ||
      !contains(macos_release, "macos-package.sh") ||
      !contains(macos_release, "create-release-artifact.py") ||
      line_count(macos_release) > 150) return 6;
  return 0;
}
