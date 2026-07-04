#include "window_runtime_test_support.hpp"

#include <optional>
#include <span>
#include <string_view>
#include <vector>
#include <variant>

namespace {

struct RuntimeEnabledAction {
  static constexpr std::string_view name = "enablement.runtime.enabled";
};

struct RuntimeDisabledAction {
  static constexpr std::string_view name = "enablement.runtime.disabled";
};

struct ContextDisabledAction {
  static constexpr std::string_view name = "enablement.context.disabled";
};

struct LatestAppAction {
  static constexpr std::string_view name = "enablement.latest.app";
};

RuntimeFixture* enablement_fixture = nullptr;

void dispatch_action_enablement_sequence() {
  auto& callback = enablement_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 69,
      .action = cgpui::KeyAction::pressed});
}

std::optional<cgpui::ActionRegistration> find_registration(
    std::span<const cgpui::ActionRegistration> registrations,
    std::string_view action_name) {
  for (const cgpui::ActionRegistration& registration : registrations) {
    if (registration.name == action_name) {
      return registration;
    }
  }
  return std::nullopt;
}

class ActionEnablementMetadataView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.runtime.register_app_action<RuntimeEnabledAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          runtime_enabled_count += 1;
          return cgpui::EventResult::consumed_event();
        },
        cgpui::ActionRegistrationOptions{.enabled = true});
    context.runtime.register_app_action<RuntimeDisabledAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          runtime_disabled_count += 1;
          return cgpui::EventResult::consumed_event();
        },
        cgpui::ActionRegistrationOptions{.enabled = false});
    context.register_app_action<ContextDisabledAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          context_disabled_count += 1;
          return cgpui::EventResult::consumed_event();
        },
        cgpui::ActionRegistrationOptions{.enabled = false});
    context.register_window_action(
        "enablement.window.disabled",
        [this](const cgpui::WindowRuntimeContext&) {
          window_disabled_count += 1;
          return cgpui::EventResult::consumed_event();
        },
        cgpui::ActionRegistrationOptions{.enabled = false});
    context.register_view_action(
        "enablement.view.enabled",
        [this](const cgpui::WindowRuntimeContext&) {
          view_enabled_count += 1;
          return cgpui::EventResult::consumed_event();
        },
        cgpui::ActionRegistrationOptions{.enabled = true});
    context.runtime.register_action<LatestAppAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          latest_general_count += 1;
          return cgpui::EventResult::consumed_event();
        },
        cgpui::ActionRegistrationOptions{.enabled = false});
    context.runtime.register_app_action<LatestAppAction>(
        [this](const cgpui::WindowRuntimeContext&) {
          latest_app_count += 1;
          return cgpui::EventResult::consumed_event();
        },
        cgpui::ActionRegistrationOptions{.enabled = true});

    registrations.assign(
        context.action_registrations().begin(),
        context.action_registrations().end());
    disabled_registrations = context.action_registrations_for_enabled(false);

    runtime_enabled_result =
        context.dispatch_action<RuntimeEnabledAction>();
    runtime_disabled_result =
        context.dispatch_action<RuntimeDisabledAction>();
    context_disabled_result =
        context.dispatch_action<ContextDisabledAction>();
    window_disabled_result =
        context.dispatch_action("enablement.window.disabled");
    view_enabled_result =
        context.dispatch_action("enablement.view.enabled");
    latest_app_result =
        context.dispatch_action<LatestAppAction>();

    return cgpui::EventResult::consumed_event();
  }

  int runtime_enabled_count = 0;
  int runtime_disabled_count = 0;
  int context_disabled_count = 0;
  int window_disabled_count = 0;
  int view_enabled_count = 0;
  int latest_general_count = 0;
  int latest_app_count = 0;
  std::vector<cgpui::ActionRegistration> registrations;
  std::vector<cgpui::ActionRegistration> disabled_registrations;
  cgpui::ActionDispatchResult runtime_enabled_result{};
  cgpui::ActionDispatchResult runtime_disabled_result{};
  cgpui::ActionDispatchResult context_disabled_result{};
  cgpui::ActionDispatchResult window_disabled_result{};
  cgpui::ActionDispatchResult view_enabled_result{};
  cgpui::ActionDispatchResult latest_app_result{};
};

int test_action_enablement_metadata_controls_dispatch() {
  RuntimeFixture fixture;
  ActionEnablementMetadataView view;
  enablement_fixture = &fixture;
  fixture.app.on_run = &dispatch_action_enablement_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  enablement_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (view.registrations.size() != 7 ||
      view.disabled_registrations.size() != 4) {
    return 2;
  }

  const auto runtime_enabled = find_registration(
      view.registrations,
      RuntimeEnabledAction::name);
  const auto runtime_disabled = find_registration(
      view.registrations,
      RuntimeDisabledAction::name);
  const auto context_disabled = find_registration(
      view.registrations,
      ContextDisabledAction::name);
  if (!runtime_enabled.has_value() || !runtime_enabled->enabled ||
      !runtime_disabled.has_value() || runtime_disabled->enabled ||
      !context_disabled.has_value() || context_disabled->enabled) {
    return 3;
  }
  if (find_registration(
          view.disabled_registrations,
          "enablement.view.enabled")
          .has_value()) {
    return 4;
  }

  if (view.runtime_enabled_count != 1 || view.view_enabled_count != 1) {
    return 5;
  }
  if (view.runtime_disabled_count != 0 || view.context_disabled_count != 0 ||
      view.window_disabled_count != 0) {
    return 6;
  }
  if (view.latest_general_count != 0 || view.latest_app_count != 1 ||
      !view.latest_app_result.handled) {
    return 8;
  }
  if (!view.runtime_enabled_result.handled ||
      !view.view_enabled_result.handled ||
      view.runtime_disabled_result.handled ||
      view.context_disabled_result.handled ||
      view.window_disabled_result.handled) {
    return 7;
  }

  return 0;
}

} // namespace

int main() {
  return test_action_enablement_metadata_controls_dispatch();
}
