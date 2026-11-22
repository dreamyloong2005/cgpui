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
  const std::string header =
      read_source("include/cgpui/ui/test_app.hpp");
  const std::string internal = read_source("src/ui/test_app_internal.hpp");
  const std::string facade = read_source("src/ui/test_app.cpp");
  const std::string platform = read_source("src/ui/test_app_platform.cpp");
  const std::string renderer = read_source("src/ui/test_app_renderer.cpp");
  const std::string runtime =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string runtime_internal =
      read_source("include/cgpui/ui/detail/window_runtime_opaque_declarations.hpp");
  const std::string behavior =
      read_source("tests/ui/test_app_window_setup_test.cpp");
  const std::string cleanliness = read_source(
      "tests/header_cleanliness/test_app_header_cleanliness.cpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/header_cleanliness/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &header, &internal, &facade, &platform, &renderer, &runtime,
      &runtime_internal, &behavior, &cleanliness, &aggregate, &manifests, &vocabulary, &core,
      &ledger_json};
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
      !contains(manifests, "\"test_app_window_setup_test.cpp\"") ||
      !contains(manifests, "\"test_app_header_cleanliness.cpp\"") ||
      !contains(manifests,
                "\"phase_g_test_app_window_setup_structure_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_test_app_window_setup_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(header) > 125 || line_count(internal) > 140 ||
      line_count(facade) > 135 || line_count(platform) > 75 ||
      line_count(renderer) > 45 || line_count(behavior) > 75) return 6;

  if (!contains(vocabulary, "`TestApp`") ||
      !contains(vocabulary, "`TestAppWindow`") ||
      !contains(core, "Standalone `TestApp` and `TestAppWindow`") ||
      !contains(ledger_json, "\"phase_g_step_659_sources\"")) return 8;
  return 0;
}
