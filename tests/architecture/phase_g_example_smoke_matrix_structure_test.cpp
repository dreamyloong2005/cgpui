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
  const std::string targets = read_source("scripts/ci/example-targets.txt");
  const std::string windows =
      read_source("scripts/ci/windows-example-smoke.ps1");
  const std::string linux = read_source("scripts/ci/linux-example-smoke.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-g-windows-linux.yml");
  const std::string previous = read_source(
      "tests/architecture/phase_g_release_packaging_structure_test.cpp");
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
      &targets, &windows, &linux, &workflow, &previous, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  const char* examples[]{
      "api_parity_hello_world", "api_parity_public_api_compatibility",
      "api_parity_public_authoring_workflow",
      "api_parity_public_svg_image_sources",
      "api_parity_public_svg_raster_upload",
      "api_parity_public_context_capabilities",
      "api_parity_public_async_test_workflow",
      "api_parity_public_phase_b_surface_closure",
      "api_parity_public_window_examples",
      "api_parity_public_window_examples_workflow",
      "api_parity_public_text_input_examples",
      "api_parity_public_text_wrapper_examples",
      "api_parity_public_text_input_workflow",
      "api_parity_public_rich_text_examples",
      "api_parity_public_window_examples_widget_catalog",
      "api_parity_public_window_examples_interaction_states",
      "api_parity_public_window_examples_service_matrix",
      "api_parity_public_animation_example",
      "api_parity_public_opacity_example",
      "api_parity_public_image_example",
      "api_parity_public_gif_viewer_example"};
  for (const char* example : examples) {
    if (!contains(targets, example)) return 2;
  }

  if (!contains(windows, "windows-package.ps1") ||
      !contains(windows, "-Mode Debug") ||
      !contains(windows, "ReusePreparedRoot") ||
      !contains(windows, "example-targets.txt") ||
      !contains(windows, "xmake build") || !contains(windows, "-j 1") ||
      !contains(windows, "xmake run") || !contains(windows, "xmake test") ||
      !contains(windows, "windows_demo_smoke_flow")) return 3;

  if (!contains(linux, "linux-package.sh") ||
      !contains(linux, "CGPUI_CI_REUSE_PREPARED_ROOT") ||
      !contains(linux, "python-tools") ||
      !contains(linux, "example-targets.txt") ||
      !contains(linux, "weston") || !contains(linux, "xmake build") ||
      !contains(linux, "-j 1") || !contains(linux, "xmake run") ||
      !contains(linux, "xmake test") ||
      !contains(linux, "linux_demo_smoke_flow")) return 4;

  if (!contains(workflow, "windows-examples-smoke:") ||
      !contains(workflow, "scripts/ci/windows-example-smoke.ps1") ||
      !contains(workflow, "linux-examples-smoke:") ||
      !contains(workflow, "scripts/ci/linux-example-smoke.sh") ||
      !contains(workflow, "weston") ||
      !contains(workflow, "mesa-vulkan-drivers")) return 5;

  if (!contains(previous, "Phase G Step 669 adds Windows and Linux Release") ||
      !contains(xmake,
                "target(\"phase_g_example_smoke_matrix_structure_test\")")) {
    return 6;
  }
  if (line_count(targets) > 30 || line_count(windows) > 150 ||
      line_count(linux) > 180 || line_count(workflow) > 210 ||
      line_count(xmake) > 4360) return 7;

  constexpr const char* completion =
      "Phase G Step 670 adds Windows and Linux CI example/smoke matrices "
      "that serially build every public API example, run their noninteractive "
      "entry points, execute animation, opacity, image, and GIF registration "
      "smoke coverage, and pass first-frame, resize, close, and interaction "
      "smoke flows on platform display backends. Step 671 architecture and "
      "header test matrix coverage is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json, "\"phase_g_step_670_sources\"") ||
      !contains(ledger_json,
          "\"phase_f_current_handoff\": \"Step 671 architecture and header test matrix coverage\"")) {
    return 9;
  }
  return 0;
}
