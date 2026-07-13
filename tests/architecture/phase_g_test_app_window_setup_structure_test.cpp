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
  const std::string header =
      read_source("include/cgpui/ui/test_app.hpp");
  const std::string internal = read_source("src/ui/test_app_internal.hpp");
  const std::string facade = read_source("src/ui/test_app.cpp");
  const std::string platform = read_source("src/ui/test_app_platform.cpp");
  const std::string renderer = read_source("src/ui/test_app_renderer.cpp");
  const std::string runtime =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string runtime_internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string behavior =
      read_source("tests/ui/test_app_window_setup_test.cpp");
  const std::string cleanliness = read_source(
      "tests/header_cleanliness/test_app_header_cleanliness.cpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_image_gif_examples_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &internal, &facade, &platform, &renderer, &runtime,
      &runtime_internal, &behavior, &cleanliness, &aggregate, &previous,
      &xmake, &vocabulary, &core,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "class TestAppWindow") ||
      !contains(header, "class TestApp") ||
      !contains(header, "Result<TestAppWindow> try_open_window(") ||
      !contains(header, "T* root_view_as()") ||
      contains(header, "PlatformApplication") ||
      contains(header, "TestRenderer")) return 2;
  if (!contains(internal, "struct TestAppState") ||
      !contains(internal, "class TestPlatformApplication final") ||
      !contains(internal, "class TestRenderer final") ||
      contains(runtime, "friend struct detail::TestAppState") ||
      !contains(runtime_internal, "friend struct detail::TestAppState")) {
    return 3;
  }
  if (!contains(facade, "TestApp::try_open_window(") ||
      !contains(facade, "TestApp requires a root view") ||
      !contains(platform, "TestPlatformApplication::create_child_window(") ||
      !contains(renderer, "TestRenderer::begin_frame()")) return 4;
  if (!contains(behavior, "first.runtime_id() == second.runtime_id()") ||
      !contains(behavior, "missing.error().code !=") ||
      !contains(cleanliness, "#include \"cgpui/ui/test_app.hpp\"") ||
      !contains(aggregate, "#include \"cgpui/ui/test_app.hpp\"") ||
      !contains(previous, "Step 659 GPUI-style app and window test setup") ||
      !contains(xmake, "target(\"phase_g_test_app_window_fixture_test\")") ||
      !contains(xmake, "target(\"test_app_header_cleanliness\")") ||
      !contains(xmake,
                "target(\"phase_g_test_app_window_setup_structure_test\")") ||
      !contains(xmake,
                "set_basename(\"phase_g_test_app_window_fixture_structure_test\")")) {
    return 5;
  }
  if (line_count(header) > 125 || line_count(internal) > 140 ||
      line_count(facade) > 135 || line_count(platform) > 75 ||
      line_count(renderer) > 45 || line_count(behavior) > 75) return 6;

  constexpr const char* completion =
      "Phase G Step 659 adds standalone GPUI-style TestApp and TestAppWindow "
      "setup with private deterministic platform and renderer ownership, "
      "persistent multi-window creation, stable runtime/root-view handles, "
      "typed root-view access, and fail-closed empty-root rejection. Step 660 "
      "GPUI-style simulated input production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(vocabulary, "`TestApp`") ||
      !contains(vocabulary, "`TestAppWindow`") ||
      !contains(core, "Standalone `TestApp` and `TestAppWindow`") ||
      !contains(ledger_json, "\"phase_g_step_659_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 668 Linux debug "
                "build and packaging coverage\"")) return 8;
  return 0;
}
