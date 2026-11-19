// SDK packaging (C10, docs/sdk-packaging.md): the SDK wrappers reuse
// windows-package.ps1 / linux-package.sh and create-release-artifact.py,
// package only a clean committed tree, and the shared sdk-package.py step
// writes the manifest, the CMake config package and the README.
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" +
                       path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool has(const std::string& text, std::string_view value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t result = 0;
  for (const char value : text) {
    result += value == '\n' ? 1U : 0U;
  }
  return result;
}

int fail(int code, std::string_view message) {
  std::cerr << "sdk packaging structure: " << message << '\n';
  return code;
}

}  // namespace

int main() {
  const std::string windows = read_source("scripts/ci/windows-sdk-package.ps1");
  const std::string linux_script =
      read_source("scripts/ci/linux-sdk-package.sh");
  const std::string packer = read_source("scripts/ci/sdk-package.py");
  const std::string lock_reader =
      read_source("scripts/ci/sdk_package_lock.py");
  const std::string archiver =
      read_source("scripts/ci/create-release-artifact.py");
  const std::string base = read_source("scripts/ci/windows-package.ps1");
  const std::string doc = read_source("docs/sdk-packaging.md");
  const std::string sample_cmake =
      read_source("examples/sdk_consumer/CMakeLists.txt");
  const std::string sample_xmake = read_source("examples/sdk_consumer/xmake.lua");
  const std::string matrix =
      read_source("scripts/ci/architecture-header-targets.txt");
  if (windows.empty() || linux_script.empty() || packer.empty() ||
      doc.empty() || sample_cmake.empty() || sample_xmake.empty()) {
    return fail(1, "SDK scripts, sample and docs/sdk-packaging.md must exist");
  }
  if (!has(windows, "git -C $repoRoot worktree add --detach") ||
      !has(windows, "worktree remove --force") ||
      !has(windows, "check-clean --repo-root $worktree") ||
      !has(linux_script, "check-clean --repo-root \"$repo_root\"")) {
    return fail(2, "packages come from a clean committed tree");
  }
  if (!has(windows, "windows-package.ps1") || !has(windows, "-Runtimes") ||
      !has(linux_script, "linux-package.sh") ||
      !has(windows, "create-release-artifact.py") ||
      !has(linux_script, "create-release-artifact.py") ||
      !has(base, "--runtimes=$Runtimes") ||
      !has(archiver, "\"--mode\"")) {
    return fail(3, "the SDK reuses the existing package and archive scripts");
  }
  if (!has(packer, "\"git_commit\"") || !has(packer, "\"dirty\": False") ||
      !has(packer, "\"vulkan_sdk_version\"") ||
      !has(packer, "\"system_packages\"") ||
      !has(packer, "\"system_libraries\"") ||
      !has(packer, "\"cxx_standard\"") || !has(packer, "\"sha256\"") ||
      !has(packer, "msvc_runtime(") || !has(packer, "example_workbench") ||
      !has(packer, "copy_private_headers(")) {
    return fail(4, "the manifest records source, toolchain, links and hashes");
  }
  // Provenance comes from the committed lock only, so a package mirror in
  // the effective git or xmake configuration cannot leak in.
  if (!has(packer, "\"xmake_dependencies\": lock_provenance(") ||
      !has(lock_reader, "\"git\", \"-C\", str(repo), \"show\", f\"HEAD:{LOCK_FILE}\"") ||
      !has(lock_reader, "\"lock_sha256\"") ||
      !has(lock_reader, "\"package_hash\"") ||
      !has(lock_reader, "\"repo_url\"") ||
      !has(lock_reader, "\"repo_commit\"") ||
      has(lock_reader, "XMAKE_MAIN_REPO") || has(lock_reader, "config --get")) {
    return fail(8, "dependency provenance comes from the committed lock");
  }
  if (!has(packer, "cgpuiConfig.cmake") ||
      !has(packer, "cgpuiConfigVersion.cmake") ||
      !has(packer, "add_library(cgpui::cgpui INTERFACE IMPORTED)") ||
      !has(packer, "INTERFACE_LINK_LIBRARIES") ||
      !has(sample_cmake, "cgpui::cgpui") ||
      !has(sample_xmake, "manifest.link_names")) {
    return fail(5, "consumers link through CMake and xmake");
  }
  // Tripwires: split the modules before raising these.
  if (line_count(packer) > 540 || line_count(lock_reader) > 80 || line_count(windows) > 140 ||
      line_count(linux_script) > 80) {
    return fail(6, "SDK packaging scripts exceed their line limits");
  }
  if (!has(matrix, "sdk_packaging_structure_test|")) {
    return fail(7, "the test must be in architecture-header-targets.txt");
  }
  return 0;
}
