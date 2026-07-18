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
std::size_t count(const std::string& text, std::string_view value) {
  std::size_t result = 0;
  for (std::size_t offset = 0;
       (offset = text.find(value, offset)) != std::string::npos;
       offset += value.size()) {
    ++result;
  }
  return result;
}
}  // namespace

int main() {
  const std::string runner = read_source("tests/stress/phase_j_stress_runner.cpp");
  const std::string profile = read_source("docs/gpui-stress-profile.json");
  const std::string validator =
      read_source("tools/gpui_parity/validate_stress_report.py");
  const std::string header = read_source("include/cgpui/ui/test_app.hpp");
  const std::string input = read_source("src/ui/test_app_input.cpp");
  const std::string windows = read_source("scripts/ci/windows-stress-matrix.ps1");
  const std::string linux = read_source("scripts/ci/linux-stress-matrix.sh");
  const std::string macos = read_source("scripts/ci/macos-stress-matrix.sh");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{&runner, &profile, &validator, &header, &input,
                                &windows, &linux, &roadmap, &ledger};
  for (const auto* source : required) if (source->empty()) return 1;
  constexpr std::array scenarios{
      "window_churn", "many_entities", "large_uniform_list", "ime_composition",
      "clipboard_ownership", "drag_drop_cancel", "asset_reload",
      "task_cancellation"};
  for (const auto* scenario : scenarios) {
    if (!contains(runner, scenario) || !contains(profile, scenario)) return 2;
  }
  if (!contains(header, "dispatch_platform_event") ||
      !contains(input, "TestAppWindow::dispatch_platform_event") ||
      !contains(runner, "cgpui::DragExited") ||
      !contains(runner, "cgpui::ImeComposition") ||
      !contains(runner, "cgpui::AssetReloadState")) return 3;
  if (!contains(validator, "min_operation_count") ||
      !contains(windows, "phase_j_stress_runner") ||
      !contains(linux, "CGPUI_LINUX_BACKEND")) return 4;
  if (!contains(roadmap, "- [x] Steps 817-822: Add stress tests") ||
      !contains(ledger, "\"phase_j_steps_817_822_status\": \"complete\"")) {
    return 5;
  }
  if (!contains(profile, "\"macos-arm64\"") ||
      !contains(profile, "\"macos-x86_64\"") ||
      !contains(runner, "platform == \"macos-arm64\"") ||
      !contains(runner, "platform == \"macos-x86_64\"") ||
      count(runner, "results.push_back(") != 8 ||
      !contains(validator, "report.get(\"platform\")") ||
      !contains(macos, "phase_j_stress_runner")) {
    return 6;
  }
  return 0;
}
