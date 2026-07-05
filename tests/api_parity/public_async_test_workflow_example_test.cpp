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
      read_source("examples/api_parity/public_async_test_workflow/main.cpp");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string xmake = read_source("xmake.lua");

  if (example.empty() || vocabulary.empty() || xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"api_parity_public_async_test_workflow\")") ||
      !contains(
          xmake,
          "examples/api_parity/public_async_test_workflow/main.cpp") ||
      !contains(xmake,
                "target(\"public_async_test_workflow_example_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/public_async_test_workflow_example_test.cpp")) {
    return 2;
  }

  constexpr std::array required_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicAsyncTestWorkflowView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicAsyncTestWorkflowView>&",
      "static_assert(cgpui::Render<PublicAsyncTestWorkflowView>)",
      "const cgpui::AsyncContextCapability async = context.async_context()",
      "const cgpui::TestContextCapability test = context.test_context()",
      "async.defer(",
      "async.schedule_timer(",
      "async.schedule_repeating_timer(",
      "async.try_spawn_task(",
      "async.try_spawn_background_task(",
      "async.batch_updates(",
      "test.run_until_parked()",
      "test.advance_time_until_parked(",
      "test.request_redraw()",
      "test.try_draw_frame()",
      "test.draw_frame()",
      "test.dispatch_keystroke(",
      "test.simulate_keystrokes(",
      "test.dispatch_pointer_move(",
      "test.dispatch_pointer_button(",
      "test.dispatch_pointer_scroll(",
      "test.dispatch_window_activation(",
      "test.dispatch_window_focus(",
      "test.focus(",
      "test.release_focus(",
      "test.write_to_clipboard(",
      "test.read_from_clipboard()",
      "test.paste_clipboard_text()",
      "cgpui::Application::create()",
  };
  for (std::size_t index = 0; index < required_fragments.size(); ++index) {
    if (!contains(example, required_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array frozen_terms{
      "`AsyncContextCapability`",
      "`TestContextCapability`",
      "`try_spawn_task(...) -> Result<TaskHandle>`",
      "`try_draw_frame() -> Result<void>`",
  };
  for (std::size_t index = 0; index < frozen_terms.size(); ++index) {
    if (!contains(vocabulary, frozen_terms[index])) {
      return 60 + static_cast<int>(index);
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
      return 90 + static_cast<int>(index);
    }
  }

  return 0;
}
