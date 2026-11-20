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
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string provenance =
      read_source("scripts/ci/sdk_package_provenance.py");
  const std::string cmake = read_source("scripts/ci/sdk_package_cmake.py");
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
      provenance.empty() || cmake.empty() ||
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
      !has(base, "sdk-package.py\") workspace") ||
      !has(base, "--cxx-runtime $cxxRuntime") ||
      !has(archiver, "\"--mode\"")) {
    return fail(3, "the SDK reuses the existing package and archive scripts");
  }
  if (!has(packer, "\"git_commit\"") || !has(packer, "\"dirty\": False") ||
      !has(packer, "\"vulkan_sdk_version\"") ||
      !has(packer, "\"system_packages\"") ||
      !has(packer, "\"system_libraries\"") ||
      !has(packer, "\"cxx_standard\"") || !has(packer, "\"sha256\"") ||
      !has(packer, "msvc_runtime(") || !has(packer, "example_workbench") ||
      !has(packer, "\"build_system\"") ||
      !has(packer, "copy_private_headers(")) {
    return fail(4, "the manifest records source, toolchain, links and hashes");
  }
  // Vendored dependency provenance comes from third_party/VENDORED.md and
  // the trees as committed at HEAD (git show HEAD:<path>), so the line
  // endings of a checkout cannot change a digest.
  if (!has(packer, "\"vendored_dependencies\": vendored_dependencies(") ||
      !has(provenance, "\"show\", f\"HEAD:{name}\"") ||
      !has(provenance, "\"tree_sha256\"") ||
      !has(provenance, "\"source_archive_sha256\"") ||
      !has(provenance, "\"upstream_commit\"") ||
      !has(provenance, "VENDORED.md")) {
    return fail(8, "dependency provenance comes from the committed tree");
  }
  if (!has(packer, "cgpuiConfig.cmake") ||
      !has(packer, "cgpuiConfigVersion.cmake") ||
      !has(cmake, "add_library(cgpui::cgpui INTERFACE IMPORTED)") ||
      !has(cmake, "INTERFACE_LINK_LIBRARIES") ||
      !has(sample_cmake, "cgpui::cgpui") ||
      !has(sample_xmake, "manifest.link_names")) {
    return fail(5, "consumers link through CMake and xmake");
  }
  // Tripwires: split the modules before raising these.
  if (line_count(packer) > 540 || line_count(provenance) > 160 ||
      line_count(cmake) > 170 || line_count(windows) > 140 ||
      line_count(linux_script) > 80) {
    return fail(6, "SDK packaging scripts exceed their line limits");
  }
  // Both Windows packages use /MD (mcpp has no debug CRT); the docs say what
  // a consumer must set and what is lost, and the test runs under ASan
  // instead (scripts/dev/asan_workspace.py, test.ps1 -Asan, test.sh --asan).
  const std::string asan = read_source("scripts/dev/asan_workspace.py");
  const std::string test_ps1 = read_source("scripts/dev/test.ps1");
  const std::string test_sh = read_source("scripts/dev/test.sh");
  if (!has(doc, "## C++ runtime (Windows)") ||
      !has(doc, "## Debug information (Windows)") ||
      !has(doc, "_ITERATOR_DEBUG_LEVEL=2") ||
      !has(cmake, "set_runtimes(\\\"MD\\\")") ||
      !has(cmake, "MultiThreadedDLL") || !has(asan, "/fsanitize=address") ||
      !has(asan, "-fsanitize=address,undefined") ||
      !has(asan, "-D_GLIBCXX_ASSERTIONS") || !has(asan, "CGPUI_ASAN=1") ||
      !has(test_ps1, "[switch]$Asan") ||
      !has(test_ps1, "Get-MsvcAsanRuntimeDir") ||
      !has(test_ps1, "asan_entry_windows.cpp") ||
      !has(asan, "main=cgpui_asan_main") ||
      !has(test_sh, "--asan")) {
    return fail(9, "the /MD CRT and the ASan replacement are documented");
  }
  // Windows ASan programs end without the DLL detach that hangs in exit.
  const std::string entry = read_source("scripts/dev/asan_entry_windows.cpp");
  const std::string started = read_source("docs/getting-started.md");
  if (!has(entry, "_cexit();") || !has(entry, "TerminateProcess(GetCurrentProcess(), ") ||
      !has(started, "asan_entry_windows.cpp")) {
    return fail(10, "the Windows ASan entry skips the DLL detach");
  }
  if (!has(matrix, "sdk_packaging_structure_test|")) {
    return fail(7, "the test must be in architecture-header-targets.txt");
  }
  return 0;
}
