#include "window_runtime_test_support.hpp"

#include <variant>

namespace {

RuntimeFixture* command_palette_key_fixture = nullptr;

void dispatch_command_palette_key_sequence() {
  auto& callback = command_palette_key_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 'I',
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 'P',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'L',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'F',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'D',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'B',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

class CommandPaletteKeyIntegrationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
    if (key == nullptr || key->key_code != 'I') {
      return cgpui::EventResult::unhandled();
    }

    context.focus(focused_element_id);
    register_actions(context);
    register_palette_entries(context);
    entry_count = context.command_palette_entries().size();
    return cgpui::EventResult::consumed_event();
  }

  void register_actions(const cgpui::WindowRuntimeContext& context) {
    context.register_app_action(
        "palette.key.app",
        [this](const cgpui::WindowRuntimeContext&) {
          app_key_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action(
        "palette.key.view",
        [this](const cgpui::WindowRuntimeContext&) {
          view_key_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_focused_element_action(
        focused_element_id,
        "palette.key.focused",
        [this](const cgpui::WindowRuntimeContext&) {
          focused_key_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action(
        "palette.key.disabled",
        [this](const cgpui::WindowRuntimeContext&) {
          disabled_key_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action(
        "palette.key.invalid",
        [this](const cgpui::WindowRuntimeContext&) {
          invalid_key_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action(
        "palette.key.bound",
        [this](const cgpui::WindowRuntimeContext&) {
          explicit_context_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  void register_palette_entries(const cgpui::WindowRuntimeContext& context) {
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.key.app",
        .title = "Open Palette",
        .group = "Command",
        .scope = cgpui::ActionScope::app,
        .key_binding = "ctrl-p",
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.key.view",
        .title = "List View Commands",
        .group = "Command",
        .scope = cgpui::ActionScope::view,
        .view_id = context.view_id,
        .key_binding = "ctrl-l",
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.key.focused",
        .title = "Focused Command",
        .group = "Command",
        .scope = cgpui::ActionScope::focused_element,
        .element_id = focused_element_id,
        .key_binding = "ctrl-f",
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.key.disabled",
        .title = "Disabled Command",
        .group = "Command",
        .scope = cgpui::ActionScope::app,
        .enabled = false,
        .key_binding = "ctrl-d",
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.key.invalid",
        .title = "Invalid Key Command",
        .group = "Command",
        .scope = cgpui::ActionScope::app,
        .key_binding = "ctrl-unknown",
    });
    context.register_command_palette_entry(cgpui::CommandPaletteEntry{
        .action_name = "palette.key.bound",
        .title = "Explicit Context Command",
        .group = "Command",
        .scope = cgpui::ActionScope::app,
        .key_binding = "ctrl-b",
        .key_context = cgpui::KeyBindingContext::window(),
    });
  }

  cgpui::ElementId focused_element_id{94};
  std::size_t entry_count = 0;
  int app_key_count = 0;
  int view_key_count = 0;
  int focused_key_count = 0;
  int disabled_key_count = 0;
  int invalid_key_count = 0;
  int explicit_context_count = 0;
};

int test_command_palette_entries_install_key_bindings() {
  RuntimeFixture fixture;
  CommandPaletteKeyIntegrationView view;
  command_palette_key_fixture = &fixture;
  fixture.app.on_run = &dispatch_command_palette_key_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  command_palette_key_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (view.entry_count != 6 || runtime.command_palette_entries().size() != 6) {
    return 2;
  }
  if (view.app_key_count != 1 || view.view_key_count != 1 ||
      view.focused_key_count != 1 || view.explicit_context_count != 1) {
    return 3;
  }
  if (view.disabled_key_count != 0 || view.invalid_key_count != 0) {
    return 4;
  }

  return 0;
}

} // namespace

int main() {
  return test_command_palette_entries_install_key_bindings();
}
