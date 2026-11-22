#include "popup_test_support.hpp"
#include "cgpui/ui/workbench_widgets.hpp"

#include <algorithm>
#include <iostream>
#include <string>

namespace {

cgpui::PointerButton button(cgpui::MouseButton which, bool pressed, cgpui::Point point) {
  return {.button = which, .pressed = pressed, .click_count = 1, .position = point};
}

class OverlayView final : public cgpui::View {
 public:
  OverlayView() {
    menu.on_activate([this](std::string_view id) { activated = std::string(id); });
  }

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    tooltip.set_target(1, {{100, 100}, {40, 30}}, "Popup fallback tooltip");
    tooltip.layout(context.viewport_size, context.scale, theme);
    menu.layout(context.viewport_size, context.scale, theme);
    cgpui::WidgetTreeBuilder builder(context);
    tooltip.append_elements(builder, builder.root());
    menu.append_elements(builder, builder.root());
    builder.install(context);
    return {};
  }

  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    const std::size_t before = paint_list.commands().size();
    tooltip.paint(paint_list, theme);
    tooltip_commands = paint_list.commands().size() - before;
    const std::size_t after_tooltip = paint_list.commands().size();
    menu.paint(paint_list, theme);
    menu_commands = paint_list.commands().size() - after_tooltip;
  }

  cgpui::EventResult handle_event(const cgpui::PlatformEvent& event,
                                const cgpui::WindowRuntimeContext& context) override {
    (void)tooltip.handle_event(event, context);
    if (const auto result = menu.handle_event(event, context); result.consumed) return result;
    if (const auto* pressed = std::get_if<cgpui::PointerButton>(&event);
        pressed != nullptr && pressed->button == cgpui::MouseButton::right && pressed->pressed) {
      menu.open_at(pressed->position,
                   {cgpui::MenuItem::action("file.new", "New File"),
                    cgpui::MenuItem::action("edit.rename", "Rename")}, context);
      context.request_render();
    }
    return cgpui::EventResult::unhandled();
  }

  // Keep the default popup_window=true for both widgets. Only timer duration
  // changes so the test can drive visibility without wall-clock sleeps.
  cgpui::Tooltip tooltip{cgpui::TooltipOptions{.delay_ms = 0, .fade_ms = 0}};
  cgpui::PopoverMenu menu;
  cgpui::WorkbenchTheme theme = cgpui::workbench_theme(cgpui::SystemAppearance{});
  std::string activated;
  std::size_t tooltip_commands = 0;
  std::size_t menu_commands = 0;
};

enum class Failure {
  renderer_refusal,
  native_creation,
  child_renderer,
  placement,
};

int fallback_case(Failure scenario) {
  popup_test::Application application; // Platform supports independent popups.
  application.child_creation_succeeds = scenario != Failure::native_creation;
  application.popup_placement_succeeds = scenario != Failure::placement;
  OverlayView view;
  std::vector<std::unique_ptr<popup_test::Renderer>> renderers;
  int factory_calls = 0;
  cgpui::WindowRuntime runtime(application, view,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        if (++factory_calls > 1 && scenario == Failure::child_renderer) {
          return std::unexpected(cgpui::Error{
              .code = cgpui::ErrorCode::renderer_initialization_failed,
              .message = "test child renderer creation failure"});
        }
        renderers.push_back(std::make_unique<popup_test::Renderer>(
            scenario != Failure::renderer_refusal));
        return renderers.back().get();
      });
  int failure = 0;
  const auto drain = [&] { application.root.callback(cgpui::WindowWakeupRequested{}); };
  const auto frame = [&] {
    application.root.callback(cgpui::WindowRedrawRequested{});
    drain(); // Placement failure closes the newly opened popup through defer.
    drain();
  };
  const auto no_popup = [&] {
    return std::none_of(application.children.begin(), application.children.end(),
                        [](const auto& child) { return child->open; }) &&
           runtime.app_opened_windows().empty() && runtime.window_runtime_records().size() == 1;
  };
  const auto key = [&](std::uint32_t code) {
    application.root.callback(cgpui::KeyboardKey{.key_code = code,
                                                .action = cgpui::KeyAction::pressed});
  };
  application.on_run = [&] {
    if (cgpui::popup_windows_supported(runtime) !=
        (scenario != Failure::renderer_refusal)) { failure = 8; return; }
    application.root.callback(cgpui::WindowFocused{.focused = true});
    frame();
    application.root.callback(cgpui::PointerMoved{.position = {110, 110}});
    runtime.advance_time(0);
    frame();
    if (!view.tooltip.visible() || view.tooltip.opacity() != 1.0F ||
        view.tooltip_commands == 0 || !no_popup()) { failure = 1; return; }
    key(27); // The visible overlay tooltip can still be dismissed.
    frame();
    if (view.tooltip.visible() || view.tooltip_commands != 0 || !no_popup()) { failure = 2; return; }

    application.root.callback(button(cgpui::MouseButton::right, true, {50, 60}));
    frame();
    if (!view.menu.is_open() || view.menu_commands == 0 ||
        !no_popup()) { failure = 3; return; }
    const auto item = view.menu.item_rect(0, 1);
    const cgpui::Point at{item.origin.x + 10, item.origin.y + 5};
    application.root.callback(cgpui::PointerMoved{.position = at});
    application.root.callback(button(cgpui::MouseButton::left, true, at));
    application.root.callback(button(cgpui::MouseButton::left, false, at));
    if (view.activated != "edit.rename" || view.menu.is_open()) { failure = 4; return; }

    view.activated.clear();
    application.root.callback(button(cgpui::MouseButton::right, true, {50, 60}));
    frame();
    key(40); // Down selects the first item; Enter activates it.
    key(13);
    if (view.activated != "file.new" || view.menu.is_open() ||
        !no_popup()) { failure = 5; return; }

    view.activated.clear();
    application.root.callback(button(cgpui::MouseButton::right, true, {50, 60}));
    frame();
    key(27);
    frame();
    if (view.menu.is_open() || !view.activated.empty() || view.menu_commands != 0 ||
        !no_popup()) { failure = 6; return; }

    // Prove each fallback happened for the requested reason, rather than
    // accidentally disabling popup preference or all platform support.
    const bool attempted = application.child_creation_attempts > 0;
    if (scenario == Failure::renderer_refusal) {
      if (attempted || factory_calls != 1) failure = 7;
    } else if (!attempted) {
      failure = 7;
    } else if (scenario == Failure::native_creation) {
      if (!application.children.empty() || factory_calls != 1) failure = 7;
    } else if (scenario == Failure::child_renderer) {
      if (application.children.empty() || factory_calls <= 1 || renderers.size() != 1) failure = 7;
    } else if (application.children.empty() || renderers.size() <= 1 ||
               application.children.front()->placement_attempts == 0) {
      failure = 7;
    }
  };
  const int result = runtime.run({.size = {640, 480}}, {.request_initial_redraw = false});
  if (result != 0 || failure != 0) {
    std::cerr << "workbench popup fallback scenario " << static_cast<int>(scenario)
              << " failed: " << failure << ", runtime: " << result << '\n';
  }
  return result == 0 ? failure : 10;
}

} // namespace

int main() {
  for (const auto scenario : {Failure::renderer_refusal, Failure::native_creation,
                              Failure::child_renderer, Failure::placement}) {
    if (const int result = fallback_case(scenario); result != 0) return result;
  }
  return 0;
}
