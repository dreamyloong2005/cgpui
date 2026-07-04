#include "window_runtime_test_support.hpp"

#include "cgpui/ui/key_binding.hpp"

#include <variant>

namespace {

RuntimeFixture* disabled_scope_fixture = nullptr;

void dispatch_disabled_scope_sequence() {
  auto& callback = disabled_scope_fixture->window.callback;
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
      .key_code = 'X',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'Y',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

class DisabledKeyScopeView final : public cgpui::View {
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
    bindings_registered = bind_key_scopes(context);
    return cgpui::EventResult::consumed_event();
  }

  void register_actions(const cgpui::WindowRuntimeContext& context) {
    context.register_app_action(
        "keyscope.app.fallback",
        [this](const cgpui::WindowRuntimeContext&) {
          app_fallback_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "keyscope.window.disabled",
        [this](const cgpui::WindowRuntimeContext&) {
          disabled_window_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action(
        "keyscope.view.fallback",
        [this](const cgpui::WindowRuntimeContext&) {
          view_fallback_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action(
        "keyscope.view.disabled",
        [this](const cgpui::WindowRuntimeContext&) {
          disabled_view_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action(
        "keyscope.partial.fallback",
        [this](const cgpui::WindowRuntimeContext&) {
          partial_fallback_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_focused_element_action(
        focused_element_id,
        "keyscope.partial.disabled",
        [this](const cgpui::WindowRuntimeContext&) {
          disabled_partial_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  bool bind_key_scopes(const cgpui::WindowRuntimeContext& context) {
    disabled_window_scope = cgpui::KeyBindingContext::window().disabled();
    disabled_view_scope =
        cgpui::KeyBindingContext::view(context.view_id).disabled();
    disabled_focused_scope =
        cgpui::KeyBindingContext::focused_element(focused_element_id)
            .disabled();

    return !disabled_window_scope.enabled && !disabled_view_scope.enabled &&
           !disabled_focused_scope.enabled &&
           context.bind_key(
               "ctrl-k",
               "keyscope.app.fallback",
               cgpui::KeyBindingContext::app()) &&
           context.bind_key(
               "ctrl-k",
               "keyscope.window.disabled",
               disabled_window_scope) &&
           context.bind_key(
               "ctrl-l",
               "keyscope.view.fallback",
               cgpui::KeyBindingContext::app()) &&
           context.bind_key(
               "ctrl-l",
               "keyscope.view.disabled",
               disabled_view_scope) &&
           context.bind_key(
               "ctrl-x",
               "keyscope.partial.fallback",
               cgpui::KeyBindingContext::app()) &&
           context.bind_key(
               "ctrl-x ctrl-y",
               "keyscope.partial.disabled",
               disabled_focused_scope);
  }

  cgpui::ElementId focused_element_id{88};
  cgpui::KeyBindingContext disabled_window_scope;
  cgpui::KeyBindingContext disabled_view_scope;
  cgpui::KeyBindingContext disabled_focused_scope;
  bool bindings_registered = false;
  int app_fallback_count = 0;
  int disabled_window_count = 0;
  int view_fallback_count = 0;
  int disabled_view_count = 0;
  int partial_fallback_count = 0;
  int disabled_partial_count = 0;
};

int test_disabled_key_scopes_are_skipped() {
  RuntimeFixture fixture;
  DisabledKeyScopeView view;
  disabled_scope_fixture = &fixture;
  fixture.app.on_run = &dispatch_disabled_scope_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  disabled_scope_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (!view.bindings_registered) {
    return 2;
  }
  if (view.app_fallback_count != 1 || view.disabled_window_count != 0) {
    return 3;
  }
  if (view.view_fallback_count != 1 || view.disabled_view_count != 0) {
    return 4;
  }
  if (view.partial_fallback_count != 1 || view.disabled_partial_count != 0) {
    return 5;
  }

  return 0;
}

} // namespace

int main() {
  return test_disabled_key_scopes_are_skipped();
}
