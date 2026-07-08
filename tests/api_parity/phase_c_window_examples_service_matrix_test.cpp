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
      "examples/api_parity/public_window_examples_service_matrix/main.cpp");
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
          "target(\"api_parity_public_window_examples_service_matrix\")") ||
      !contains(
          xmake,
          "examples/api_parity/public_window_examples_service_matrix/main.cpp") ||
      !contains(
          xmake,
          "target(\"phase_c_window_examples_service_matrix_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_c_window_examples_service_matrix_test.cpp")) {
    return 2;
  }

  constexpr std::array required_example_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicWindowExamplesServiceMatrixView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicWindowExamplesServiceMatrixView>&",
      "static_assert(cgpui::Render<PublicWindowExamplesServiceMatrixView>)",
      "const cgpui::TestContextCapability test = context.test_context()",
      "context.register_command_palette_entry<InstallServiceMenuAction>",
      "context.try_install_native_menu(",
      "cgpui::NativeMenuItemKind::submenu",
      "cgpui::NativeMenuItemKind::separator",
      "cgpui::NativeMenuAccelerator",
      "cgpui::KeyBindingContext::window()",
      "test.dispatch_window_activation(true)",
      "test.dispatch_window_focus(true)",
      "test.simulate_keystrokes(\"ctrl-alt-m\")",
      "cgpui::WindowOptions{}",
      ".decorations(true)",
      ".resizable(true)",
      ".transparent(false)",
      ".titlebar_visible(true)",
      "cgpui::BoxShadow",
      ".shadow(",
      ".shadow_sm()",
      ".fixed()",
      "cgpui::text_input(",
      "cgpui::button(",
      "cgpui::menu_item(",
      "window/examples service matrix",
  };
  for (std::size_t index = 0; index < required_example_fragments.size();
       ++index) {
    if (!contains(example, required_example_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase C Step 359 window/examples service matrix",
      "examples/api_parity/public_window_examples_service_matrix/main.cpp",
      "api_parity_public_window_examples_service_matrix",
      "phase_c_window_examples_service_matrix_test.cpp",
      "menu accelerators and command palette service matrix",
      "window options, shadow, fixed positioning, and text input service examples",
      "Phase C Step 360 window/examples closeout",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(vocabulary, required_docs[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_json,
                "\"step_360\": \"Phase C Step 360 window/examples closeout\"") ||
      !contains(ledger_json,
                "\"next_step\": \"Phase C Step 361 SVG/image element "
                "front-end APIs\"")) {
    return 90;
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
      return 110 + static_cast<int>(index);
    }
  }

  return 0;
}
