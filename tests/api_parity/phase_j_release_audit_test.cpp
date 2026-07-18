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
}  // namespace

int main() {
  const std::string policy = read_source("docs/gpui-release-artifact-policy.json");
  const std::string packer = read_source("scripts/ci/create-release-artifact.py");
  const std::string windows = read_source("scripts/ci/windows-phase-j-release.ps1");
  const std::string linux = read_source("scripts/ci/linux-phase-j-release.sh");
  const std::string linux_package = read_source("scripts/ci/linux-package.sh");
  const std::string macos_package = read_source("scripts/ci/macos-package.sh");
  const std::string bootstrap =
      read_source(".github/actions/setup-phase-j-dependencies/action.yml");
  const std::string workflow = read_source(".github/workflows/phase-j-windows-linux.yml");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger = read_source("docs/gpui-complete-parity-ledger.json");
  if (policy.empty() || packer.empty() || windows.empty() || linux.empty() ||
      linux_package.empty() || bootstrap.empty() || workflow.empty() ||
      roadmap.empty() || ledger.empty()) return 1;
  const std::array linux_libraries{
      "libcgpui_platform_linux_wayland.a", "libcgpui_platform_linux_x11.a",
      "libcgpui_platform_linux_atspi.a"};
  for (const auto* library : linux_libraries)
    if (!contains(linux_package, library)) return 2;
  if (!contains(packer, "release-manifest.json") ||
      !contains(packer, "sha256") || !contains(packer, "SOURCE_DATE_EPOCH") ||
      !contains(packer, "--archive-name") || !contains(packer, "ZIP_STORED") ||
      !contains(packer, "gzip.GzipFile")) return 3;
  if (!contains(windows, "Compare-Object") ||
      !contains(windows, "windows-package.ps1") ||
      !contains(linux, "cmp -s") || !contains(linux, "linux-package.sh")) return 4;
  if (!contains(bootstrap, "xmake-version: 3.0.9") ||
      !contains(bootstrap, "hashFiles('xmake.lua', 'xmake-requires.lock')") ||
      !contains(bootstrap, "build/phase-j-ci/dependencies")) return 5;
  if (!contains(workflow, "windows-release") ||
      !contains(workflow, "linux-release") ||
      !contains(workflow, "windows-performance") ||
      !contains(workflow, "linux-wayland-performance") ||
      !contains(workflow, "linux-x11-performance") ||
      !contains(workflow, "linux-wayland-stress") ||
      !contains(workflow, "linux-x11-stress") ||
      !contains(workflow, "architecture-header")) return 6;
  if (!contains(policy, "2024-01-01T00:00:00Z") ||
      !contains(policy, "cgpui-windows-release.zip") ||
      !contains(policy, "cgpui-linux-release.tar.gz")) return 7;
  if (!contains(roadmap, "- [x] Steps 829-834: Package Windows/Linux builds") ||
      !contains(ledger, "\"phase_j_steps_829_834_status\": \"complete_non_macos\"") ||
      !contains(ledger, "Phase J Steps 841+ upstream drift delta plan")) return 8;
  constexpr std::array macos_libraries{
      "libcgpui_core.a", "libcgpui_platform.a", "libcgpui_platform_macos.a",
      "libcgpui_renderer.a", "libcgpui_renderer_metal.a", "libcgpui_ui.a",
      "libcgpui_app.a"};
  for (const auto* library : macos_libraries)
    if (!contains(macos_package, library)) return 9;
  if (!contains(macos_package, "hello_window") ||
      !contains(macos_package, "deployment_target") ||
      !contains(macos_package, "metadata/build.json") ||
      !contains(macos_package, "metadata/xmake-requires.lock") ||
      !contains(macos_package, "arm64|x86_64")) return 9;
  return 0;
}
