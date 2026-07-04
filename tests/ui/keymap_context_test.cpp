#include "window_runtime_test_support.hpp"

#include "cgpui/ui/key_binding.hpp"

#include <variant>

namespace {

RuntimeFixture* keymap_context_fixture = nullptr;

void dispatch_keymap_context_sequence() {
  auto& callback = keymap_context_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 'I',
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 'K',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'L',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'M',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'N',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

class KeymapContextView final : public cgpui::View {
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
    register_priority_chain(context);
    register_view_chain(context);
    register_window_chain(context);
    register_app_chain(context);
    bindings_registered = bind_contexts(context);
    return cgpui::EventResult::consumed_event();
  }

  void register_priority_chain(const cgpui::WindowRuntimeContext& context) {
    context.register_app_action(
        "keymap.priority.app",
        [this](const cgpui::WindowRuntimeContext&) {
          priority_app_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "keymap.priority.window",
        [this](const cgpui::WindowRuntimeContext&) {
          priority_window_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action(
        "keymap.priority.view",
        [this](const cgpui::WindowRuntimeContext&) {
          priority_view_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_focused_element_action(
        focused_element_id,
        "keymap.priority.focused",
        [this](const cgpui::WindowRuntimeContext&) {
          priority_focused_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  void register_view_chain(const cgpui::WindowRuntimeContext& context) {
    context.register_app_action(
        "keymap.view.app",
        [this](const cgpui::WindowRuntimeContext&) {
          view_app_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "keymap.view.window",
        [this](const cgpui::WindowRuntimeContext&) {
          view_window_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action(
        "keymap.view.active",
        [this](const cgpui::WindowRuntimeContext&) {
          view_active_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  void register_window_chain(const cgpui::WindowRuntimeContext& context) {
    context.register_app_action(
        "keymap.window.app",
        [this](const cgpui::WindowRuntimeContext&) {
          window_app_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "keymap.window.active",
        [this](const cgpui::WindowRuntimeContext&) {
          window_active_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  void register_app_chain(const cgpui::WindowRuntimeContext& context) {
    context.register_app_action(
        "keymap.app.active",
        [this](const cgpui::WindowRuntimeContext&) {
          app_active_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  bool bind_contexts(const cgpui::WindowRuntimeContext& context) {
    return context.bind_key(
               "ctrl-k",
               "keymap.priority.app",
               cgpui::KeyBindingContext::app()) &&
           context.bind_key(
               "ctrl-k",
               "keymap.priority.window",
               cgpui::KeyBindingContext::window()) &&
           context.bind_key(
               "ctrl-k",
               "keymap.priority.view",
               cgpui::KeyBindingContext::view(context.view_id)) &&
           context.bind_key(
               "ctrl-k",
               "keymap.priority.focused",
               cgpui::KeyBindingContext::focused_element(
                   focused_element_id)) &&
           context.bind_key(
               "ctrl-l",
               "keymap.view.app",
               cgpui::KeyBindingContext::app()) &&
           context.bind_key(
               "ctrl-l",
               "keymap.view.window",
               cgpui::KeyBindingContext::window()) &&
           context.bind_key(
               "ctrl-l",
               "keymap.view.active",
               cgpui::KeyBindingContext::view(context.view_id)) &&
           context.bind_key(
               "ctrl-m",
               "keymap.window.app",
               cgpui::KeyBindingContext::app()) &&
           context.bind_key(
               "ctrl-m",
               "keymap.window.active",
               cgpui::KeyBindingContext::window()) &&
           context.bind_key(
               "ctrl-m",
               "keymap.window.inactive-view",
               cgpui::KeyBindingContext::view(
                   cgpui::ViewId{context.view_id.value + 100})) &&
           context.bind_key(
               "ctrl-m",
               "keymap.window.inactive-focused",
               cgpui::KeyBindingContext::focused_element(
                   cgpui::ElementId{focused_element_id.value + 100})) &&
           context.bind_key(
               "ctrl-n",
               "keymap.app.active",
               cgpui::KeyBindingContext::app());
  }

  cgpui::ElementId focused_element_id{88};
  bool bindings_registered = false;
  int priority_app_count = 0;
  int priority_window_count = 0;
  int priority_view_count = 0;
  int priority_focused_count = 0;
  int view_app_count = 0;
  int view_window_count = 0;
  int view_active_count = 0;
  int window_app_count = 0;
  int window_active_count = 0;
  int app_active_count = 0;
};

int test_keymap_context_selects_most_specific_active_binding() {
  RuntimeFixture fixture;
  KeymapContextView view;
  keymap_context_fixture = &fixture;
  fixture.app.on_run = &dispatch_keymap_context_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  keymap_context_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (!view.bindings_registered) {
    return 2;
  }
  if (view.priority_focused_count != 1 ||
      view.priority_view_count != 0 ||
      view.priority_window_count != 0 ||
      view.priority_app_count != 0) {
    return 3;
  }
  if (view.view_active_count != 1 ||
      view.view_window_count != 0 ||
      view.view_app_count != 0) {
    return 4;
  }
  if (view.window_active_count != 1 ||
      view.window_app_count != 0) {
    return 5;
  }
  if (view.app_active_count != 1) {
    return 6;
  }

  return 0;
}

} // namespace

int main() {
  return test_keymap_context_selects_most_specific_active_binding();
}
