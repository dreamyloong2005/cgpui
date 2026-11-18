#include "cgpui/ui/tab_strip.hpp"
#include "cgpui/ui/text_measurement.hpp"

#include "workbench_test_support.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

using cgpui::TabItem;
using cgpui::TabStripEvent;
using cgpui::TabStripEventKind;

std::vector<TabItem> make_tabs(std::size_t count) {
  std::vector<TabItem> tabs;
  for (std::size_t index = 0; index < count; ++index) {
    tabs.push_back(TabItem{.id = 100 + index,
                           .title = "file_" + std::to_string(index) + ".cpp",
                           .dirty = index == 1});
  }
  return tabs;
}

struct Fixture {
  cgpui::TestApp app;
  cgpui::TabStrip strip;
  cgpui::WorkbenchTheme theme = cgpui::workbench_theme(cgpui::SystemAppearance{});
  std::vector<TabStripEvent> events;
  std::vector<TabItem> model;
  float width;
  cgpui::TestAppWindow window;

  explicit Fixture(std::size_t count = 5, float strip_width = 900.0F)
      : model(make_tabs(count)), width(strip_width),
        window(app.open_window(cgpui::WindowOptions{}.size(1000.0F, 400.0F),
                               std::make_unique<workbench_test::HostView>())) {
    auto* view = window.root_view_as<workbench_test::HostView>();
    view->build = [this](cgpui::WidgetTreeBuilder& builder,
                         const cgpui::ViewContext& context) {
      strip.layout(cgpui::Rect{{0, 30}, {width, 34}}, context.scale, theme);
      strip.append_elements(builder, builder.root());
    };
    view->on_event = [this](const cgpui::PlatformEvent& event,
                            const cgpui::ViewContext& context) {
      return strip.handle_event(event, context);
    };
    strip.on_event([this](const TabStripEvent& event) { apply(event); });
    strip.set_tabs(model);
    strip.set_selected(model.front().id);
    window.draw_frame();
  }

  // The app owns the tab list: it applies every event and pushes it back.
  void apply(const TabStripEvent& event) {
    events.push_back(event);
    if (event.kind == TabStripEventKind::select) {
      strip.set_selected(event.tab);
    } else if (event.kind == TabStripEventKind::close) {
      model.erase(model.begin() + static_cast<long>(event.from_index));
      strip.set_tabs(model);
    } else {
      TabItem moved = model[event.from_index];
      model.erase(model.begin() + static_cast<long>(event.from_index));
      model.insert(model.begin() + static_cast<long>(event.to_index), moved);
      strip.set_tabs(model);
    }
  }
};

int test_real_text_metrics_and_dpi() {
  Fixture f;
  for (std::size_t index = 0; index < f.model.size(); ++index) {
    const float expected =
        cgpui::measure_text(f.model[index].title, f.theme.font,
                            f.theme.font_size, cgpui::DpiScale{})
            .logical_size.width;
    if (expected <= 0.0F || f.strip.title_width(index) != expected) return 10;
    if (f.strip.tab_rect(index).size.width < expected) return 11;
  }
  f.window.simulate_resize(cgpui::Size{1500.0F, 600.0F}, cgpui::DpiScale{1.5F});
  f.window.draw_frame();
  for (std::size_t index = 0; index < f.model.size(); ++index) {
    const cgpui::Rect rect = f.strip.tab_rect(index);
    const float left = rect.origin.x * 1.5F;
    const float right = (rect.origin.x + rect.size.width) * 1.5F;
    if (std::fabs(left - std::round(left)) > 0.001F ||
        std::fabs(right - std::round(right)) > 0.001F) {
      return 12;
    }
  }
  return 0;
}

int test_select_close_and_middle_click() {
  using workbench_test::center;
  using workbench_test::click;
  Fixture f;
  click(f.window, center(f.strip.tab_rect(2)));
  if (f.strip.selected() != 102 || f.events.back().kind != TabStripEventKind::select) {
    return 20;
  }
  click(f.window, center(f.strip.close_rect(2)));
  if (f.events.back().kind != TabStripEventKind::close || f.events.back().tab != 102 ||
      f.strip.tabs().size() != 4) {
    return 21;
  }
  // Middle press on one tab and release on another closes nothing.
  (void)f.window.dispatch_platform_event(cgpui::PointerButton{
      .button = cgpui::MouseButton::middle, .pressed = true,
      .position = center(f.strip.tab_rect(0))});
  (void)f.window.dispatch_platform_event(cgpui::PointerButton{
      .button = cgpui::MouseButton::middle, .pressed = false,
      .position = center(f.strip.tab_rect(1))});
  if (f.strip.tabs().size() != 4) return 22;
  click(f.window, center(f.strip.tab_rect(1)), cgpui::MouseButton::middle);
  if (f.strip.tabs().size() != 3 || f.events.back().tab != 101) return 23;
  return 0;
}

