#include "cgpui/ui/popover_menu.hpp"
#include "cgpui/ui/text_measurement.hpp"

#include "workbench_test_support.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

using cgpui::MenuItem;
using cgpui::PopoverCloseReason;

constexpr cgpui::Size window_size{800.0F, 500.0F};

std::vector<MenuItem> items() {
  return {MenuItem::action("file.open", "Open", "Ctrl+O"),
          MenuItem::action("file.rename", "Rename Symbol", "F2"),
          MenuItem::action("file.delete", "Delete", "Del", false),
          MenuItem::separator(),
          MenuItem::submenu("New", {MenuItem::action("new.file", "File", "Ctrl+N"),
                                    MenuItem::action("new.folder", "Folder")})};
}

// A focusable "content" node stands in for the element that had focus
// before the menu opened; right-clicks that reach the content open the menu.
struct Fixture {
  cgpui::TestApp app;
  cgpui::PopoverMenu menu;
  cgpui::WorkbenchTheme theme = cgpui::workbench_theme(cgpui::SystemAppearance{});
  cgpui::ElementId content_id;
  std::vector<std::string> activated;
  std::vector<PopoverCloseReason> closed;
  int content_presses = 0;
  cgpui::TestAppWindow window;

  Fixture()
      : window(app.open_window(cgpui::WindowOptions{}.size(window_size.width,
                                                           window_size.height),
                               std::make_unique<workbench_test::HostView>())) {
    auto* view = window.root_view_as<workbench_test::HostView>();
    view->build = [this](cgpui::WidgetTreeBuilder& builder,
                         const cgpui::ViewContext& context) {
      builder.add(builder.root(),
                  cgpui::WidgetNode{.bounds = {{0, 0}, window_size},
                                    .focusable = true, .hit_testable = true},
                  true, &content_id);
      menu.layout(context.viewport_size, context.scale, theme);
      menu.append_elements(builder, builder.root());
    };
    view->on_event = [this](const cgpui::PlatformEvent& event,
                            const cgpui::ViewContext& context) {
      if (const cgpui::EventResult result = menu.handle_event(event, context);
          result.consumed) {
        return result;
      }
      if (const auto* button = std::get_if<cgpui::PointerButton>(&event);
          button != nullptr && button->pressed) {
        content_presses += 1;
        if (button->button == cgpui::MouseButton::right) {
          menu.open_at(button->position, items(), context);
        }
      }
      return cgpui::EventResult::unhandled();
    };
    menu.on_activate([this](std::string_view id) { activated.emplace_back(id); });
    menu.on_close([this](PopoverCloseReason reason) { closed.push_back(reason); });
    window.draw_frame();
    window.focus(content_id);
  }

  void open_at(cgpui::Point point) {
    (void)window.dispatch_platform_event(cgpui::PointerButton{
        .button = cgpui::MouseButton::right, .pressed = true, .position = point});
    (void)window.dispatch_platform_event(cgpui::PointerButton{
        .button = cgpui::MouseButton::right, .pressed = false, .position = point});
    window.draw_frame();
  }

  [[nodiscard]] cgpui::ElementId focus() const {
    return window.input_state().keyboard_focus_element_owner.value_or(cgpui::ElementId{});
  }
};

int test_open_focus_and_escape_restore() {
  Fixture f;
  f.open_at({100, 100});
  if (!f.menu.is_open() || f.menu.level_bounds(0).origin.x != 100.0F ||
      f.menu.level_bounds(0).origin.y != 100.0F) {
    return 10;
  }
  if (f.menu.element_id().value == 0 || f.focus() != f.menu.element_id()) return 11;
  workbench_test::key(f.window, 27);
  if (f.menu.is_open() || f.closed.back() != PopoverCloseReason::escape) return 12;
  if (f.focus() != f.content_id) return 13;
  return 0;
}

