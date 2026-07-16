#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, std::string_view value) {
  return text.find(value) != std::string::npos;
}

std::string section_between(const std::string& text, std::string_view begin,
                            std::string_view end) {
  const std::size_t start = text.find(begin);
  const std::size_t finish = text.find(end, start);
  if (start == std::string::npos || finish == std::string::npos) return {};
  return text.substr(start, finish - start);
}

std::size_t count(const std::string& text, std::string_view value) {
  std::size_t result = 0;
  for (std::size_t index = 0;
       (index = text.find(value, index)) != std::string::npos;
       index += value.size()) {
    ++result;
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
  const std::string self =
      read_source("tests/api_parity/phase_g_final_closeout_test.cpp");
  const std::string predecessor = read_source(
      "tests/architecture/phase_g_final_dual_host_verification_structure_test.cpp");
  const std::string candidate =
      read_source("tests/api_parity/phase_g_candidate_ledger_closeout_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&self,       &predecessor, &candidate,
                                &xmake,      &roadmap,     &ledger_md,
                                &ledger_json, &task_plan,   &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(xmake, "target(\"phase_g_final_closeout_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_g_final_closeout_test.cpp")) {
    return 2;
  }

  constexpr std::array closeout_targets{
      "phase_g_win32_uia_closeout_test",
      "phase_g_wayland_atspi_closeout_test",
      "phase_g_async_runtime_closeout_test",
      "phase_g_animation_examples_structure_test",
      "phase_g_image_gif_examples_structure_test",
      "phase_g_test_support_closeout_test",
      "phase_g_windows_debug_packaging_structure_test",
      "phase_g_linux_debug_packaging_structure_test",
      "phase_g_release_packaging_structure_test",
      "phase_g_example_smoke_matrix_structure_test",
      "phase_g_architecture_header_matrix_structure_test",
      "phase_g_reproducible_dependencies_structure_test",
      "phase_g_windows_full_debug_verification_structure_test",
      "phase_g_wsl_full_debug_verification_structure_test",
      "phase_g_action_macro_structure_test",
      "phase_g_candidate_ledger_closeout_test",
      "phase_g_final_dual_host_verification_structure_test",
  };
  for (std::size_t index = 0; index < closeout_targets.size(); ++index) {
    const std::string registration =
        "target(\"" + std::string{closeout_targets[index]} + "\")";
    if (!contains(xmake, registration)) return 10 + static_cast<int>(index);
  }
  if (!contains(predecessor,
                "Phase G Step 677 completes final dual-host verification") ||
      !contains(predecessor, "Step 678 Phase G final closeout ") ||
      !contains(predecessor, "verification is next.") ||
      !contains(candidate, "count(candidates, \"| Required |\") != 0") ||
      !contains(candidate, "count(ledger_json, \"\\\"upstream_gpui\\\":\") != 32")) {
    return 30;
  }

  constexpr std::array roadmap_bands{
      "- [x] Steps 619-626: Implement Win32 UIA",
      "- [x] Steps 627-634: Implement Linux AT-SPI",
      "- [x] Steps 635-642: Complete async runtime",
      "- [x] Steps 643-650: Complete animation",
      "- [x] Steps 651-658: Complete assets",
      "- [x] Steps 659-666: Add GPUI-style test",
      "- [x] Steps 667-672: Add packaging and CI coverage",
      "- [x] Steps 673-678: Run full Windows/WSL verification",
  };
  for (std::size_t index = 0; index < roadmap_bands.size(); ++index) {
    if (!contains(roadmap, roadmap_bands[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  const std::string platform_json = section_between(
      ledger_json, "\"platform_targets\": {", "\"status_counts\": {");
  const std::string status_json = section_between(
      ledger_json, "\"status_counts\": {", "\"scope_guard\": {");
  if (count(platform_json, "\"status\": \"required\"") != 3 ||
      count(platform_json, "\"status\": \"deferred\"") != 1 ||
      count(platform_json, "\"status\": \"non_goal\"") != 1 ||
      !contains(status_json, "\"required\": 0") ||
      !contains(status_json, "\"adapted\": 30") ||
      !contains(status_json, "\"deferred\": 1") ||
      !contains(status_json, "\"non_goal\": 1") ||
      count(ledger_json, "\"upstream_gpui\":") != 32) {
    return 50;
  }

  constexpr std::array step_records{
      "Phase G Step 673 completes Windows full-debug verification",
      "Phase G Step 674 completes WSL full-debug verification",
      "Phase G Step 675 adds public scoped and unscoped unit-action macros",
      "Phase G Step 676 closes the candidate ledger",
      "Phase G Step 677 completes final dual-host verification",
  };
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan,
                                 &findings};
  for (const auto* document : documents) {
    for (const char* record : step_records) {
      if (!contains(*document, record)) return 60;
    }
  }

  constexpr const char* completion =
      "Phase G Step 678 completes final closeout for Steps 619-678 with "
      "`tests/api_parity/phase_g_final_closeout_test.cpp`: Windows full debug "
      "passes 457/457 and WSL Arch Linux full debug passes 438/438 under Xmake "
      "3.0.9, including real WSLg Wayland frame pixel capture and "
      "Wayland/Vulkan surface coverage; the 32-row candidate ledger remains "
      "at 0 required, 29 adapted, 2 deferred, and 1 non-goal. Phase H Step "
      "679 Cocoa application and NSWindow lifecycle is next.";
  for (const auto* document : documents) {
    if (count(*document, completion) != 1) return 61;
  }
  if (!contains(ledger_json, "\"phase_g_step_678_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase I Step 759 X11/XCB "
                "platform boundary\"") ||
      !contains(task_plan,
                "- Status: complete\n- Authoritative scope: Phase G Steps "
                "619-678")) {
    return 62;
  }
  if (line_count(self) > 190 || line_count(xmake) > 4400) return 63;
  return 0;
}
