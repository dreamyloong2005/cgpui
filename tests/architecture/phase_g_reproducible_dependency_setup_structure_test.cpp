#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t count(const std::string& text, const char* value) {
  std::size_t result = 0;
  std::size_t offset = 0;
  while ((offset = text.find(value, offset)) != std::string::npos) {
    ++result;
    offset += std::char_traits<char>::length(value);
  }
  return result;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string windows_dependencies =
      read_source("scripts/ci/windows-dependencies.ps1");
  const std::string linux_dependencies =
      read_source("scripts/ci/linux-dependencies.sh");
  const std::string action =
      read_source(".github/actions/setup-phase-g-dependencies/action.yml");
  const std::string lock = read_source("third_party/VENDORED.md");
  const std::string windows_package =
      read_source("scripts/ci/windows-package.ps1");
  const std::string linux_package = read_source("scripts/ci/linux-package.sh");
  const std::string windows_examples =
      read_source("scripts/ci/windows-example-smoke.ps1");
  const std::string linux_examples =
      read_source("scripts/ci/linux-example-smoke.sh");
  const std::string windows_matrix =
      read_source("scripts/ci/windows-architecture-header.ps1");
  const std::string linux_matrix =
      read_source("scripts/ci/linux-architecture-header.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-g-windows-linux.yml");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string manifests = read_source("third_party/VENDORED.md") +
      read_source("tests/architecture/mcpp.toml");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &windows_dependencies, &linux_dependencies, &action, &lock,
      &windows_package, &linux_package, &windows_examples, &linux_examples,
      &windows_matrix, &linux_matrix, &workflow, &manifest, &manifests,
      &ledger_json};
  for (const auto* source : required) if (source->empty()) return 1;

  // Dependencies are pinned in the repository: lunasvg / plutovg are
  // vendored at the versions the xmake lock pinned, with the archive
  // digest recorded; mcpp itself is pinned (version and digest) in CI.
  if (!contains(manifests, "Source archive SHA-256") ||
      !contains(lock, "| v3.5.0 | `83c58df8103dc7dca423dfd824992af94d49bed6`") ||
      !contains(lock, "| v1.3.3 | `bbd91f0d06a71491691b36330f29dffa4af87ccf`") ||
      !contains(lock, "1abf1472ee6c4d19797916e8cc3c2e4b628e0d81178ffac60bdb0d457e32c690") ||
      !contains(lock, "2b0d17a6e016f47b86f9c00e2cb82600041b1ea1f7d2a00c2d46ae542cbfed3c") ||
      !contains(read_source("third_party/lunasvg/mcpp.toml"), "version = \"3.5.0\"") ||
      !contains(read_source("third_party/plutovg/mcpp.toml"), "version = \"1.3.3\"")) return 2;
  if (!contains(read_source("mcpp.toml"), "mcpp = \">=2026.11.21.1\"")) return 2;

  if (!contains(windows_dependencies, "mcpp-env.ps1") ||
      !contains(windows_dependencies, "MCPP_HOME") ||
      !contains(windows_dependencies, "VULKAN_SDK") ||
      !contains(windows_dependencies, "GITHUB_ENV")) return 3;
  if (!contains(linux_dependencies, "mcpp-env.sh") ||
      !contains(linux_dependencies, "MCPP_HOME") ||
      !contains(linux_dependencies, "pkg-config --exists") ||
      !contains(linux_dependencies, "GITHUB_ENV")) return 4;

  const std::string* windows_consumers[]{
      &windows_package, &windows_examples, &windows_matrix};
  for (const auto* consumer : windows_consumers) {
    if (!contains(*consumer, "windows-dependencies.ps1")) return 5;
  }
  const std::string* linux_consumers[]{
      &linux_package, &linux_examples, &linux_matrix};
  for (const auto* consumer : linux_consumers) {
    if (!contains(*consumer, "linux-dependencies.sh")) return 6;
  }

  const std::string setup_mcpp = read_source(".github/actions/setup-mcpp/action.yml");
  if (!contains(action, "uses: ./.github/actions/setup-mcpp") ||
      !contains(setup_mcpp, "version=2026.11.21.1") ||
      !contains(setup_mcpp, "a39c91c15f082458210e76874ed6ee88b3402c357ac723fb2faa12c2b82bcff4") ||
      !contains(setup_mcpp, "bafefffb6651b47e3e1be0c85f12df091a9e60d8f635537c6812295dc2327ac2") ||
      !contains(setup_mcpp, "9d535e4f89dc63f58d122527b4441f46c52fe6b4d5b386f3672494b10c55a35d") ||
      // Release tags are v<version>; without the v every download is a 404.
      count(setup_mcpp, "releases/download/v$version/") != 3 ||
      contains(setup_mcpp, "releases/download/$version/") ||
      !contains(action, "windows-dependencies.ps1") ||
      !contains(action, "linux-dependencies.sh") ||
      count(workflow, "uses: ./.github/actions/setup-phase-g-dependencies") != 8) return 7;

  // A fresh hosted runner must have the native dependencies *before* the
  // environment checks run. Installing them later in each job is too late.
  const auto before = [&](const char* first, const char* second) {
    const auto end = action.find(second);
    return end != std::string::npos && action.find(first) < end;
  };
  if (!before("sudo apt-get install", "linux-dependencies.sh --github-env") ||
      !before("Get-FileHash", "Start-Process") ||
      !before("Start-Process", "VULKAN_SDK=$sdk") ||
      !before("VULKAN_SDK=$sdk", ". ./scripts/ci/windows-dependencies.ps1")) return 12;
  const char* linux_packages[]{
      "pkg-config", "libwayland-dev", "libdbus-1-dev", "libxkbcommon-dev",
      "libxkbcommon-x11-dev", "libxcb1-dev", "libxcb-xkb-dev", "libxcb-cursor-dev",
      "libharfbuzz-dev", "libfreetype-dev", "libfontconfig1-dev", "libvulkan-dev",
      "meson ninja-build python3-venv"};
  for (const auto* package : linux_packages)
    if (!contains(action, package)) return 12;

  if (!contains(manifest,
          "phase_g_reproducible_dependency_setup_structure_test|") ||
      !contains(manifests,
          "\"phase_g_reproducible_dependency_setup_structure_test.cpp\"")) {
    return 8;
  }
  if (line_count(windows_dependencies) > 100 ||
      line_count(linux_dependencies) > 100 || line_count(action) > 80 ||
      line_count(workflow) > 155 || line_count(read_source("mcpp.toml")) > 100) return 9;

  if (!contains(ledger_json, "\"phase_g_step_672_sources\"")) {
    return 11;
  }
  return 0;
}
