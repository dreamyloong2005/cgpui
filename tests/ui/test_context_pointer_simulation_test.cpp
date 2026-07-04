#include "window_runtime_test_support.hpp"

#include <optional>
#include <vector>
#include <variant>

namespace {

RuntimeFixture* test_context_pointer_fixture = nullptr;

void dispatch_initial_pointer_simulation_event() {
  test_context_pointer_fixture->window.callback(cgpui::KeyboardKey{
      .key_code = 'I',
      .action = cgpui::KeyAction::pressed});
}

class TestContextPointerView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
        key != nullptr && key->key_code == 'I' && !queued_simulation) {
      queued_simulation = true;
      context.defer([](const cgpui::WindowRuntimeContext& deferred_context) {
        const cgpui::TestContextCapability test_context =
            deferred_context.test_context();
        test_context.dispatch_pointer_move(cgpui::Point{5.0F, 5.0F});
        test_context.dispatch_pointer_button(
            cgpui::MouseButton::left,
            true,
            cgpui::Point{5.0F, 15.0F});
        test_context.dispatch_pointer_scroll(
            cgpui::Point{0.0F, -3.0F},
            cgpui::Point{30.0F, 15.0F});
      });
      return cgpui::EventResult::consumed_event();
    }

    if (std::holds_alternative<cgpui::PointerMoved>(event)) {
      pointer_move_count += 1;
      move_position_from_context = context.test_context()
                                       .input_state()
                                       .pointer_position;
    } else if (const auto* button = std::get_if<cgpui::PointerButton>(&event);
               button != nullptr) {
      pointer_button_count += 1;
      button_position_from_event = button->position;
      button_pressed = button->pressed;
      button_kind = button->button;
    } else if (const auto* scrolled =
                   std::get_if<cgpui::PointerScrolled>(&event);
               scrolled != nullptr) {
      pointer_scroll_count += 1;
      scroll_delta_from_event = scrolled->delta;
      scroll_position_from_context = context.test_context()
                                         .input_state()
                                         .pointer_position;
    }

    return cgpui::EventResult::unhandled();
  }

  bool queued_simulation = false;
  int pointer_move_count = 0;
  int pointer_button_count = 0;
  int pointer_scroll_count = 0;
  bool button_pressed = false;
  cgpui::MouseButton button_kind = cgpui::MouseButton::other;
  cgpui::Point move_position_from_context{};
  cgpui::Point button_position_from_event{};
  cgpui::Point scroll_delta_from_event{};
  cgpui::Point scroll_position_from_context{};
};

int test_test_context_simulates_pointer_input_through_runtime_dispatch() {
  RuntimeFixture fixture;
  TestContextPointerView view;
  test_context_pointer_fixture = &fixture;
  fixture.app.on_run = &dispatch_initial_pointer_simulation_event;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{10});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{11});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 20.0F, .height = 30.0F});
  second->assign_id(cgpui::ElementId{12});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));
  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 40.0F) {
    return 1;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  std::vector<cgpui::EventDispatchRecord> records;
  std::vector<cgpui::Point> callback_positions;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        records.push_back(record);
        callback_positions.push_back(
            context.test_context().input_state().pointer_position);
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  test_context_pointer_fixture = nullptr;

  if (result != 0) {
    return 2;
  }
  if (records.size() != 4 || callback_positions.size() != records.size()) {
    return 3;
  }
  if (records[0].event_kind != cgpui::EventKind::keyboard_key ||
      records[1].event_kind != cgpui::EventKind::pointer_moved ||
      records[2].event_kind != cgpui::EventKind::pointer_button ||
      records[3].event_kind != cgpui::EventKind::pointer_scrolled) {
    return 4;
  }
  if (view.pointer_move_count != 1 || view.pointer_button_count != 1 ||
      view.pointer_scroll_count != 1) {
    return 5;
  }
  if (!equal(view.move_position_from_context, cgpui::Point{5.0F, 5.0F}) ||
      !equal(callback_positions[1], cgpui::Point{5.0F, 5.0F})) {
    return 6;
  }
  if (!view.button_pressed || view.button_kind != cgpui::MouseButton::left ||
      !equal(view.button_position_from_event, cgpui::Point{5.0F, 15.0F}) ||
      !equal(callback_positions[2], cgpui::Point{5.0F, 15.0F})) {
    return 7;
  }
  if (!equal(view.scroll_delta_from_event, cgpui::Point{0.0F, -3.0F}) ||
      !equal(view.scroll_position_from_context, cgpui::Point{30.0F, 15.0F}) ||
      !equal(callback_positions[3], cgpui::Point{30.0F, 15.0F})) {
    return 8;
  }
  if (!records[1].route.target_element_id.has_value() ||
      *records[1].route.target_element_id != cgpui::ElementId{11}) {
    return 9;
  }
  if (!records[2].route.target_element_id.has_value() ||
      *records[2].route.target_element_id != cgpui::ElementId{12}) {
    return 10;
  }
  if (!records[3].route.target_element_id.has_value() ||
      *records[3].route.target_element_id != cgpui::ElementId{10}) {
    return 11;
  }

  return 0;
}

} // namespace

int main() {
  return test_test_context_simulates_pointer_input_through_runtime_dispatch();
}