int test_flips_and_shifts_at_edges() {
  Fixture f;
  f.open_at({790, 490});
  const cgpui::Rect bounds = f.menu.level_bounds(0);
  if (!f.menu.flipped_x(0) || !f.menu.flipped_y(0)) return 20;
  if (bounds.origin.x + bounds.size.width > 790.0F + 0.01F ||
      bounds.origin.y + bounds.size.height > 490.0F + 0.01F) {
    return 21;
  }
  // A submenu that would leave the window on the right opens to the left.
  workbench_test::key(f.window, 35);  // End: the "New" submenu
  workbench_test::key(f.window, 39);  // Right: open it
  const cgpui::Rect submenu = f.menu.level_bounds(1);
  if (f.menu.level_count() != 2 || !f.menu.flipped_x(1) ||
      submenu.origin.x + submenu.size.width > bounds.origin.x + 0.01F ||
      submenu.origin.x < 4.0F) {
    return 22;
  }
  return 0;
}

int test_outside_click_dismisses() {
  Fixture f;
  f.open_at({100, 100});
  const int presses = f.content_presses;
  (void)f.window.dispatch_platform_event(cgpui::PointerButton{
      .button = cgpui::MouseButton::left, .pressed = true, .position = {600, 400}});
  if (f.menu.is_open() || f.closed.back() != PopoverCloseReason::outside_click) return 30;
  if (f.content_presses != presses) return 31;  // the dismissing press is consumed
  if (f.focus() != f.content_id) return 32;
  // A click inside the surface but on padding keeps the menu open.
  f.open_at({100, 100});
  const cgpui::Rect bounds = f.menu.level_bounds(0);
  workbench_test::click(f.window, {bounds.origin.x + 2.0F, bounds.origin.y + 1.0F});
  return f.menu.is_open() ? 0 : 33;
}

int test_keyboard_navigation_and_submenu() {
  using workbench_test::key;
  Fixture f;
  f.open_at({100, 100});
  key(f.window, 40);
  if (f.menu.highlighted(0) != 0) return 40;
  key(f.window, 40);
  key(f.window, 40);  // skips the disabled item and the separator
  if (f.menu.highlighted(0) != 4) return 41;
  key(f.window, 40);  // wraps
  if (f.menu.highlighted(0) != 0) return 42;
  key(f.window, 38);
  key(f.window, 39);  // opens "New", first item highlighted
  if (f.menu.level_count() != 2 || f.menu.highlighted(1) != 0) return 43;
  key(f.window, 37);  // Left closes the submenu only
  if (f.menu.level_count() != 1 || !f.menu.is_open()) return 44;
  key(f.window, 13);  // Enter on a submenu opens it
  key(f.window, 40);
  key(f.window, 13);
  if (f.menu.is_open() || f.activated != std::vector<std::string>{"new.folder"} ||
      f.closed.back() != PopoverCloseReason::activated || f.focus() != f.content_id) {
    return 45;
  }
  return 0;
}

int test_pointer_hover_and_activation() {
  using workbench_test::center;
  Fixture f;
  f.open_at({100, 100});
  f.window.dispatch_pointer_move(center(f.menu.item_rect(0, 4)));
  if (f.menu.level_count() != 2 || f.menu.highlighted(0) != 4) return 50;
  f.window.draw_frame();
  // Disabled items neither highlight nor activate.
  workbench_test::click(f.window, center(f.menu.item_rect(0, 2)));
  if (!f.menu.is_open() || !f.activated.empty()) return 51;
  f.window.dispatch_pointer_move(center(f.menu.item_rect(0, 4)));
  workbench_test::click(f.window, center(f.menu.item_rect(1, 0)));
  if (f.menu.is_open() || f.activated != std::vector<std::string>{"new.file"}) return 52;
  return 0;
}

