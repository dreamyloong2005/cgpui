#include "window_runtime_test_support.hpp"

#include <optional>
#include <span>
#include <string_view>
#include <vector>
#include <variant>

namespace {

struct RuntimeGeneralAction {
  static constexpr std::string_view name = "scope.runtime.general";
};

struct RuntimeWindowAction {
  static constexpr std::string_view name = "scope.runtime.window";
};

struct RuntimeViewAction {
  static constexpr std::string_view name = "scope.runtime.view";
};

struct RuntimeFocusedAction {
  static constexpr std::string_view name = "scope.runtime.focused";
};

RuntimeFixture* scope_metadata_fixture = nullptr;

void dispatch_scope_metadata_sequence() {
  auto& callback = scope_metadata_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 77,
      .action = cgpui::KeyAction::pressed});
}

std::optional<cgpui::ActionRegistration> find_registration(
    std::span<const cgpui::ActionRegistration> registrations,
    std::string_view action_name,
    cgpui::ActionRegistrationScope registration_scope) {
  for (const cgpui::ActionRegistration& registration : registrations) {
    if (registration.name == action_name &&
        registration.registration_scope == registration_scope) {
      return registration;
    }
  }
  return std::nullopt;
}

std::size_t count_registration(
    std::span<const cgpui::ActionRegistration> registrations,
    std::string_view action_name,
    cgpui::ActionRegistrationScope registration_scope) {
  std::size_t count = 0;
  for (const cgpui::ActionRegistration& registration : registrations) {
    if (registration.name == action_name &&
        registration.registration_scope == registration_scope) {
      count += 1;
    }
  }
  return count;
}

class ActionScopeMetadataView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.focus(focused_element_id);
    context.runtime.register_action<RuntimeGeneralAction>(
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_action<RuntimeGeneralAction>(
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_app_action(
        "scope.runtime.app",
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_window_action<RuntimeWindowAction>(
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_view_action<RuntimeViewAction>(
        context.view_id,
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_focused_element_action<RuntimeFocusedAction>(
        focused_element_id,
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.runtime.register_action(
        "scope.invalid.empty_handler",
        cgpui::ActionHandler{});
    context.runtime.register_action(
        "",
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });

    context.register_action(
        "scope.context.general",
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action(
        "scope.context.view",
        [](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::consumed_event();
        });

    registrations.assign(
        context.runtime.action_registrations().begin(),
        context.runtime.action_registrations().end());
    context_registrations.assign(
        context.action_registrations().begin(),
        context.action_registrations().end());
    view_registrations = context.action_registrations_for_scope(
        cgpui::ActionRegistrationScope::view);

    return cgpui::EventResult::consumed_event();
  }

  cgpui::ElementId focused_element_id{93};
  std::vector<cgpui::ActionRegistration> registrations;
  std::vector<cgpui::ActionRegistration> context_registrations;
  std::vector<cgpui::ActionRegistration> view_registrations;
};

int test_action_scope_metadata_records_registration_sources() {
  RuntimeFixture fixture;
  ActionScopeMetadataView view;
  scope_metadata_fixture = &fixture;
  fixture.app.on_run = &dispatch_scope_metadata_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  scope_metadata_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (view.registrations.size() != view.context_registrations.size()) {
    return 2;
  }

  const auto general = find_registration(
      view.registrations,
      RuntimeGeneralAction::name,
      cgpui::ActionRegistrationScope::general);
  if (!general.has_value() ||
      general->dispatch_scope != cgpui::ActionScope::app ||
      general->view_id.has_value() || general->element_id.has_value()) {
    return 3;
  }
  if (count_registration(
          view.registrations,
          RuntimeGeneralAction::name,
          cgpui::ActionRegistrationScope::general) != 1) {
    return 4;
  }

  const auto app = find_registration(
      view.registrations,
      "scope.runtime.app",
      cgpui::ActionRegistrationScope::app);
  if (!app.has_value() || app->dispatch_scope != cgpui::ActionScope::app) {
    return 5;
  }

  const auto window = find_registration(
      view.registrations,
      RuntimeWindowAction::name,
      cgpui::ActionRegistrationScope::window);
  if (!window.has_value() ||
      window->dispatch_scope != cgpui::ActionScope::window) {
    return 6;
  }

  const auto view_registration = find_registration(
      view.registrations,
      RuntimeViewAction::name,
      cgpui::ActionRegistrationScope::view);
  if (!view_registration.has_value() ||
      view_registration->dispatch_scope != cgpui::ActionScope::view ||
      !view_registration->view_id.has_value() ||
      *view_registration->view_id != cgpui::ViewId{1}) {
    return 7;
  }

  const auto focused = find_registration(
      view.registrations,
      RuntimeFocusedAction::name,
      cgpui::ActionRegistrationScope::focused_element);
  if (!focused.has_value() ||
      focused->dispatch_scope != cgpui::ActionScope::focused_element ||
      !focused->element_id.has_value() ||
      *focused->element_id != view.focused_element_id) {
    return 8;
  }

  if (find_registration(
          view.registrations,
          "scope.invalid.empty_handler",
          cgpui::ActionRegistrationScope::general)
          .has_value()) {
    return 9;
  }
  if (view.view_registrations.size() != 2) {
    return 10;
  }
  if (!find_registration(
           view.view_registrations,
           "scope.context.view",
           cgpui::ActionRegistrationScope::view)
           .has_value()) {
    return 11;
  }

  return 0;
}

} // namespace

int main() {
  return test_action_scope_metadata_records_registration_sources();
}
