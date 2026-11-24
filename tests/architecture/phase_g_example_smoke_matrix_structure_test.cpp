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
  const std::string targets = read_source("scripts/ci/example-targets.txt");
  const std::string windows =
      read_source("scripts/ci/windows-example-smoke.ps1");
  const std::string linux = read_source("scripts/ci/linux-example-smoke.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-g-windows-linux.yml");
  const std::string manifests = read_source("tests/architecture/mcpp.toml");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &targets, &windows, &linux, &workflow, &manifests,
      &ledger_json};
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
      contains(windows, "-j 1") ||
      !contains(windows, "& $mcpp run -p examples/api_parity") ||
      !contains(windows, "& $mcpp test -p examples/hello_window") ||
      !contains(windows, "windows_demo_smoke_flow")) return 3;

  if (!contains(linux, "linux-package.sh") ||
      !contains(linux, "CGPUI_CI_REUSE_PREPARED_ROOT") ||
      !contains(linux, "linux-dependencies.sh") ||
      !contains(linux, "example-targets.txt") ||
      !contains(linux, "weston") || contains(linux, "-j 1") ||
      !contains(linux, "\"$MCPP\" run") ||
      !contains(linux, "\"$MCPP\" test") ||
      !contains(linux, "linux_demo_smoke_flow")) return 4;

  if (!contains(workflow, "windows-examples-smoke:") ||
      !contains(workflow, "scripts/ci/windows-example-smoke.ps1") ||
      !contains(workflow, "linux-examples-smoke:") ||
      !contains(workflow, "scripts/ci/linux-example-smoke.sh") ||
      !contains(workflow, "weston") ||
      !contains(workflow, "mesa-vulkan-drivers")) return 5;

  const auto windows_job = workflow.find("  windows-examples-smoke:");
  const auto linux_job = workflow.find("  linux-examples-smoke:", windows_job);
  if (linux_job == std::string::npos ||
      !contains(workflow.substr(windows_job, linux_job - windows_job), "CGPUI_RENDERER: vulkan") ||
      contains(windows, "$env:CGPUI_RENDERER =")) return 10;

  if (!contains(manifests,
                "\"phase_g_example_smoke_matrix_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(targets) > 30 || line_count(windows) > 150 ||
      line_count(linux) > 180 || line_count(workflow) > 210 ||
      line_count(read_source("mcpp.toml")) > 100) return 7;

  if (!contains(ledger_json, "\"phase_g_step_670_sources\"")) {
    return 9;
  }
  return 0;
}
