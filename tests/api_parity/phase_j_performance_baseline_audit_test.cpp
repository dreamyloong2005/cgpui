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

}  // namespace

int main() {
  const std::string runner =
      read_source("tests/performance/phase_j_performance_runner.cpp");
  const std::string policy =
      read_source("docs/gpui-performance-baseline-policy.json");
  const std::string validator =
      read_source("tools/gpui_parity/validate_performance_baseline.py");
  const std::string windows =
      read_source("scripts/ci/windows-performance-baseline.ps1");
  const std::string linux =
      read_source("scripts/ci/linux-performance-baseline.sh");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &runner, &policy, &validator, &windows, &linux, &roadmap, &ledger};
  for (const auto* source : required) if (source->empty()) return 1;

  constexpr std::array metrics{
      "startup", "first_frame", "resize", "text_layout",
      "list_scrolling", "image_loading", "async_wakeup", "frame_pacing"};
  for (const auto* metric : metrics) {
    if (!contains(runner, metric) || !contains(policy, metric)) return 2;
  }
  if (!contains(runner, "cgpui::TestApp") ||
      !contains(runner, "cgpui::measure_text") ||
      !contains(runner, "calculate_uniform_list_visible_range") ||
      !contains(runner, "cgpui::decode_image") ||
      !contains(runner, "start_animation")) return 3;
  if (!contains(validator, "max_mean_ns") ||
      !contains(windows, "phase_j_performance_runner") ||
      !contains(linux, "phase_j_performance_runner") ||
      !contains(linux, "CGPUI_LINUX_BACKEND")) return 4;
  if (!contains(roadmap, "- [x] Steps 811-816: Add performance baselines") ||
      !contains(ledger, "\"phase_j_steps_811_816_status\": \"complete\"") ||
      !contains(ledger, "Phase J Step 817 public stress runner")) return 5;
  return 0;
}