int test_accelerator_column_uses_text_metrics() {
  Fixture f;
  f.open_at({100, 100});
  const std::vector<MenuItem> menu = items();
  float widest = 0.0F;
  for (const MenuItem& item : menu) {
    if (item.kind == cgpui::MenuItemKind::separator) continue;
    widest = std::max(widest, cgpui::measure_text(item.label, f.theme.font,
                                                  f.theme.font_size, {})
                                  .logical_size.width);
  }
  if (widest <= 0.0F || f.menu.label_column_width(0) != widest) return 60;
  const cgpui::Rect bounds = f.menu.level_bounds(0);
  // padding 4 + item padding 10 + widest label + gap 32 <= accelerator column.
  if (f.menu.accelerator_column_x(0) + 0.01F < bounds.origin.x + 14.0F + widest + 32.0F) {
    return 61;
  }
  const float ctrl_o =
      cgpui::measure_text("Ctrl+O", f.theme.font, f.theme.font_size, {}).logical_size.width;
  if (f.menu.accelerator_width(0, 0) != ctrl_o || ctrl_o <= 0.0F) return 62;
  return 0;
}

int test_accessibility() {
  Fixture f;
  f.open_at({100, 100});
  workbench_test::key(f.window, 35);
  workbench_test::key(f.window, 39);
  const cgpui::AccessibilityTreeSnapshot snapshot = workbench_test::snapshot_of(
      window_size, f.menu.element_id(), [&](cgpui::WidgetTreeBuilder& builder) {
        // Same stable nodes in the same order as the window, so ids match.
        builder.add(builder.root(), cgpui::WidgetNode{}, true);
        f.menu.append_elements(builder, builder.root());
      });
  const auto* root = workbench_test::find_role(snapshot, cgpui::AccessibilityRole::menu);
  if (root == nullptr || !root->focusable || !root->focused) return 70;
  if (workbench_test::count_role(snapshot, cgpui::AccessibilityRole::menu) != 2 ||
      workbench_test::count_role(snapshot, cgpui::AccessibilityRole::menu_item) != 6 ||
      workbench_test::count_role(snapshot, cgpui::AccessibilityRole::separator) != 1) {
    return 71;
  }
  const auto* disabled =
      workbench_test::find_role(snapshot, cgpui::AccessibilityRole::menu_item, 2);
  const auto* submenu =
      workbench_test::find_role(snapshot, cgpui::AccessibilityRole::menu_item, 3);
  if (disabled == nullptr || disabled->enabled || disabled->value != "Del" ||
      submenu == nullptr || submenu->patterns.expanded != true ||
      submenu->children.size() != 1) {
    return 72;
  }
  return 0;
}

} // namespace

// Without backdrop blur the surface is opaque, so content under the menu
// does not show through (even with a translucent theme override).
int test_surface_is_opaque_without_blur() {
  for (const float override_alpha : {1.0F, 0.5F}) {
    Fixture f;
    f.theme.menu.a = override_alpha == 1.0F ? f.theme.menu.a : override_alpha;
    f.open_at({100, 100});
    cgpui::PaintList paint_list;
    f.menu.paint(paint_list, f.theme);
    const cgpui::Rect bounds = f.menu.level_bounds(0);
    bool found = false;
    for (const cgpui::PaintCommand& command : paint_list.commands()) {
      if (command.kind != cgpui::PaintCommandKind::rounded_rect) continue;
      const cgpui::Rect rect = command.rounded_rect.rect;
      if (rect.origin.x != bounds.origin.x || rect.origin.y != bounds.origin.y ||
          rect.size.width != bounds.size.width) {
        continue;
      }
      found = true;
      if (command.rounded_rect.color.a != 1.0F) return 80;
    }
    if (!found) return 81;
  }
  return 0;
}

int main() {
  for (const auto test : {test_open_focus_and_escape_restore,
                          test_flips_and_shifts_at_edges, test_outside_click_dismisses,
                          test_keyboard_navigation_and_submenu,
                          test_pointer_hover_and_activation,
                          test_accelerator_column_uses_text_metrics, test_accessibility,
                          test_surface_is_opaque_without_blur}) {
    if (const int result = test(); result != 0) {
      std::cerr << "workbench_popover_menu_test failed: " << result << '\n';
      return result;
    }
  }
  return 0;
}
