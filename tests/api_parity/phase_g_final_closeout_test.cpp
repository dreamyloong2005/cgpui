#include <algorithm>
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
  std::string text = source
                         ? std::string{std::istreambuf_iterator<char>(source), {}}
                         : std::string{};
  text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
  return text;
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
  const std::string manifests = read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &self,
      &predecessor,
      &candidate,
      &manifests,
      &ledger_json};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(manifests, "\"phase_g_final_closeout_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_final_closeout_test.cpp\"")) {
    return 2;
  }

  constexpr std::array closeout_targets{
      "phase_g_win32_uia_closeout_test.cpp",
      "phase_g_wayland_atspi_closeout_test.cpp",
      "phase_g_async_runtime_closeout_test.cpp",
      "phase_g_animation_examples_structure_test.cpp",
      "phase_g_image_gif_examples_structure_test.cpp",
      "phase_g_test_support_closeout_test.cpp",
      "phase_g_windows_debug_packaging_structure_test.cpp",
      "phase_g_linux_debug_packaging_structure_test.cpp",
      "phase_g_release_packaging_structure_test.cpp",
      "phase_g_example_smoke_matrix_structure_test.cpp",
      "phase_g_architecture_header_matrix_structure_test.cpp",
      "phase_g_reproducible_dependency_setup_structure_test.cpp",
      "phase_g_windows_full_debug_verification_structure_test.cpp",
      "phase_g_wsl_full_debug_verification_structure_test.cpp",
      "phase_g_action_macro_structure_test.cpp",
      "phase_g_candidate_ledger_closeout_test.cpp",
      "phase_g_final_dual_host_verification_structure_test.cpp",
  };
  for (std::size_t index = 0; index < closeout_targets.size(); ++index) {
    const std::string registration =
        std::string{"\""} + std::string{closeout_targets[index]} + "\"";
    if (!contains(manifests, registration)) return 10 + static_cast<int>(index);
  }
  if (!contains(predecessor, "wayland_frame_pixel_capture_test.cpp") ||
      !contains(predecessor, "wayland_vulkan_surface_test.cpp") ||
      !contains(candidate, "count(candidates, \"| Required |\") != 0") ||
      !contains(candidate, "count(ledger_json, \"\\\"upstream_gpui\\\":\") != 32")) {
    return 30;
  }

  const std::string platform_json = section_between(
      ledger_json, "\"platform_targets\": {", "\"status_counts\": {");
  const std::string status_json = section_between(
      ledger_json, "\"status_counts\": {", "\"scope_guard\": {");
  if (count(platform_json, "\"status\": \"required\"") != 4 ||
      count(platform_json, "\"status\": \"deferred\"") != 0 ||
      count(platform_json, "\"status\": \"non_goal\"") != 1 ||
      !contains(status_json, "\"required\": 0") ||
      !contains(status_json, "\"adapted\": 31") ||
      !contains(status_json, "\"deferred\": 0") ||
      !contains(status_json, "\"non_goal\": 1") ||
      count(ledger_json, "\"upstream_gpui\":") != 32) {
    return 50;
  }

  if (!contains(ledger_json, "\"phase_g_step_678_sources\"")) {
    return 62;
  }
  if (line_count(self) > 190 || line_count(read_source("mcpp.toml")) > 100) return 63;
  return 0;
}
