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
  const std::string action = read_source("include/cgpui/ui/action.hpp");
  const std::string macros = read_source("include/cgpui/ui/action_macros.hpp");
  const std::string aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string behavior =
      read_source("tests/api_parity/action_macro_surface_test.cpp");
  const std::string cleanliness = read_source(
      "tests/header_cleanliness/action_macros_header_cleanliness.cpp");
  const std::string manifests = read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/header_cleanliness/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &action,      &macros,     &aggregate,  &behavior, &cleanliness,
      &manifests,      &manifest,   &vocabulary,
      &core,        &ledger_md,  &ledger_json};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(action, "concept Action") ||
      !contains(macros, "#include \"cgpui/ui/action.hpp\"") ||
      !contains(macros, "#define CGPUI_ACTIONS(...)") ||
      !contains(macros, "#define CGPUI_ACTIONS_IN(namespace_name, ...)") ||
      !contains(macros, "friend constexpr bool operator==") ||
      contains(macros, "WindowRuntime")) {
    return 2;
  }
  if (!contains(behavior, "#include \"cgpui/prelude.hpp\"") ||
      !contains(behavior, "CGPUI_ACTIONS(SaveDocumentAction") ||
      !contains(behavior, "CGPUI_ACTIONS_IN(editor") ||
      !contains(behavior, "editor::OpenDocumentAction") ||
      !contains(behavior, "std::equality_comparable")) {
    return 3;
  }
  if (!contains(cleanliness,
                "#include \"cgpui/ui/action_macros.hpp\"") ||
      !contains(cleanliness, "header_scope::ScopedHeaderAction") ||
      !contains(aggregate,
                "#include \"cgpui/ui/action_macros.hpp\"")) {
    return 4;
  }
  if (!contains(manifests, "\"action_macro_surface_test.cpp\"") ||
      !contains(manifests, "\"action_macros_header_cleanliness.cpp\"") ||
      !contains(manifests, "\"phase_g_action_macro_structure_test.cpp\"") ||
      !contains(manifest,
                "action_macros_header_cleanliness|tests/header_cleanliness/"
                "action_macros_header_cleanliness.cpp") ||
      !contains(manifest,
                "phase_g_action_macro_structure_test|tests/architecture/"
                "phase_g_action_macro_structure_test.cpp")) {
    return 5;
  }
  if (line_count(macros) > 150 || line_count(behavior) > 45 ||
      line_count(cleanliness) > 20 || line_count(read_source("mcpp.toml")) > 100) {
    return 6;
  }

  if (!contains(vocabulary, "`CGPUI_ACTIONS(...)`") ||
      !contains(vocabulary, "`CGPUI_ACTIONS_IN(namespace_name, ...)`") ||
      !contains(core, "Unit-action macro production behavior") ||
      contains(ledger_md, "action macro payloads still missing") ||
      contains(ledger_json, "action macro payloads remain missing")) {
    return 8;
  }
  if (!contains(ledger_json, "\"phase_g_step_675_sources\"")) {
    return 10;
  }
  return 0;
}
