// A secondary window (every TestApp window is a window runtime record)
// owns and publishes its element tree like the root window: platform
// accessibility nodes, click routing by hit test, cursor from the hit
// element and focus on press.
#include "cgpui/ui/splitter.hpp"
#include "cgpui/ui/tree_view.hpp"

#include "workbench_test_support.hpp"

#include <memory>
#include <optional>

namespace {

using cgpui::AccessibilityRole;
using cgpui::ElementId;

struct Fixture {
  cgpui::TestApp app;
  cgpui::Splitter splitter{
      cgpui::SplitterOptions{.name = "Panels"},
      {cgpui::SplitterPaneOptions{.default_fraction = 0.5F},
       cgpui::SplitterPaneOptions{.default_fraction = 0.5F}}};
  cgpui::TreeView tree;
  cgpui::Rect button_bounds;
  ElementId button_id;
  cgpui::TestAppWindow window;
  workbench_test::HostView* view = nullptr;

  Fixture()
      : window(app.open_window(cgpui::WindowOptions{}.size(600.0F, 400.0F),
                               std::make_unique<workbench_test::HostView>())) {
    tree.set_roots({cgpui::TreeItem{.id = 1, .label = "src", .has_children = true},
                    cgpui::TreeItem{.id = 2, .label = "README.md"}});
    view = window.root_view_as<workbench_test::HostView>();
    view->build = [this](cgpui::WidgetTreeBuilder& builder,
                         const cgpui::ViewContext& context) {
      splitter.layout(cgpui::Rect{{0, 0}, context.viewport_size}, context.scale);
      tree.layout(splitter.pane_rect(0), context.scale);
      const cgpui::Rect right = splitter.pane_rect(1);
      button_bounds = cgpui::Rect{{right.origin.x + 20, right.origin.y + 20}, {80, 24}};
      splitter.append_elements(builder, builder.root());
      tree.append_elements(builder, builder.root());
      builder.add(builder.root(),
                  cgpui::WidgetNode{.role = AccessibilityRole::button,
                                    .name = "Run",
                                    .bounds = button_bounds,
                                    .focusable = true,
                                    .hit_testable = true},
                  true, &button_id);
    };
    view->on_event = [this](const cgpui::PlatformEvent& event,
                            const cgpui::ViewContext& context) {
      if (splitter.handle_event(event, context).consumed) {
        return cgpui::EventResult::consumed_event();
      }
      return tree.handle_event(event, context);
    };
    window.draw_frame();
  }

  [[nodiscard]] const cgpui::WindowRuntimeRecord* record() const {
    return view->runtime->window_runtime_record(window.runtime_id());
  }
};

bool has_role(const cgpui::PlatformAccessibilityTreeUpdate& update,
              cgpui::PlatformAccessibilityRole role) {
  for (const auto& node : update.nodes) {
    if (node.role == role) return true;
  }
  return false;
}

} // namespace

namespace {

int test_record_publishes_accessibility_nodes() {
  Fixture f;
  if (f.window.runtime_id() == f.view->runtime->root_window_runtime_id()) return 10;
  const cgpui::WindowRuntimeRecord* record = f.record();
  if (record == nullptr || !record->last_accessibility_update.has_value()) return 11;
  const auto& update = *record->last_accessibility_update;
  if (!has_role(update, cgpui::PlatformAccessibilityRole::button) ||
      !has_role(update, cgpui::PlatformAccessibilityRole::tree) ||
      !has_role(update, cgpui::PlatformAccessibilityRole::tree_item) ||
      !has_role(update, cgpui::PlatformAccessibilityRole::separator)) {
    return 12;
  }
  for (const auto& node : update.nodes) {
    if (node.element_id == f.button_id.value) {
      return node.bounds.has_value() && node.bounds->origin.x == f.button_bounds.origin.x &&
                     node.bounds->origin.y == f.button_bounds.origin.y
                 ? 0
                 : 13;
    }
  }
  return 14;
}

int test_record_routes_clicks_by_hit_test() {
  Fixture f;
  const cgpui::Point button = workbench_test::center(f.button_bounds);
  f.window.dispatch_pointer_button(cgpui::MouseButton::left, true, button);
  const cgpui::WindowRuntimeRecord* record = f.record();
  if (!record->last_event_dispatch.has_value()) return 20;
  if (record->last_event_dispatch->route.target_element_id != f.button_id) return 21;
  if (f.window.input_state().keyboard_focus_element_owner != f.button_id) return 22;
  f.window.dispatch_pointer_button(cgpui::MouseButton::left, false, button);

  const cgpui::Point row = workbench_test::center(f.tree.row_rect(1));
  f.window.dispatch_pointer_button(cgpui::MouseButton::left, true, row);
  // Rows are accessibility-only nodes; the click lands on the tree node,
  // whose ancestry ends at the window root.
  const auto& route = f.record()->last_event_dispatch->route;
  if (route.target_element_id != f.tree.element_id()) return 23;
  if (route.element_ancestry.size() != 2) return 24;
  if (f.window.input_state().keyboard_focus_element_owner != f.tree.element_id()) return 26;
  return f.tree.selected(2) ? 0 : 25;
}

int test_record_sets_resize_cursor_over_divider() {
  Fixture f;
  const cgpui::Point divider = workbench_test::center(f.splitter.divider_hit_rect(0));
  f.window.dispatch_pointer_move(divider);
  if (f.window.input_state().cursor_shape != cgpui::CursorShape::resize_left_right) {
    return 30;
  }
  if (!f.window.input_state().hovered_element_id.has_value()) return 31;
  f.window.dispatch_pointer_move(workbench_test::center(f.button_bounds));
  if (f.window.input_state().cursor_shape != cgpui::CursorShape::default_arrow) return 32;
  return f.window.input_state().hovered_element_id == f.button_id ? 0 : 33;
}

} // namespace

int main() {
  if (const int result = test_record_publishes_accessibility_nodes(); result != 0) {
    return result;
  }
  if (const int result = test_record_routes_clicks_by_hit_test(); result != 0) {
    return result;
  }
  return test_record_sets_resize_cursor_over_divider();
}
