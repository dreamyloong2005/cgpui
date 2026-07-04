#include "window_runtime_test_support.hpp"

#include <optional>
#include <string_view>
#include <variant>

namespace {

struct RuntimeSaveAction {
  static constexpr std::string_view name = "typed.runtime.save";
};

struct RuntimeWindowAction {
  static constexpr std::string_view name = "typed.runtime.window";
};

struct RuntimeViewAction {
  static constexpr std::string_view name = "typed.runtime.view";
};

struct RuntimeFocusedAction {
  static constexpr std::string_view name = "typed.runtime.focused";
};

struct RuntimeMissingAction {
  static constexpr std::string_view name = "typed.runtime.missing";
};

struct ContextLegacyAction {
  static constexpr std::string_view name = "typed.context.legacy";
};

struct ContextAppAction {
  static constexpr std::string_view name = "typed.context.app";
};

struct ContextWindowAction {
  static constexpr std::string_view name = "typed.context.window";
};

struct ContextViewAction {
  static constexpr std::string_view name = "typed.context.view";
};

struct ContextFocusedAction {
  static constexpr std::string_view name = "typed.context.focused";
};

RuntimeFixture* typed_action_fixture = nullptr;

void dispatch_typed_action_sequence() {
  auto& callback = typed_action_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

class RuntimeTypedActionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.focus(focused_element_id);
    context.runtime.register_action<RuntimeSaveAction>(
        [this](const cgpui::WindowRuntimeContext& action_context) {
          save_count += 1;
          save_context_view_id = action_context.view_id;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_window_action<RuntimeWindowAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          window_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_view_action<RuntimeViewAction>(
        context.view_id,
        [this](const cgpui::WindowRuntimeContext&) {
          view_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_focused_element_action<RuntimeFocusedAction>(
        focused_element_id,
        [this](const cgpui::WindowRuntimeContext&) {
          focused_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_action(
        "typed.runtime.string",
        [this](const cgpui::WindowRuntimeContext&) {
          string_count += 1;
          return cgpui::EventResult::consumed_event();
        });

    save_result = context.runtime.dispatch_action<RuntimeSaveAction>();
    window_result = context.runtime.dispatch_action<RuntimeWindowAction>();
    view_result = context.runtime.dispatch_action<RuntimeViewAction>();
    focused_result = context.runtime.dispatch_action<RuntimeFocusedAction>();
    string_result = context.runtime.dispatch_action("typed.runtime.string");
    missing_result = context.runtime.dispatch_action<RuntimeMissingAction>();
    last_result = context.runtime.last_action_dispatch();
    return cgpui::EventResult::consumed_event();
  }

  cgpui::ElementId focused_element_id{77};
  int save_count = 0;
  int window_count = 0;
  int view_count = 0;
  int focused_count = 0;
  int string_count = 0;
  cgpui::ViewId save_context_view_id{};
  cgpui::ActionDispatchResult save_result{};
  cgpui::ActionDispatchResult window_result{};
  cgpui::ActionDispatchResult view_result{};
  cgpui::ActionDispatchResult focused_result{};
  cgpui::ActionDispatchResult string_result{};
  cgpui::ActionDispatchResult missing_result{};
  std::optional<cgpui::ActionDispatchResult> last_result;
};

int test_runtime_registers_and_dispatches_typed_actions() {
  RuntimeFixture fixture;
  RuntimeTypedActionView view;
  typed_action_fixture = &fixture;
  fixture.app.on_run = &dispatch_typed_action_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  typed_action_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (view.save_count != 1 || view.window_count != 1 ||
      view.view_count != 1 || view.focused_count != 1 ||
      view.string_count != 1) {
    return 2;
  }
  if (view.save_context_view_id != cgpui::ViewId{1}) {
    return 3;
  }
  if (view.save_result.name != "typed.runtime.save" ||
      !view.save_result.handled || !view.save_result.scope.has_value() ||
      *view.save_result.scope != cgpui::ActionScope::app) {
    return 4;
  }
  if (view.window_result.name != "typed.runtime.window" ||
      !view.window_result.scope.has_value() ||
      *view.window_result.scope != cgpui::ActionScope::window) {
    return 5;
  }
  if (view.view_result.name != "typed.runtime.view" ||
      !view.view_result.scope.has_value() ||
      *view.view_result.scope != cgpui::ActionScope::view ||
      !view.view_result.view_id.has_value() ||
      *view.view_result.view_id != cgpui::ViewId{1}) {
    return 6;
  }
  if (view.focused_result.name != "typed.runtime.focused" ||
      !view.focused_result.scope.has_value() ||
      *view.focused_result.scope != cgpui::ActionScope::focused_element ||
      !view.focused_result.element_id.has_value() ||
      *view.focused_result.element_id != view.focused_element_id) {
    return 7;
  }
  if (view.string_result.name != "typed.runtime.string" ||
      !view.string_result.handled ||
      view.missing_result.name != "typed.runtime.missing" ||
      view.missing_result.handled || view.missing_result.scope.has_value()) {
    return 8;
  }
  if (!view.last_result.has_value() ||
      view.last_result->name != "typed.runtime.missing" ||
      view.last_result->handled) {
    return 9;
  }

  return 0;
}

class ContextTypedActionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.focus(focused_element_id);
    context.register_action<ContextLegacyAction>(
        [this](const cgpui::ViewContext& action_context) {
          legacy_count += 1;
          legacy_context_view_id = action_context.view_id;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action<ContextAppAction>(
        [this](const cgpui::ViewContext&) {
          app_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action<ContextWindowAction>(
        [this](const cgpui::ViewContext&) {
          window_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action<ContextViewAction>(
        [this](const cgpui::ViewContext&) {
          view_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_focused_element_action<ContextFocusedAction>(
        focused_element_id,
        [this](const cgpui::ViewContext&) {
          focused_count += 1;
          return cgpui::EventResult::consumed_event();
        });

    legacy_result = context.dispatch_action<ContextLegacyAction>();
    app_result = context.dispatch_action<ContextAppAction>();
    window_result = context.dispatch_action<ContextWindowAction>();
    view_result = context.dispatch_action<ContextViewAction>();
    focused_result = context.dispatch_action<ContextFocusedAction>();
    return cgpui::EventResult::consumed_event();
  }

  cgpui::ElementId focused_element_id{91};
  int legacy_count = 0;
  int app_count = 0;
  int window_count = 0;
  int view_count = 0;
  int focused_count = 0;
  cgpui::ViewId legacy_context_view_id{};
  cgpui::ActionDispatchResult legacy_result{};
  cgpui::ActionDispatchResult app_result{};
  cgpui::ActionDispatchResult window_result{};
  cgpui::ActionDispatchResult view_result{};
  cgpui::ActionDispatchResult focused_result{};
};

int test_context_registers_and_dispatches_typed_actions() {
  RuntimeFixture fixture;
  ContextTypedActionView view;
  typed_action_fixture = &fixture;
  fixture.app.on_run = &dispatch_typed_action_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  typed_action_fixture = nullptr;

  if (result != 0) {
    return 10;
  }
  if (view.legacy_count != 1 || view.app_count != 1 ||
      view.window_count != 1 || view.view_count != 1 ||
      view.focused_count != 1) {
    return 11;
  }
  if (view.legacy_context_view_id != cgpui::ViewId{1}) {
    return 12;
  }
  if (view.legacy_result.name != "typed.context.legacy" ||
      !view.legacy_result.scope.has_value() ||
      *view.legacy_result.scope != cgpui::ActionScope::app ||
      view.app_result.name != "typed.context.app" ||
      !view.app_result.scope.has_value() ||
      *view.app_result.scope != cgpui::ActionScope::app) {
    return 13;
  }
  if (view.window_result.name != "typed.context.window" ||
      !view.window_result.scope.has_value() ||
      *view.window_result.scope != cgpui::ActionScope::window ||
      view.view_result.name != "typed.context.view" ||
      !view.view_result.scope.has_value() ||
      *view.view_result.scope != cgpui::ActionScope::view) {
    return 14;
  }
  if (view.focused_result.name != "typed.context.focused" ||
      !view.focused_result.scope.has_value() ||
      *view.focused_result.scope != cgpui::ActionScope::focused_element ||
      !view.focused_result.element_id.has_value() ||
      *view.focused_result.element_id != view.focused_element_id) {
    return 15;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_runtime_registers_and_dispatches_typed_actions();
      result != 0) {
    return result;
  }
  if (const int result = test_context_registers_and_dispatches_typed_actions();
      result != 0) {
    return result;
  }
  return 0;
}
