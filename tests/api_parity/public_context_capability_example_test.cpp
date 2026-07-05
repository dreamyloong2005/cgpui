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
      read_source("examples/api_parity/public_context_capabilities/main.cpp");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string xmake = read_source("xmake.lua");

  if (example.empty() || vocabulary.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"api_parity_public_context_capabilities\")") ||
      !contains(
          xmake,
          "examples/api_parity/public_context_capabilities/main.cpp") ||
      !contains(xmake, "target(\"public_context_capability_example_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/public_context_capability_example_test.cpp")) {
    return 2;
  }

  constexpr std::array required_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicContextCapabilitiesView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicContextCapabilitiesView>&",
      "static_assert(cgpui::Render<PublicContextCapabilitiesView>)",
      "cgpui::AppContext",
      "cgpui::ViewContextCapability<PublicContextCapabilitiesView>",
      "cgpui::WindowContextCapability",
      "cgpui::ElementContextCapability",
      "app_context()",
      "view_context<PublicContextCapabilitiesView>()",
      "window_context()",
      "element_context(",
      "set_global(",
      "update_global<",
      "set_app_theme(",
      "view_context.view()",
      "view_context.weak_view()",
      "window_context.window()",
      "window_context.request_render()",
      "element_context.focus()",
      "element_context.capture_pointer()",
      "element_context.set_cursor(",
      "element_context.state_or_init<",
      "cgpui::Application::create()",
  };
  for (std::size_t index = 0; index < required_fragments.size(); ++index) {
    if (!contains(example, required_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array frozen_terms{
      "`AppContext`",
      "`ViewContextCapability<T>`",
      "`WindowContextCapability`",
      "`ElementContextCapability`",
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
    if (contains(example, forbidden_fragments[index])) {
      return 80 + static_cast<int>(index);
    }
  }

  return 0;
}
