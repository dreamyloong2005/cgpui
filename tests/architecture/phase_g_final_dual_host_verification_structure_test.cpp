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
  const std::string self = read_source(
      "tests/architecture/phase_g_final_dual_host_verification_structure_test.cpp");
  const std::string candidate =
      read_source("tests/api_parity/phase_g_candidate_ledger_closeout_test.cpp");
  const std::string windows = read_source(
      "tests/architecture/phase_g_windows_full_debug_verification_structure_test.cpp");
  const std::string wsl = read_source(
      "tests/architecture/phase_g_wsl_full_debug_verification_structure_test.cpp");
  const std::string dependency = read_source(
      "tests/architecture/phase_g_reproducible_dependency_setup_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &self,       &candidate, &windows, &wsl,       &dependency, &xmake,
      &manifest,   &roadmap,   &ledger_md, &ledger_json, &task_plan,  &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(xmake,
                "target(\"phase_g_final_dual_host_verification_structure_test\")") ||
      !contains(xmake,
                "tests/architecture/phase_g_final_dual_host_verification_structure_test.cpp") ||
      !contains(manifest,
                "phase_g_final_dual_host_verification_structure_test|"
                "tests/architecture/phase_g_final_dual_host_verification_structure_test.cpp")) {
    return 2;
  }
  if (!contains(candidate, "Phase G Step 676 closes the candidate ledger") ||
      !contains(candidate, "phase_g_step_676_remaining_gap") ||
      !contains(candidate, "Step 677 final Windows/WSL verification") ||
      !contains(windows, "Phase G Step 673 completes Windows full-debug") ||
      !contains(wsl, "Phase G Step 674 completes WSL full-debug")) {
    return 3;
  }
  if (!contains(xmake, "set_policy(\"package.requires_lock\", true)") ||
      !contains(dependency, "3.0.9") ||
      !contains(xmake, "target(\"wayland_frame_pixel_capture_test\")") ||
      !contains(xmake, "target(\"wayland_vulkan_surface_test\")")) {
    return 4;
  }

  constexpr const char* completion =
      "Phase G Step 677 completes final dual-host verification: Windows full "
      "debug passes 456/456 and WSL Arch Linux full debug passes 437/437 under "
      "Xmake 3.0.9, including real WSLg Wayland frame pixel capture and "
      "Wayland/Vulkan surface coverage. Step 678 Phase G final closeout "
      "verification is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 5;
  }
  if (!contains(ledger_json, "\"phase_g_step_677_sources\"") ||
      !contains(ledger_json,
                "\"phase_g_step_677_remaining_gap\": \"Step 678 Phase G "
                "final closeout verification\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase J Step 799 re-run "
                "upstream extractor against the pinned revision\"")) {
    return 6;
  }
  if (line_count(self) > 130 || line_count(xmake) > 4380) {
    return 7;
  }
  return 0;
}
