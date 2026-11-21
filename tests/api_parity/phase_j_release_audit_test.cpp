#include <array>
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
}  // namespace

int main() {
  const std::string policy = read_source("docs/gpui-release-artifact-policy.json");
  const std::string packer = read_source("scripts/ci/create-release-artifact.py");
  const std::string windows = read_source("scripts/ci/windows-phase-j-release.ps1");
  const std::string linux = read_source("scripts/ci/linux-phase-j-release.sh");
  const std::string linux_package = read_source("scripts/ci/linux-package.sh");
  const std::string macos_package = read_source("scripts/ci/macos-package.sh");
  const std::string macos_release =
      read_source("scripts/ci/macos-phase-j-release.sh");
  const std::string bootstrap =
      read_source(".github/actions/setup-phase-j-dependencies/action.yml");
  const std::string workflow = read_source(".github/workflows/phase-j-desktop.yml");
  const std::string old_workflow =
      read_source(".github/workflows/phase-j-windows-linux.yml");
  const std::string root_manifest = read_source("mcpp.toml");
  const std::string macos_debug = read_source("scripts/ci/macos-debug.sh");
  const std::string macos_architecture =
      read_source("scripts/ci/macos-architecture-header.sh");
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
  // The release toolchain is pinned: the mcpp release (version and digest)
  // and the vendored dependency trees.
  if (!contains(bootstrap, "uses: ./.github/actions/setup-mcpp") ||
      !contains(read_source(".github/actions/setup-mcpp/action.yml"),
                "version=2026.11.20.1") ||
      !contains(policy, "\"dependency_provenance\": \"third_party/VENDORED.md\"") ||
      !contains(policy, "\"mcpp_version\": \"2026.11.20.1\"") ||
      !contains(bootstrap, "build/phase-j-ci/dependencies")) return 5;
  // Phase J runs only macOS arm64 jobs here. Non-macOS jobs stay separate.
  constexpr std::array retired_non_macos_jobs{
      "  windows-release:\n", "  linux-release:\n",
      "  windows-performance:\n", "  linux-wayland-performance:\n",
      "  linux-x11-performance:\n", "  linux-wayland-stress:\n",
      "  linux-x11-stress:\n", "  windows-examples:\n",
      "  linux-wayland-examples:\n", "  linux-x11-examples:\n",
      "  architecture-header:\n"};
  if (!contains(workflow, "name: Phase J macOS Completion")) return 6;
  for (const auto* job : retired_non_macos_jobs)
    if (contains(workflow, job)) return 6;
  if (!contains(policy, "2024-01-01T00:00:00Z") ||
      !contains(policy, "cgpui-windows-release.zip") ||
      !contains(policy, "cgpui-linux-release.tar.gz")) return 7;
  if (!contains(roadmap, "- [x] Steps 829-834: Package Windows/Linux builds") ||
      !contains(ledger, "\"phase_j_steps_829_834_status\": \"complete\"") ||
      !contains(ledger, "Phase J Steps 841+ upstream drift delta plan")) return 8;
  // The library list comes from the workbench's native dependency graph;
  // verifying the extracted consumer exercises the public header and link closure.
  if (!contains(macos_package, "sdk-package.py\" stage-macos") ||
      !contains(macos_release, "macos-sdk-consumer.py") ||
      !contains(macos_package, "hello_window") ||
      !contains(macos_package, "deployment_target") ||
      !contains(macos_package, "metadata/build.json") ||
      !contains(macos_package, "metadata/mcpp.toml") ||
      !contains(macos_package, "cgpui_require_macos_arm64")) return 9;
  if (!contains(packer, "\"macos\"") ||
      !contains(packer, "architecture") ||
      !contains(packer, "deployment_target") ||
      !contains(policy, "cgpui-macos-arm64-release.tar.gz") ||
      contains(policy, "cgpui-macos-x86_64-release.tar.gz") ||
      !contains(policy, "\"macos_deployment_target\": \"13.0\"") ||
      !contains(macos_release, "macos-package.sh") ||
      !contains(macos_release, "create-release-artifact.py") ||
      !contains(macos_release, "cmp -s")) return 10;
  constexpr std::array macos_jobs{
      "macos-debug-examples", "macos-performance-stress", "macos-release",
      "macos-architecture-header"};
  for (const auto* job : macos_jobs)
    if (!contains(workflow, job)) return 11;
  if (!old_workflow.empty() ||
      !contains(workflow, "runs-on: macos-15\n") ||
      contains(workflow, "macos-15-intel") ||
      !contains(workflow, "CGPUI_EXPECTED_ARCH: arm64") ||
      !contains(workflow, "MACOSX_DEPLOYMENT_TARGET: \"13.0\"") ||
      !contains(workflow, "cgpui-macos-arm64-release") ||
      !contains(bootstrap, "Configure the macOS mcpp environment") ||
      !contains(bootstrap, "macos-dependencies.sh") ||
      !contains(root_manifest, "\"packages/platform-macos\",") ||
      !contains(macos_debug, "macos-dependencies.sh") ||
      !contains(macos_architecture, "architecture-header-targets.txt")) return 11;
  return 0;
}
