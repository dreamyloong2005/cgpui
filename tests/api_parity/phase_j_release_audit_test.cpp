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
  // The release toolchain is pinned: the mcpp release (version and digest)
  // and the vendored dependency trees.
  if (!contains(bootstrap, "uses: ./.github/actions/setup-mcpp") ||
      !contains(read_source(".github/actions/setup-mcpp/action.yml"),
                "version=2026.11.18.1") ||
      !contains(policy, "\"dependency_provenance\": \"third_party/VENDORED.md\"") ||
      !contains(policy, "\"mcpp_version\": \"2026.11.18.1\"")) return 5;
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
  return 0;
}
