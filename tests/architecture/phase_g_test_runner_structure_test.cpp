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
      read_source("include/cgpui/ui/test_runner.hpp");
  const std::string source = read_source("src/ui/test_runner.cpp");
  const std::string behavior = read_source("tests/ui/test_runner_test.cpp");
  const std::string macro_behavior =
      read_source("tests/ui/test_runner_macro_test.cpp");
  const std::string cleanliness = read_source(
      "tests/header_cleanliness/test_runner_header_cleanliness.cpp");
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
      &header, &source, &behavior, &macro_behavior, &cleanliness, &aggregate, &manifests, &vocabulary, &core, &ledger_json};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "struct TestRunOptions") ||
      !contains(header, "struct TestRunSummary") ||
      !contains(header, "TestRunArgumentStorage") ||
      !contains(header, "#define CGPUI_TEST_WITH_OPTIONS") ||
      !contains(header, "#define CGPUI_TEST(name") ||
      contains(header, "std::function")) return 2;
  if (!contains(source, "read_environment_number(\"SEED\")") ||
      !contains(source, "read_environment_number(\"ITERATIONS\")") ||
      !contains(source, "attempt <= options.max_retries") ||
      !contains(source, "current_exception_message()") ||
      contains(source, "WindowRuntime")) return 3;
  if (!contains(behavior, "std::vector<std::uint64_t>{0, 0, 1, 1, 9, 9}") ||
      !contains(behavior, "environment_seeds !=") ||
      !contains(behavior, "reported_failure != failure_summary.failure") ||
      !contains(macro_behavior, "CGPUI_TEST(") ||
      !contains(macro_behavior, "cgpui::TestApp& second")) return 4;
  if (!contains(cleanliness, "#include \"cgpui/ui/test_runner.hpp\"") ||
      !contains(aggregate, "#include \"cgpui/ui/test_runner.hpp\"")) {
    return 5;
  }
  if (!contains(manifests, "\"test_runner_test.cpp\"") ||
      !contains(manifests, "\"test_runner_macro_test.cpp\"") ||
      !contains(manifests, "\"test_runner_header_cleanliness.cpp\"") ||
      !contains(manifests, "\"phase_g_test_runner_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(header) > 190 || line_count(source) > 190 ||
      line_count(behavior) > 140 || line_count(macro_behavior) > 65 ||
      line_count(cleanliness) > 25) return 7;

  if (!contains(vocabulary, "`CGPUI_TEST`") ||
      !contains(core, "GPUI-style C++ test runner") ||
      !contains(ledger_json, "\"phase_g_step_665_sources\"")) return 9;
  return 0;
}
