#include "cgpui/ui/splitter.hpp"

#include "workbench_test_support.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

using cgpui::Splitter;
using cgpui::SplitterPaneOptions;

struct Fixture {
  cgpui::TestApp app;
  Splitter splitter{
      cgpui::SplitterOptions{.name = "Workbench"},
      {SplitterPaneOptions{.default_fraction = 0.25F, .min_size = 120.0F,
                           .max_size = 400.0F, .collapsible = true},
       SplitterPaneOptions{.default_fraction = 0.5F, .min_size = 200.0F},
       SplitterPaneOptions{.default_fraction = 0.25F, .min_size = 160.0F,
                           .collapsible = true}}};
  std::vector<cgpui::SplitterChange> changes;
  std::vector<std::vector<float>> fractions;
  cgpui::TestAppWindow window;
  workbench_test::HostView* view = nullptr;

  Fixture()
      : window(app.open_window(cgpui::WindowOptions{}.size(1000.0F, 600.0F),
                               std::make_unique<workbench_test::HostView>())) {
    view = window.root_view_as<workbench_test::HostView>();
    view->build = [this](cgpui::WidgetTreeBuilder& builder,
                         const cgpui::ViewContext& context) {
      splitter.layout(cgpui::Rect{{0, 40}, {context.viewport_size.width,
                                            context.viewport_size.height - 40}},
                      context.scale);
      splitter.append_elements(builder, builder.root());
    };
    view->on_event = [this](const cgpui::PlatformEvent& event,
                            const cgpui::ViewContext& context) {
      return splitter.handle_event(event, context);
    };
    splitter.on_change([this](const cgpui::SplitterChange& change) {
      changes.push_back(change);
      fractions.emplace_back(change.fractions.begin(), change.fractions.end());
    });
    window.draw_frame();
  }
};

bool near(float lhs, float rhs, float tolerance = 0.01F) {
  return std::fabs(lhs - rhs) <= tolerance;
}

bool integral(float logical, float scale) {
  const float device = logical * scale;
  return std::fabs(device - std::round(device)) < 0.001F;
}

int test_layout_and_dpi() {
  Fixture f;
  const float w0 = f.splitter.pane_rect(0).size.width;
  const float w1 = f.splitter.pane_rect(1).size.width;
  if (!near(w0, 250.0F, 1.0F) || !near(w1, 499.0F, 1.0F)) return 10;
  if (f.splitter.divider_rect(0).size.width != 1.0F) return 11;
  if (f.splitter.divider_hit_rect(0).size.width != 8.0F) return 12;
  f.window.simulate_resize(cgpui::Size{1500.0F, 900.0F}, cgpui::DpiScale{1.5F});
  f.window.draw_frame();
  for (std::size_t pane = 0; pane < 3; ++pane) {
    const cgpui::Rect rect = f.splitter.pane_rect(pane);
    if (!integral(rect.origin.x, 1.5F) ||
        !integral(rect.origin.x + rect.size.width, 1.5F)) {
      return 13;
    }
  }
  // One logical pixel at 150% rounds to two device pixels.
  if (!near(f.splitter.divider_rect(0).size.width * 1.5F, 2.0F, 0.001F)) return 14;
  return 0;
}

int test_pointer_drag() {
  Fixture f;
  const cgpui::Rect line = f.splitter.divider_rect(0);
  const float start = f.splitter.pane_rect(0).size.width;
  // Press 3 px left of the visible line: inside the wider hit area.
  const cgpui::Point press{line.origin.x - 3.0F, 300.0F};
  (void)f.window.dispatch_platform_event(cgpui::PointerButton{
      .button = cgpui::MouseButton::left, .pressed = true, .position = press});
  const cgpui::ViewInputState pressed = f.window.input_state();
  if (f.splitter.dragging_divider() != 0 || !pressed.pointer_captured) return 20;
  if (pressed.keyboard_focus_element_owner != f.splitter.divider_element_id(0)) {
    return 21;
  }
  f.window.dispatch_pointer_move({press.x + 50.0F, 310.0F});
  if (!near(f.splitter.pane_rect(0).size.width, start + 50.0F, 1.0F)) return 22;
  if (f.changes.empty() || f.changes.back().finished) return 23;
  f.window.dispatch_pointer_move({100.0F, 310.0F});
  if (!near(f.splitter.pane_rect(0).size.width, 120.0F, 1.0F)) return 24;
  f.window.dispatch_pointer_move({700.0F, 310.0F});
  if (!near(f.splitter.pane_rect(0).size.width, 400.0F, 1.0F)) return 25;
  f.window.dispatch_pointer_button(cgpui::MouseButton::left, false, {700.0F, 310.0F});
  if (f.splitter.dragging_divider().has_value() || !f.changes.back().finished ||
      f.window.input_state().pointer_captured) {
    return 26;
  }
  if (!near(f.fractions.back()[0] * 998.0F, 400.0F, 1.5F)) return 27;
  // Hit testing through the built tree: 3 px beside the line is the
  // divider (resize cursor), the pane interior is not.
  cgpui::WidgetTreeBuilder builder(cgpui::Size{1000.0F, 600.0F});
  f.splitter.append_elements(builder, builder.root());
  const std::unique_ptr<cgpui::ElementTree> tree = builder.finish();
  const cgpui::Rect moved = f.splitter.divider_rect(0);
  const cgpui::ElementId hit =
      tree->hit_test_root({moved.origin.x + 3.0F, 300.0F});
  const auto* node = tree->find_as<cgpui::WidgetNodeElement>(hit);
  if (hit != f.splitter.divider_element_id(0) || node == nullptr ||
      node->node().cursor != cgpui::CursorShape::resize_left_right) {
    return 28;
  }
  if (tree->hit_test_root({200.0F, 300.0F}) != tree->root_id()) return 29;
  return 0;
}

