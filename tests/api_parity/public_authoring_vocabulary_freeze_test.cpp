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
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string prelude = read_source("include/cgpui/prelude.hpp");
  const std::string ui_aggregate = read_source("include/cgpui/ui/ui.hpp");
  const std::string app_aggregate = read_source("include/cgpui/app/app.hpp");
  const std::string example =
      read_source("examples/api_parity/public_api_compatibility/main.cpp");
  const std::string xmake = read_source("xmake.lua");

  if (vocabulary.empty() || prelude.empty() || ui_aggregate.empty() ||
      app_aggregate.empty() || example.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"public_authoring_vocabulary_freeze_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/public_authoring_vocabulary_freeze_test.cpp")) {
    return 2;
  }

  constexpr std::array required_sections{
      "# Public Authoring Vocabulary Freeze",
      "Phase B Step 308",
      "The frozen include boundary is `#include \"cgpui/prelude.hpp\"`.",
      "Application entry vocabulary",
      "View authoring vocabulary",
      "Entity and view handle vocabulary",
      "Action and key vocabulary",
      "Async, test, and Result vocabulary",
      "Platform service vocabulary",
      "Built-in widget vocabulary",
      "Uniform list vocabulary",
      "Out of scope for this freeze",
      "Forbidden from public examples",
  };
  for (std::size_t index = 0; index < required_sections.size(); ++index) {
    if (!contains(vocabulary, required_sections[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array frozen_terms{
      "`Application`",
      "`App`",
      "`Window`",
      "`WindowOptions`",
      "`AppContext`",
      "`Context<T>`",
      "`Render<T>`",
      "`IntoElement`",
      "`ViewContext`",
      "`ViewContextCapability<T>`",
      "`WindowContextCapability`",
      "`ElementContextCapability`",
      "`EntityHandle<T>`",
      "`WeakEntity<T>`",
      "`ViewHandle<T>`",
      "`WeakViewHandle<T>`",
      "`Action<T>`",
      "`CommandPaletteEntry`",
      "`KeyBindingContext`",
      "`Result<T>`",
      "`ErrorCode`",
      "`AsyncContextCapability`",
      "`TestContextCapability`",
      "`NativeMenuModel`",
      "`NativeFileDialogOptions`",
      "`button(...)`",
      "`ButtonBuilder::label(...)`",
      "`label(...)`",
      "`text_input(...)`",
      "`TextModel`",
      "`checkbox(...)`",
      "`radio(...)`",
      "`toggle_switch(...)`",
      "`ToggleBuilder`",
      "`slider(...)`",
      "`SliderBuilder`",
      "`list_item(...)`",
      "`menu_item(...)`",
      "`ItemBuilder`",
      "`image(...)`",
      "`icon(...)`",
      "`svg(...)`",
      "`ImageBuilder`",
      "`ImageSource`",
      "`ImageSourceKind`",
      "`image_source(...)`",
      "`svg_image_source(...)`",
      "`div()`",
      "`h_flex()`",
      "`v_flex()`",
      "`h_stack()`",
      "`v_stack()`",
      "`UniformListVisibleRange`",
      "`UniformListItemIdentity`",
      "`UniformListItemMeasurement`",
      "`UniformListItemMeasurementResult`",
      "`UniformListRecyclingWindow`",
      "`UniformListSelectionSource`",
      "`UniformListSelectionDirection`",
      "`UniformListSelection`",
      "`UniformListSelectionState`",
      "`UniformListLayoutSnapshot`",
      "`UniformListScrollAnchor`",
      "`UniformListItemMeasurementCache`",
      "`calculate_uniform_list_visible_range(...)`",
      "`measure_uniform_list_items(...)`",
      "`calculate_uniform_list_recycling_window(...)`",
      "`select_uniform_list_item_at_point(...)`",
      "`move_uniform_list_selection(...)`",
      "`capture_uniform_list_scroll_anchor(...)`",
      "`apply_uniform_list_scroll_anchor(...)`",
      "`ScrollableListElement::layout_snapshot()`",
      "`ScrollableListElement::measurement_cache()`",
      "`ScrollableListElement::selection()`",
  };
  for (std::size_t index = 0; index < frozen_terms.size(); ++index) {
    if (!contains(vocabulary, frozen_terms[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  constexpr std::array out_of_scope{
      "`ClipboardItem` payload parity",
      "upstream `gpui::test` macro equivalents",
      "remain explicit types outside the macro contract",
      "task priorities",
      "structured task groups",
  };
  for (std::size_t index = 0; index < out_of_scope.size(); ++index) {
    if (!contains(vocabulary, out_of_scope[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  constexpr std::array prelude_edges{
      "cgpui/app/app.hpp",
      "cgpui/core/entity.hpp",
      "cgpui/core/error.hpp",
      "cgpui/ui/ui.hpp",
  };
  for (std::size_t index = 0; index < prelude_edges.size(); ++index) {
    if (!contains(prelude, prelude_edges[index])) {
      return 90 + static_cast<int>(index);
    }
  }

  constexpr std::array aggregate_edges{
      "cgpui/app/application.hpp",
      "cgpui/app/app_facade.hpp",
      "cgpui/app/window.hpp",
      "cgpui/ui/action.hpp",
      "cgpui/ui/async_context.hpp",
      "cgpui/ui/key_binding.hpp",
      "cgpui/ui/render.hpp",
      "cgpui/ui/test_context.hpp",
      "cgpui/ui/uniform_list.hpp",
      "cgpui/ui/view_context.hpp",
      "cgpui/ui/view_handle.hpp",
      "cgpui/ui/window_context.hpp",
  };
  for (std::size_t index = 0; index < aggregate_edges.size(); ++index) {
    const bool in_app = contains(app_aggregate, aggregate_edges[index]);
    const bool in_ui = contains(ui_aggregate, aggregate_edges[index]);
    if (!in_app && !in_ui) {
      return 110 + static_cast<int>(index);
    }
  }

  constexpr std::array example_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "cgpui::Application::create()",
      "cgpui::App",
      "cgpui::Window",
      "cgpui::Context<",
      "cgpui::Render<",
      "cgpui::IntoElement",
      "cgpui::EntityHandle<",
      "cgpui::WeakEntity<",
      "cgpui::ViewHandle<",
      "cgpui::WeakViewHandle<",
      "cgpui::Action<",
      "cgpui::CommandPaletteEntry",
      "cgpui::KeyBindingContext",
      "cgpui::Result<",
      "cgpui::AsyncContextCapability",
      "cgpui::TestContextCapability",
      "cgpui::NativeMenuModel",
      "cgpui::NativeFileDialogOptions",
  };
  for (std::size_t index = 0; index < example_fragments.size(); ++index) {
    if (!contains(example, example_fragments[index])) {
      return 140 + static_cast<int>(index);
    }
  }

  constexpr std::array forbidden_example_fragments{
      "WindowRuntime",
      "#include \"cgpui/ui/",
      "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/",
      "#include \"src/",
      "#include \"../",
  };
  for (std::size_t index = 0; index < forbidden_example_fragments.size();
       ++index) {
    if (contains(example, forbidden_example_fragments[index])) {
      return 180 + static_cast<int>(index);
    }
  }

  return 0;
}
