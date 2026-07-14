#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
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
  const std::string wrapper = read_source("scripts/ci/linux-debug.sh");
  const std::string core = read_source("scripts/ci/linux-package.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-g-windows-linux.yml");
  const std::string previous = read_source(
      "tests/architecture/phase_g_windows_debug_packaging_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &wrapper, &core, &workflow, &previous, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(wrapper, "linux-package.sh") ||
      !contains(wrapper, "debug") ||
      !contains(core, "set -euo pipefail") ||
      !contains(core, "realpath -m") ||
      !contains(core, "rm -rf -- \"$output_root\"") ||
      !contains(core, "CGPUI_CI_TMPDIR") ||
      !contains(core, "mktemp -d") ||
      !contains(core, "linux-dependencies.sh") ||
      !contains(core, "xmake f") ||
      !contains(core, "--ccache=n") ||
      !contains(core, "xmake build") ||
      !contains(core, "-j 1") ||
      !contains(core, "hello_window")) return 2;

  const char* artifacts[]{
      "libcgpui_core.a", "libcgpui_platform.a",
      "libcgpui_platform_linux_wayland.a", "libcgpui_renderer.a",
      "libcgpui_renderer_vulkan.a", "libcgpui_ui.a", "libcgpui_app.a",
      "hello_window", "include/cgpui", "README.md", "manifest.json"};
  for (const char* artifact : artifacts) {
    if (!contains(core, artifact)) return 3;
  }
  if (!contains(core, "python3") ||
      !contains(core, "schema_version") ||
      !contains(core, "cgpui-linux-{mode}") ||
      !contains(workflow, "linux-debug:") ||
      !contains(workflow, "runs-on: ubuntu-latest") ||
      !contains(workflow, "scripts/ci/linux-debug.sh") ||
      !contains(workflow, "actions/upload-artifact@v4") ||
      !contains(workflow, "build/phase-g-ci/linux-debug/package")) {
    return 4;
  }

  if (!contains(previous,
                "Phase G Step 667 adds a Windows Debug CI packaging path") ||
      !contains(xmake,
                "target(\"phase_g_linux_debug_packaging_structure_test\")") ||
      !contains(roadmap,
                "- [x] Phase G Step 668 adds a Linux Debug CI packaging")) {
    return 5;
  }
  if (line_count(wrapper) > 20 || line_count(core) > 190 ||
      line_count(workflow) > 150 ||
      line_count(xmake) > 4320) return 6;

  constexpr const char* completion =
      "Phase G Step 668 adds a Linux Debug CI packaging path with "
      "workspace-confined output cleanup, serial Xmake configuration and "
      "build, public headers, framework libraries, demo executable, README, "
      "manifest validation, and uploaded artifact coverage. Step 669 Windows "
      "and Linux release build and packaging coverage is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(ledger_json, "\"phase_g_step_668_sources\"") ||
      !contains(ledger_json, "scripts/ci/linux-package.sh") ||
      !contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 675 action macro production behavior\"")) {
    return 8;
  }
  return 0;
}
