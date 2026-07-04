#include "window_runtime_test_support.hpp"

#include "cgpui/platform/target.hpp"
#include "cgpui/ui/key_binding.hpp"

#include <optional>
#include <variant>

namespace {

RuntimeFixture* platform_modifier_fixture = nullptr;

void dispatch_platform_modifier_sequence() {
  auto& callback = platform_modifier_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 'I',
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 'S',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.super = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'S',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'P',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'P',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.super = true}});
}

class PlatformModifierView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
    if (key == nullptr || key->key_code != 'I') {
      return cgpui::EventResult::unhandled();
    }

    context.register_action(
        "platform.secondary",
        [this](const cgpui::WindowRuntimeContext&) {
          secondary_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_action(
        "platform.primary",
        [this](const cgpui::WindowRuntimeContext&) {
          platform_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    secondary_bound =
        context.bind_key("secondary-s", "platform.secondary");
    platform_bound = context.bind_key("platform-p", "platform.primary");
    return cgpui::EventResult::unhandled();
  }

  bool secondary_bound = false;
  bool platform_bound = false;
  int secondary_count = 0;
  int platform_count = 0;
};

int test_platform_modifier_parser_normalizes_secondary() {
  const std::optional<cgpui::KeyBinding> windows_secondary =
      cgpui::parse_key_binding(
          "secondary-s",
          "platform.secondary",
          cgpui::DesktopPlatformTarget::windows);
  if (!windows_secondary.has_value() ||
      windows_secondary->key_code != 'S' ||
      !windows_secondary->modifiers.control ||
      windows_secondary->modifiers.super ||
      windows_secondary->modifiers.alt ||
      windows_secondary->modifiers.shift) {
    return 1;
  }

  const std::optional<cgpui::KeyBinding> linux_secondary =
      cgpui::parse_key_binding(
          "secondary-s",
          "platform.secondary",
          cgpui::DesktopPlatformTarget::linux_wayland);
  if (!linux_secondary.has_value() ||
      !linux_secondary->modifiers.control ||
      linux_secondary->modifiers.super) {
    return 2;
  }

  const std::optional<cgpui::KeyBinding> mac_secondary =
      cgpui::parse_key_binding(
          "secondary-s",
          "platform.secondary",
          cgpui::DesktopPlatformTarget::macos_cocoa);
  if (!mac_secondary.has_value() ||
      mac_secondary->modifiers.control ||
      !mac_secondary->modifiers.super) {
    return 3;
  }

  const std::optional<cgpui::KeyBinding> platform =
      cgpui::parse_key_binding(
          "platform-p",
          "platform.primary",
          cgpui::DesktopPlatformTarget::windows);
  if (!platform.has_value() ||
      platform->key_code != 'P' ||
      platform->modifiers.control ||
      !platform->modifiers.super) {
    return 4;
  }

  const std::optional<cgpui::KeyBinding> win =
      cgpui::parse_key_binding(
          "win-p",
          "platform.primary",
          cgpui::DesktopPlatformTarget::windows);
  if (!win.has_value() ||
      win->modifiers.control ||
      !win->modifiers.super) {
    return 5;
  }

  if (cgpui::parse_key_binding(
          "ctrl-secondary-s",
          "platform.duplicate",
          cgpui::DesktopPlatformTarget::windows)
          .has_value()) {
    return 6;
  }

  return 0;
}

int test_context_bind_key_uses_current_platform_semantics() {
  RuntimeFixture fixture;
  PlatformModifierView view;
  platform_modifier_fixture = &fixture;
  fixture.app.on_run = &dispatch_platform_modifier_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  platform_modifier_fixture = nullptr;

  if (result != 0) {
    return 7;
  }
  if (!view.secondary_bound || !view.platform_bound) {
    return 8;
  }
  if (view.secondary_count != 1 || view.platform_count != 1) {
    return 9;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_platform_modifier_parser_normalizes_secondary();
      result != 0) {
    return result;
  }
  return test_context_bind_key_uses_current_platform_semantics();
}
