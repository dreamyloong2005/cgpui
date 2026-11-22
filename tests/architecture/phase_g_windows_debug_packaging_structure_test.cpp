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

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string wrapper = read_source("scripts/ci/windows-debug.ps1");
  const std::string core = read_source("scripts/ci/windows-package.ps1");
  const std::string workflow =
      read_source(".github/workflows/phase-g-windows-linux.yml");
  const std::string manifests = read_source("tests/architecture/mcpp.toml");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &wrapper, &core, &workflow, &manifests, &ledger_json};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(wrapper, "windows-package.ps1") ||
      !contains(wrapper, "-Mode Debug") ||
      !contains(core, "[System.IO.Path]::GetFullPath") ||
      !contains(core, "Remove-Item -LiteralPath $outputRoot") ||
      !contains(core, "windows-dependencies.ps1") ||
      !contains(core, "sdk-package.py\") workspace") ||
      !contains(core, "& $mcpp build --profile $profileName") ||
      contains(core, "-j 1") ||
      !contains(core, "hello_window")) return 2;

  const char* artifacts[]{
      "cgpui_core.lib", "cgpui_platform.lib", "cgpui_platform_win32.lib",
      "cgpui_renderer.lib", "cgpui_renderer_vulkan.lib", "cgpui_ui.lib",
      "cgpui_app.lib", "hello_window.exe", "include/cgpui", "README.md",
      "manifest.json"};
  for (const char* artifact : artifacts) {
    if (!contains(core, artifact)) return 3;
  }
  if (!contains(core, "ConvertTo-Json") ||
      !contains(core, "schema_version") ||
      !contains(core, "cgpui-windows-$modeName") ||
      !contains(workflow, "runs-on: windows-latest") ||
      !contains(workflow, "scripts/ci/windows-debug.ps1") ||
      !contains(workflow, "actions/upload-artifact@v4") ||
      !contains(workflow, "build/phase-g-ci/windows-debug/package")) {
    return 4;
  }

  if (!contains(manifests,
                "\"phase_g_windows_debug_packaging_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(wrapper) > 35 || line_count(core) > 190 ||
      line_count(workflow) > 150 ||
      line_count(read_source("mcpp.toml")) > 100) return 6;

  if (!contains(ledger_json, "\"phase_g_step_667_sources\"") ||
      !contains(ledger_json, "scripts/ci/windows-package.ps1")) {
    return 8;
  }
  return 0;
}
