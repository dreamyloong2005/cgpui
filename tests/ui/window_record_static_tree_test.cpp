#include "cgpui/ui/runtime.hpp"
#include "cgpui/ui/test_app.hpp"
#include "cgpui/ui/element.hpp"

#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

class SwitchingView final : public cgpui::View {
 public:
  enum class Mode { dynamic, static_tree, invalid_static, empty_static, empty };
  bool supports_static_render() const override { return mode != Mode::dynamic; }
  cgpui::StaticElementTreeView tree() const {
    return {nodes, children, cgpui::ElementId{1}};
  }
  cgpui::StaticElementTreeView render_static(cgpui::ViewContext& context) override {
    context.set_element_cursor({2}, cursor);
    if (mode == Mode::empty_static || mode == Mode::empty) return {};
    if (mode == Mode::invalid_static) return {nodes, children, cgpui::ElementId{99}};
    return tree();
  }
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    if (mode == Mode::empty) return {};
    return cgpui::into_element(cgpui::div().size(200, 100)
        .background(cgpui::rgb(20, 40, 60))
        .on_click([this](const cgpui::ElementEventContext&) {
          ++dynamic_clicks;
          return cgpui::EventResult::consumed_event();
        }));
  }
  void paint(cgpui::PaintList&, cgpui::Size) override { ++paints; }
  cgpui::EventResult handle_event(const cgpui::PlatformEvent& event,
                                 const cgpui::ViewContext& context) override {
    if (context.event_route) route = *context.event_route;
    accessibility = context.accessibility_snapshot();
    ++events;
    if (capture && std::holds_alternative<cgpui::PointerButton>(event)) {
      context.capture_pointer(cgpui::ElementId{2});
    }
    return cgpui::EventResult::unhandled();
  }
  Mode mode = Mode::static_tree;
  cgpui::CursorShape cursor = cgpui::CursorShape::pointing_hand;
  cgpui::EventRoute route;
  cgpui::AccessibilityTreeSnapshot accessibility;
  int dynamic_clicks = 0;
  int events = 0;
  int paints = 0;
  bool capture = false;
  std::array<cgpui::StaticElementNode, 2> nodes{
      cgpui::StaticElementNode{.id = {1}, .children = {0, 1},
                               .bounds = {{0, 0}, {200, 100}}},
      cgpui::StaticElementNode{.id = {2}, .parent_id = {1},
                               .kind = cgpui::StaticElementKind::text,
                               .bounds = {{10, 10}, {80, 40}},
                               .text = "static target", .focusable = true}};
  std::array<cgpui::ElementId, 1> children{cgpui::ElementId{2}};
};

void click(const cgpui::TestAppWindow& window) {
  window.dispatch_pointer_button(cgpui::MouseButton::left, true, {20, 20});
  window.dispatch_pointer_button(cgpui::MouseButton::left, false, {20, 20});
}

void routing_and_isolation() {
  cgpui::TestApp app;
  auto first = app.open_window(cgpui::WindowOptions{}.size(200, 100),
                               std::make_unique<SwitchingView>());
  auto second = app.open_window(cgpui::WindowOptions{}.size(200, 100),
                                std::make_unique<SwitchingView>());
  auto* view = first.root_view_as<SwitchingView>();
  auto* other = second.root_view_as<SwitchingView>();
  other->cursor = cgpui::CursorShape::text;
  require(view->tree().valid() && other->tree().valid(), "invalid static fixture");
  first.draw_frame();
  second.draw_frame();
  first.dispatch_pointer_move({20, 20});
  require(first.input_state().cursor_shape == cgpui::CursorShape::pointing_hand,
          "static cursor not selected");
  click(first);
  require(view->route.target_element_id == cgpui::ElementId{2},
          "static child window did not route the click to the hit node");
  require(view->route.element_ancestry == std::vector<cgpui::ElementId>{{2}, {1}},
          "wrong static ancestry");
  require(first.input_state().keyboard_focus_element_owner == cgpui::ElementId{2} &&
              first.input_state().clicked_element_id == cgpui::ElementId{2},
          "static focus/click synthesis missing");
  first.dispatch_keystroke({.key_code = 65, .action = cgpui::KeyAction::pressed});
  require(view->route.target_element_id == cgpui::ElementId{2} &&
              view->route.element_ancestry == std::vector<cgpui::ElementId>{{2}, {1}},
          "keyboard route lost static focus ancestry");
  const auto* accessible = view->accessibility.node({2});
  require(accessible && accessible->focused && accessible->text == "static target",
          "window context did not expose focused static accessibility");
  require(!second.input_state().keyboard_focus_element_owner && other->events == 0,
          "static events leaked into another window");
  second.dispatch_pointer_move({20, 20});
  require(second.input_state().cursor_shape == cgpui::CursorShape::text &&
              first.input_state().cursor_shape == cgpui::CursorShape::pointing_hand,
          "window-local cursor maps leaked");
  view->capture = true;
  first.dispatch_pointer_button(cgpui::MouseButton::left, true, {20, 20});
  first.dispatch_pointer_move({250, 150});
  require(view->route.target_element_id == cgpui::ElementId{2},
          "static pointer capture lost outside hit bounds");
  // A separate window has no capture or previous focus to mask disabled behavior.
  other->nodes[1].enabled = false;
  second.draw_frame();
  second.dispatch_pointer_move({20, 20});
  click(second);
  require(!second.input_state().keyboard_focus_element_owner &&
              second.input_state().cursor_shape == cgpui::CursorShape::default_arrow &&
              other->route.target_element_id == cgpui::ElementId{1},
          "disabled static child still focused or selected its cursor");
}

void switching_and_fallback() {
  cgpui::TestApp app;
  auto window = app.open_window(cgpui::WindowOptions{}.size(200, 100),
                                std::make_unique<SwitchingView>());
  auto* view = window.root_view_as<SwitchingView>();
  for (const auto fallback : {SwitchingView::Mode::dynamic,
                             SwitchingView::Mode::invalid_static,
                             SwitchingView::Mode::empty_static}) {
    view->mode = SwitchingView::Mode::dynamic;
    auto before = window.render_snapshot().draw_rect_count;
    window.draw_frame();
    require(window.render_snapshot().draw_rect_count == before + 1,
            "dynamic fixture must actually paint");
    const int clicks_before = view->dynamic_clicks;
    click(window);
    require(view->dynamic_clicks == clicks_before + 1, "dynamic fixture must handle click");
    view->mode = SwitchingView::Mode::static_tree;
    before = window.render_snapshot().draw_rect_count;
    window.draw_frame();
    require(window.render_snapshot().draw_rect_count == before,
            "old dynamic tree painted during static frame");
    click(window);
    require(view->dynamic_clicks == clicks_before + 1 &&
                view->route.target_element_id == cgpui::ElementId{2},
            "old dynamic tree intercepted static click");
    view->mode = fallback;
    window.release_focus({2});
    before = window.render_snapshot().draw_rect_count;
    window.draw_frame();
    click(window);
    require(window.render_snapshot().draw_rect_count == before + 1 &&
                view->dynamic_clicks == clicks_before + 2,
            "dynamic/invalid/empty static fallback did not replace static tree");
  }
  view->mode = SwitchingView::Mode::empty;
  const auto before = window.render_snapshot().draw_rect_count;
  window.draw_frame();
  click(window);
  require(window.render_snapshot().draw_rect_count == before &&
              !view->route.target_element_id && view->route.element_ancestry.empty(),
          "empty render retained an old tree");
}
} // namespace

int main() {
  try { routing_and_isolation(); switching_and_fallback(); }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
  return 0;
}
