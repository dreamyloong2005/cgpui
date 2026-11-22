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
  const std::string header = read_source("include/cgpui/ui/test_app.hpp");
  const std::string internal = read_source("src/ui/test_app_internal.hpp");
  const std::string facade = read_source("src/ui/test_app.cpp");
  const std::string platform = read_source("src/ui/test_app_platform.cpp");
  const std::string timer = read_source("src/ui/test_app_timer.cpp");
  const std::string behavior =
      read_source("tests/ui/test_app_timer_control_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &header, &internal, &facade, &platform, &timer, &behavior, &manifests, &vocabulary, &core, &ledger_json};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "void advance_time(std::uint64_t delta_ms) const") ||
      !contains(header, "void run_until_parked() const") ||
      !contains(header, "advance_time_until_parked(") ||
      !contains(header, "bool cancel_timer(TimerId id) const")) return 2;
  if (!contains(internal, "void run_until_parked()") ||
      !contains(timer, "TestAppState::run_until_parked()") ||
      !contains(timer, "runtime_context.test_context().run_until_parked()") ||
      !contains(timer, "TestApp::advance_time_until_parked(")) return 3;
  if (!contains(platform, "monotonic_time_ms() const { return 0; }") ||
      !contains(facade, "runtime.state_->windows.window_ = &parent_window") ||
      !contains(facade, "runtime.state_->windows.renderer_ = &parent_renderer") ||
      !contains(behavior, "schedule_repeating_timer(") ||
      !contains(behavior, "app.cancel_timer(view->repeating)")) return 4;
  if (!contains(manifests, "\"test_app_timer_control_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_test_app_timer_control_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(header) > 125 || line_count(internal) > 140 ||
      line_count(facade) > 135 || line_count(platform) > 75 ||
      line_count(timer) > 60 || line_count(behavior) > 95) return 6;

  if (!contains(vocabulary, "`TestApp::advance_time(...)`") ||
      !contains(core, "Deterministic `TestApp` timer control") ||
      !contains(ledger_json, "\"phase_g_step_661_sources\"")) return 8;
  return 0;
}
