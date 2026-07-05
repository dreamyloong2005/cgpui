#include <array>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(path);
  if (!source) {
    source.open((std::string("../../../../") + path).c_str());
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
  const std::string workflow_example =
      read_source("examples/api_parity/public_authoring_workflow/main.cpp");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string xmake = read_source("xmake.lua");

  if (workflow_example.empty() || vocabulary.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"api_parity_public_authoring_workflow\")") ||
      !contains(
          xmake,
          "examples/api_parity/public_authoring_workflow/main.cpp") ||
      !contains(xmake, "target(\"public_api_example_expansion_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/public_api_example_expansion_test.cpp")) {
    return 2;
  }

  constexpr std::array required_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicAuthoringWorkflowView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicAuthoringWorkflowView>&",
      "static_assert(cgpui::Render<PublicAuthoringWorkflowView>)",
      "cgpui::Application::create()",
      "cgpui::App",
      "cgpui::Window",
      "cgpui::Context<",
      "cgpui::EntityHandle<",
      "cgpui::WeakEntity<",
      "cgpui::ViewHandle<",
      "cgpui::WeakViewHandle<",
      "cgpui::CommandPaletteEntry",
      "cgpui::KeyBindingContext",
      "cgpui::Result<",
      "cgpui::AsyncContextCapability",
      "cgpui::TestContextCapability",
      "cgpui::NativeMenuModel",
      "cgpui::NativeFileDialogOptions",
      "try_install_native_menu(",
      "try_show_native_file_dialog(",
      "try_spawn_task(",
      "try_draw_frame(",
      "bind_key(",
  };
  for (std::size_t index = 0; index < required_fragments.size(); ++index) {
    if (!contains(workflow_example, required_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array frozen_terms{
      "`Application`",
      "`App`",
      "`Window`",
      "`Context<T>`",
      "`EntityHandle<T>`",
      "`WeakEntity<T>`",
      "`ViewHandle<T>`",
      "`WeakViewHandle<T>`",
      "`CommandPaletteEntry`",
      "`KeyBindingContext`",
      "`Result<T>`",
      "`AsyncContextCapability`",
      "`TestContextCapability`",
      "`NativeMenuModel`",
      "`NativeFileDialogOptions`",
  };
  for (std::size_t index = 0; index < frozen_terms.size(); ++index) {
    if (!contains(vocabulary, frozen_terms[index])) {
      return 50 + static_cast<int>(index);
    }
  }

  constexpr std::array forbidden_fragments{
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
  for (std::size_t index = 0; index < forbidden_fragments.size(); ++index) {
    if (contains(workflow_example, forbidden_fragments[index])) {
      return 80 + static_cast<int>(index);
    }
  }

  return 0;
}
