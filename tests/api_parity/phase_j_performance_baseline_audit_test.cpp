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
  const std::string samples = read_source("tests/performance/baseline_sample.cpp");
  const std::string window_workloads = read_source("tests/performance/baseline_window_workloads.cpp");
  const std::string content_workloads = read_source("tests/performance/baseline_content_workloads.cpp");
  const std::string scheduling_workloads = read_source("tests/performance/baseline_scheduling_workloads.cpp");
  const std::string policy =
      read_source("docs/gpui-performance-baseline-policy.json");
  const std::string validator =
      read_source("tools/gpui_parity/validate_performance_baseline.py");
  const std::string windows =
      read_source("scripts/ci/windows-performance-baseline.ps1");
  const std::string linux =
      read_source("scripts/ci/linux-performance-baseline.sh");
  const std::string macos =
      read_source("scripts/ci/macos-performance-baseline.sh");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{&runner, &samples, &window_workloads,
      &content_workloads, &scheduling_workloads, &policy, &validator,
      &windows, &linux, &macos, &ledger};
  for (const auto* source : required) if (source->empty()) return 1;

  constexpr std::array metrics{
      "startup", "first_frame", "resize", "text_layout",
      "list_scrolling", "image_loading", "async_wakeup", "frame_pacing"};
  for (const auto* metric : metrics) {
    if (!contains(samples, metric) || !contains(policy, metric)) return 2;
  }
  if (!contains(window_workloads, "cgpui::TestApp") ||
      !contains(window_workloads, "try_draw_frame") ||
      !contains(window_workloads, "simulate_resize") ||
      !contains(content_workloads, "cgpui::measure_text") ||
      !contains(content_workloads, "calculate_uniform_list_visible_range") ||
      !contains(content_workloads, "cgpui::decode_image") ||
      !contains(scheduling_workloads, "cgpui::TestApp") ||
      !contains(scheduling_workloads, "complete_task") ||
      !contains(scheduling_workloads, "start_animation")) return 3;
  if (!contains(validator, "max_mean_ns") ||
      !contains(windows, "test -p tests/performance") ||
      !contains(linux, "-p tests/performance") ||
      !contains(linux, "CGPUI_LINUX_BACKEND")) return 4;
  if (!contains(ledger, "\"phase_j_steps_811_816_status\"")) {
    return 5;
  }
  if (!contains(policy, "\"macos-arm64\"") ||
      contains(policy, "\"macos-x86_64\"") ||
      !contains(runner, "platform == \"macos-arm64\"") ||
      contains(runner, "platform == \"macos-x86_64\"") ||
      !contains(validator, "report.get(\"platform\")") ||
      !contains(macos, "phase_j_performance_runner")) {
    return 6;
  }
  return 0;
}
