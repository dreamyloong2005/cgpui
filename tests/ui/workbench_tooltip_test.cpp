#include "cgpui/ui/tooltip.hpp"

#include "workbench_test_support.hpp"

#include <iostream>

namespace {

constexpr cgpui::Size window_size{800.0F, 500.0F};
const cgpui::Rect button{{100, 100}, {40, 30}};
const cgpui::Rect corner_button{{770, 470}, {30, 30}};

struct Fixture {
  cgpui::TestApp app;
  cgpui::Tooltip tooltip;
  cgpui::WorkbenchTheme theme;
  cgpui::TestAppWindow window;

  explicit Fixture(bool reduced_motion,
                   cgpui::TooltipPosition position = cgpui::TooltipPosition::pointer)
      : tooltip(cgpui::TooltipOptions{.position = position}),
        window(app.open_window(cgpui::WindowOptions{}.size(window_size.width,
                                                           window_size.height),
                               std::make_unique<workbench_test::HostView>())) {
    cgpui::SystemAppearance appearance;
    appearance.client_area_animation_enabled = !reduced_motion;
    theme = cgpui::workbench_theme(appearance);
    auto* view = window.root_view_as<workbench_test::HostView>();
    view->build = [this](cgpui::WidgetTreeBuilder& builder,
                         const cgpui::ViewContext& context) {
      tooltip.set_target(1, button, "Toggle the agent panel");
      tooltip.set_target(2, corner_button, "Close window");
      tooltip.layout(context.viewport_size, context.scale, theme);
      tooltip.append_elements(builder, builder.root());
    };
    view->on_event = [this](const cgpui::PlatformEvent& event,
                            const cgpui::ViewContext& context) {
      return tooltip.handle_event(event, context);
    };
    window.draw_frame();
  }

  void show_over(cgpui::Point point) {
    window.dispatch_pointer_move(point);
    app.advance_time(500);
    window.draw_frame();
  }
};

int test_delay_and_fade() {
  Fixture f(false);
  f.window.dispatch_pointer_move({110, 110});
  f.app.advance_time(499);
  if (f.tooltip.visible() || !f.tooltip.pending()) return 10;
  f.app.advance_time(1);
  if (!f.tooltip.visible() || f.tooltip.opacity() >= 1.0F) return 11;
  f.app.advance_time(200);
  if (f.tooltip.opacity() != 1.0F) return 12;
  f.window.draw_frame();
  // Pointer placement: below the pointer, inside the window.
  const cgpui::Rect bounds = f.tooltip.bounds();
  if (bounds.origin.y < 110.0F + 15.0F || bounds.size.width <= 12.0F ||
      f.tooltip.flipped()) {
    return 13;
  }
  return 0;
}

int test_reduced_motion_skips_fade() {
  Fixture f(true);
  f.window.dispatch_pointer_move({110, 110});
  f.app.advance_time(500);
  return f.tooltip.visible() && f.tooltip.opacity() == 1.0F ? 0 : 20;
}

int test_hide_triggers() {
  Fixture f(true);
  f.show_over({110, 110});
  f.window.dispatch_pointer_move({300, 300});  // leave
  if (f.tooltip.visible() || f.tooltip.pending()) return 30;
  f.show_over({110, 110});
  workbench_test::click(f.window, {112, 112});  // press
  if (f.tooltip.visible()) return 31;
  f.window.dispatch_pointer_move({114, 114});  // still over the target
  f.app.advance_time(1000);
  if (f.tooltip.visible()) return 32;  // stays hidden until the pointer leaves
  f.window.dispatch_pointer_move({300, 300});
  f.show_over({110, 110});
  if (!f.tooltip.visible()) return 33;
  f.window.dispatch_pointer_scroll({0, -1}, {110, 110});
  if (f.tooltip.visible()) return 34;
  f.window.dispatch_pointer_move({300, 300});
  f.show_over({110, 110});
  workbench_test::key(f.window, 27);  // Escape
  if (f.tooltip.visible()) return 35;
  f.window.dispatch_pointer_move({300, 300});
  f.show_over({110, 110});
  (void)f.window.dispatch_platform_event(cgpui::PointerExited{{110, 110}});
  if (f.tooltip.visible()) return 36;
  // Leaving before the delay cancels the pending tooltip.
  f.window.dispatch_pointer_move({110, 110});
  f.app.advance_time(300);
  f.window.dispatch_pointer_move({300, 300});
  f.app.advance_time(500);
  return f.tooltip.visible() ? 37 : 0;
}

int test_flips_at_window_edges() {
  Fixture f(true);
  f.show_over({785, 490});
  const cgpui::Rect bounds = f.tooltip.bounds();
  if (!f.tooltip.flipped() || bounds.origin.y + bounds.size.height > 490.0F) return 40;
  if (bounds.origin.x + bounds.size.width > window_size.width - 4.0F + 0.01F) return 41;
  Fixture anchored(true, cgpui::TooltipPosition::anchor);
  anchored.show_over({110, 110});
  const cgpui::Rect below = anchored.tooltip.bounds();
  if (anchored.tooltip.flipped() || below.origin.y != 136.0F || below.origin.x != 100.0F) {
    return 42;
  }
  return 0;
}

int test_accessibility() {
  Fixture f(true);
  f.show_over({110, 110});
  const cgpui::AccessibilityTreeSnapshot snapshot = workbench_test::snapshot_of(
      window_size, std::nullopt, [&](cgpui::WidgetTreeBuilder& builder) {
        f.tooltip.append_elements(builder, builder.root());
      });
  const auto* node =
      workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tooltip);
  if (node == nullptr || node->name != "Toggle the agent panel" || node->focusable) {
    return 50;
  }
  return 0;
}

} // namespace

int main() {
  for (const auto test : {test_delay_and_fade, test_reduced_motion_skips_fade,
                          test_hide_triggers, test_flips_at_window_edges,
                          test_accessibility}) {
    if (const int result = test(); result != 0) {
      std::cerr << "workbench_tooltip_test failed: " << result << '\n';
      return result;
    }
  }
  return 0;
}
