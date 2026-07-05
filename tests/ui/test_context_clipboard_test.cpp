#include "window_runtime_test_support.hpp"

#include <optional>
#include <string>

namespace {

RuntimeFixture* test_context_clipboard_fixture = nullptr;

void dispatch_test_context_clipboard_event() {
  test_context_clipboard_fixture->window.callback(cgpui::KeyboardKey{
      .key_code = 'C',
      .action = cgpui::KeyAction::pressed});
}

int test_test_context_drives_runtime_clipboard_helpers() {
  RuntimeFixture fixture;
  test_context_clipboard_fixture = &fixture;
  fixture.app.on_run = &dispatch_test_context_clipboard_event;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcd");
  model.set_selection(1, 3);
  cgpui::MemoryClipboard clipboard;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);

  bool installed_clipboard = false;
  bool wrote_direct = false;
  bool copied = false;
  bool cut = false;
  bool pasted = false;
  bool missing_clipboard_failed_softly = false;
  std::optional<std::string> direct_text;
  std::optional<std::string> copied_text;
  std::optional<std::string> cut_text;
  std::size_t clipboard_diagnostic_count = 0;

  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        const cgpui::TestContextCapability test_context =
            context.test_context();
        missing_clipboard_failed_softly =
            !test_context.write_to_clipboard("missing") &&
            !test_context.read_from_clipboard().has_value() &&
            !test_context.copy_selection_to_clipboard() &&
            !test_context.cut_selection_to_clipboard() &&
            !test_context.paste_clipboard_text();

        test_context.set_clipboard(&clipboard);
        installed_clipboard = true;

        wrote_direct = test_context.write_to_clipboard("direct");
        direct_text = test_context.read_from_clipboard();

        copied = test_context.copy_selection_to_clipboard();
        copied_text = test_context.read_from_clipboard();

        cut = test_context.cut_selection_to_clipboard();
        cut_text = test_context.read_from_clipboard();

        pasted = test_context.paste_clipboard_text();
        clipboard_diagnostic_count =
            test_context.platform_diagnostics().size();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  test_context_clipboard_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (!installed_clipboard || !wrote_direct || !direct_text.has_value() ||
      *direct_text != "direct") {
    return 2;
  }
  if (!missing_clipboard_failed_softly) {
    return 3;
  }
  if (!copied || !copied_text.has_value() || *copied_text != "bc") {
    return 4;
  }
  if (!cut || !cut_text.has_value() || *cut_text != "bc") {
    return 5;
  }
  if (!pasted || model.text() != "abcd" || model.cursor() != 3 ||
      !model.selection().collapsed) {
    return 6;
  }
  if (clipboard_diagnostic_count < 11) {
    return 7;
  }

  return 0;
}

} // namespace

int main() {
  return test_test_context_drives_runtime_clipboard_helpers();
}
