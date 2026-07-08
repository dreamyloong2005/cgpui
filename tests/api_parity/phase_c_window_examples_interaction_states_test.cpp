#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

} // namespace

int main() {
  const std::string example = read_source(
      "examples/api_parity/public_window_examples_interaction_states/main.cpp");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string xmake = read_source("xmake.lua");

  if (example.empty() || vocabulary.empty() || roadmap.empty() ||
      ledger_md.empty() || ledger_json.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(
          xmake,
          "target(\"api_parity_public_window_examples_interaction_states\")") ||
      !contains(
          xmake,
          "examples/api_parity/public_window_examples_interaction_states/main.cpp") ||
      !contains(
          xmake,
          "target(\"phase_c_window_examples_interaction_states_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_window_examples_interaction_states_test.cpp")) {
    return 2;
  }

  constexpr std::array required_example_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicWindowExamplesInteractionStatesView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicWindowExamplesInteractionStatesView>&",
      "static_assert(cgpui::Render<PublicWindowExamplesInteractionStatesView>)",
      "cgpui::StyleState",
      "cgpui::StyleOverlay",
      ".hover_style(",
      ".focus_style(",
      ".active_style(",
      ".disabled_style(",
      ".focusable()",
      ".tab_index(",
      ".focus_ring(cgpui::FocusRingVisibility::visible)",
      ".on_click(",
      ".disabled()",
      "cgpui::button(",
      "cgpui::menu_item(",
      "cgpui::checkbox(",
      "cgpui::text_input(",
      "cgpui::KeyBindingContext::window()",
      "context.register_command_palette_entry<ActivateInteractionStateAction>",
      "cgpui::WindowOptions{}",
      "window/examples interaction states",
  };
  for (std::size_t index = 0; index < required_example_fragments.size();
       ++index) {
    if (!contains(example, required_example_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase C Step 358 window/examples interaction states",
      "examples/api_parity/public_window_examples_interaction_states/main.cpp",
      "api_parity_public_window_examples_interaction_states",
      "phase_c_window_examples_interaction_states_test.cpp",
      "hover/focus/active/disabled interaction states",
      "focus ring and tab-index examples",
      "Phase C Step 360 window/examples closeout",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(vocabulary, required_docs[index])) {
      return 60 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_json,
                "\"step_360\": \"Phase C Step 360 window/examples closeout\"") ||
      !contains(ledger_json,
                "\"next_step\": \"Phase C Step 361 SVG/image element "
                "front-end APIs\"")) {
    return 80;
  }

  constexpr std::array forbidden_example_fragments{
      "#include \"cgpui/ui/",
      "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/",
      "#include \"src/",
      "#include \"../",
      "WindowRuntime",
      "ClipboardItem",
      "gpui::test",
      "TaskPriority",
      "StructuredTaskGroup",
      ".runtime",
      "runtime.",
      "window_runtime",
  };
  for (std::size_t index = 0; index < forbidden_example_fragments.size();
       ++index) {
    if (contains(example, forbidden_example_fragments[index])) {
      return 100 + static_cast<int>(index);
    }
  }

  return 0;
}