int test_collapse_and_restore() {
  Fixture f;
  const cgpui::Rect line = f.splitter.divider_rect(1);
  (void)f.window.dispatch_platform_event(cgpui::PointerButton{
      .button = cgpui::MouseButton::left, .pressed = true,
      .position = workbench_test::center(line)});
  f.window.dispatch_pointer_move({985.0F, 300.0F});
  f.window.dispatch_pointer_button(cgpui::MouseButton::left, false, {985.0F, 300.0F});
  if (!f.splitter.collapsed(2) || f.splitter.pane_rect(2).size.width != 0.0F) {
    return 30;
  }
  f.window.draw_frame();
  workbench_test::click(f.window, workbench_test::center(f.splitter.divider_rect(0)),
                        cgpui::MouseButton::left, 2);
  if (f.splitter.collapsed(2) || f.changes.back().source !=
                                     cgpui::SplitterChangeSource::restore) {
    return 31;
  }
  if (!near(f.splitter.fractions()[0], 0.25F) || !near(f.splitter.fractions()[1], 0.5F)) {
    return 32;
  }
  return 0;
}

int test_keyboard() {
  using namespace workbench_test;
  Fixture f;
  f.window.focus(f.splitter.divider_element_id(0));
  const float start = f.splitter.pane_rect(0).size.width;
  key(f.window, 39);
  if (!near(f.splitter.pane_rect(0).size.width, start + 16.0F, 1.0F)) return 40;
  key(f.window, 39, {.shift = true});
  if (!near(f.splitter.pane_rect(0).size.width, start + 80.0F, 1.0F)) return 41;
  key(f.window, 37);
  if (!near(f.splitter.pane_rect(0).size.width, start + 64.0F, 1.0F)) return 42;
  key(f.window, 13);
  if (!f.splitter.collapsed(0)) return 43;
  key(f.window, 13);
  if (f.splitter.collapsed(0) || f.changes.back().source !=
                                     cgpui::SplitterChangeSource::collapse) {
    return 44;
  }
  // Keys go nowhere once the divider has lost focus.
  f.window.release_focus(f.splitter.divider_element_id(0));
  const float before = f.splitter.pane_rect(0).size.width;
  key(f.window, 39);
  if (f.splitter.pane_rect(0).size.width != before) return 45;
  return 0;
}

int test_accessibility() {
  Fixture f;
  f.window.focus(f.splitter.divider_element_id(1));
  const cgpui::AccessibilityTreeSnapshot snapshot = workbench_test::snapshot_of(
      {1000.0F, 600.0F}, f.splitter.divider_element_id(1),
      [&](cgpui::WidgetTreeBuilder& builder) {
        f.splitter.append_elements(builder, builder.root());
      });
  if (workbench_test::count_role(snapshot, cgpui::AccessibilityRole::separator) != 2) {
    return 50;
  }
  const auto* second =
      workbench_test::find_role(snapshot, cgpui::AccessibilityRole::separator, 1);
  if (second == nullptr || !second->focusable || !second->focused ||
      !second->patterns.range.has_value() || second->name != "Workbench divider 2" ||
      second->value.empty()) {
    return 51;
  }
  if (f.splitter.focused_divider() != 1) return 52;
  return 0;
}

} // namespace

int main() {
  for (const auto test : {test_layout_and_dpi, test_pointer_drag,
                          test_collapse_and_restore, test_keyboard,
                          test_accessibility}) {
    if (const int result = test(); result != 0) {
      std::cerr << "workbench_splitter_test failed: " << result << '\n';
      return result;
    }
  }
  return 0;
}
