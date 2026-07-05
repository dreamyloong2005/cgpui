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
  const std::string example =
      read_source("examples/api_parity/public_phase_b_surface_closure/main.cpp");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string xmake = read_source("xmake.lua");

  if (example.empty() || vocabulary.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"api_parity_public_phase_b_surface_closure\")") ||
      !contains(
          xmake,
          "examples/api_parity/public_phase_b_surface_closure/main.cpp") ||
      !contains(xmake,
                "target(\"public_phase_b_surface_closure_example_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/public_phase_b_surface_closure_example_test.cpp")) {
    return 2;
  }

  constexpr std::array required_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicPhaseBSurfaceClosureView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicPhaseBSurfaceClosureView>&",
      "static_assert(cgpui::Render<PublicPhaseBSurfaceClosureView>)",
      "cgpui::Application::create()",
      "cgpui::AppRunnerOptions",
      "cgpui::WindowOptions",
      "cgpui::AppContext",
      "cgpui::ViewContextCapability<PublicPhaseBSurfaceClosureView>",
      "cgpui::WindowContextCapability",
      "cgpui::ElementContextCapability",
      "cgpui::EntityHandle<",
      "cgpui::WeakEntity<",
      "cgpui::ViewHandle<",
      "cgpui::WeakViewHandle<",
      "cgpui::Action<",
      "cgpui::CommandPaletteEntry",
      "cgpui::KeyBindingContext",
      "cgpui::Result<",
      "cgpui::ErrorCode",
      "cgpui::AsyncContextCapability",
      "cgpui::TestContextCapability",
      "cgpui::NativeMenuModel",
      "cgpui::NativeFileDialogOptions",
      "try_open_window(",
      "try_install_native_menu(",
      "try_show_native_file_dialog(",
      "try_spawn_task(",
      "try_spawn_background_task(",
      "try_draw_frame()",
      "bind_key(",
      "register_command_palette_entry<",
      "capture_pointer(",
      "release_pointer(",
  };
  for (std::size_t index = 0; index < required_fragments.size(); ++index) {
    if (!contains(example, required_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array frozen_terms{
      "`Application`",
      "`App`",
      "`Window`",
      "`AppContext`",
      "`Context<T>`",
      "`Render<T>`",
      "`IntoElement`",
      "`EntityHandle<T>`",
      "`WeakEntity<T>`",
      "`ViewHandle<T>`",
      "`WeakViewHandle<T>`",
      "`CommandPaletteEntry`",
      "`KeyBindingContext`",
      "`Result<T>`",
      "`ErrorCode`",
      "`AsyncContextCapability`",
      "`TestContextCapability`",
      "`NativeMenuModel`",
      "`NativeFileDialogOptions`",
  };
  for (std::size_t index = 0; index < frozen_terms.size(); ++index) {
    if (!contains(vocabulary, frozen_terms[index])) {
      return 70 + static_cast<int>(index);
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
    if (contains(example, forbidden_fragments[index])) {
      return 100 + static_cast<int>(index);
    }
  }

  return 0;
}