int test_drag_reorder() {
  using workbench_test::center;
  Fixture f;
  const cgpui::Point start = center(f.strip.tab_rect(0));
  (void)f.window.dispatch_platform_event(cgpui::PointerButton{
      .button = cgpui::MouseButton::left, .pressed = true, .position = start});
  if (!f.window.input_state().pointer_captured) return 30;
  f.window.dispatch_pointer_move({start.x + 3.0F, start.y});
  if (f.strip.dragging()) return 31;  // below the 5 px threshold
  const cgpui::Rect third = f.strip.tab_rect(2);
  const cgpui::Point drop{third.origin.x + third.size.width * 0.75F, start.y};
  f.window.dispatch_pointer_move(drop);
  if (!f.strip.dragging() || f.strip.drop_index() != 2) return 32;
  f.window.dispatch_pointer_button(cgpui::MouseButton::left, false, drop);
  const TabStripEvent& event = f.events.back();
  if (event.kind != TabStripEventKind::reorder || event.tab != 100 ||
      event.from_index != 0 || event.to_index != 2 || f.model[2].id != 100) {
    return 33;
  }
  if (f.window.input_state().pointer_captured || f.strip.dragging()) return 34;
  return 0;
}

int test_overflow_scrolls() {
  Fixture f(20, 300.0F);
  if (f.strip.content_width() <= 300.0F || f.strip.scroll_offset() != 0.0F) return 40;
  f.window.dispatch_pointer_scroll({0.0F, -2.0F}, {150.0F, 45.0F});
  if (f.strip.scroll_offset() != 80.0F) return 41;
  f.strip.set_selected(f.model.back().id);
  const cgpui::Rect last = f.strip.tab_rect(19);
  if (last.origin.x + last.size.width > 300.0F + 0.5F || last.origin.x < 0.0F) {
    return 42;
  }
  // The strip clamps: scrolling far right stops at the last tab.
  f.window.dispatch_pointer_scroll({0.0F, -1000.0F}, {150.0F, 45.0F});
  if (std::fabs(f.strip.scroll_offset() - (f.strip.content_width() - 300.0F)) > 0.01F) {
    return 43;
  }
  return 0;
}

int test_keyboard() {
  using workbench_test::key;
  Fixture f;
  f.window.focus(f.strip.element_id());
  key(f.window, 39);
  if (f.strip.selected() != 101) return 50;
  key(f.window, 35);
  if (f.strip.selected() != 104) return 51;
  key(f.window, 36);
  if (f.strip.selected() != 100) return 52;
  // Ctrl+Tab belongs to the app's actions, not the strip.
  key(f.window, 9, {.control = true});
  if (f.strip.selected() != 100) return 53;
  return 0;
}

int test_accessibility() {
  Fixture f;
  f.strip.set_selected(103);
  const cgpui::AccessibilityTreeSnapshot snapshot = workbench_test::snapshot_of(
      {1000.0F, 400.0F}, f.strip.element_id(), [&](cgpui::WidgetTreeBuilder& builder) {
        f.strip.append_elements(builder, builder.root());
      });
  const auto* list = workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tab_list);
  if (list == nullptr || !list->focusable || !list->focused || list->children.size() != 5) {
    return 60;
  }
  if (workbench_test::count_role(snapshot, cgpui::AccessibilityRole::tab) != 5) return 61;
  const auto* dirty = workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tab, 1);
  const auto* selected = workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tab, 3);
  if (dirty == nullptr || dirty->value != "modified" || dirty->patterns.selected != false ||
      selected == nullptr || selected->patterns.selected != true ||
      selected->name != "file_3.cpp") {
    return 62;
  }
  return 0;
}

} // namespace

int main() {
  for (const auto test : {test_real_text_metrics_and_dpi,
                          test_select_close_and_middle_click, test_drag_reorder,
                          test_overflow_scrolls, test_keyboard, test_accessibility}) {
    if (const int result = test(); result != 0) {
      std::cerr << "workbench_tab_strip_test failed: " << result << '\n';
      return result;
    }
  }
  return 0;
}
