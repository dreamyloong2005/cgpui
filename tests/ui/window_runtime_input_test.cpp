#include "window_runtime_test_support.hpp"

namespace {
RuntimeFixture* event_dispatch_fixture = nullptr;

void dispatch_view_events() {
  auto& callback = event_dispatch_fixture->window.callback;
  callback(cgpui::WindowFocused{.focused = true});
  callback(cgpui::PointerMoved{.position = {12.0F, 24.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .position = {12.0F, 24.0F}});
  callback(cgpui::PointerScrolled{
      .delta = {0.0F, -4.0F},
      .position = {12.0F, 24.0F}});
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true}});
  callback(cgpui::TextInput{
      .text = "A",
      .modifiers = {.shift = true}});
}

int test_runtime_dispatches_input_events_to_view() {
  RuntimeFixture fixture;
  event_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_events;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_dispatch_fixture = nullptr;

  if (result != 0) {
    return 40;
  }
  if (fixture.view.event_count != 6) {
    return 41;
  }
  if (fixture.view.focus_count != 1 ||
      fixture.view.pointer_move_count != 1 ||
      fixture.view.pointer_button_count != 1 ||
      fixture.view.pointer_scroll_count != 1 ||
      fixture.view.keyboard_key_count != 1 ||
      fixture.view.text_input_count != 1) {
    return 42;
  }
  if (!equal(
          fixture.view.last_event_viewport_size,
          cgpui::Size{640.0F, 480.0F})) {
    return 43;
  }
  if (fixture.view.last_event_frame_index != 1) {
    return 44;
  }
  if (!fixture.view.focus_event_saw_focused ||
      !fixture.view.last_input_focused) {
    return 45;
  }
  if (!equal(
          fixture.view.pointer_move_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 46;
  }
  if (!equal(
          fixture.view.pointer_button_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 47;
  }
  if (!equal(
          fixture.view.pointer_scroll_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 48;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 49;
  }

  return 0;
}

RuntimeFixture* event_redraw_fixture = nullptr;

void dispatch_event_driven_redraw() {
  event_redraw_fixture->window.callback(
      cgpui::PointerMoved{.position = {5.0F, 6.0F}});
}

int test_view_event_can_request_redraw() {
  RuntimeFixture fixture;
  event_redraw_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_driven_redraw;
  fixture.view.request_redraw_on_event = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_redraw_fixture = nullptr;

  if (result != 0) {
    return 50;
  }
  if (fixture.view.event_count != 1 ||
      fixture.view.event_redraw_requests != 1) {
    return 51;
  }
  if (fixture.window.request_redraw_count != 2) {
    return 52;
  }
  if (fixture.renderer.begin_frame_count != 2 ||
      fixture.view.paint_count != 2 ||
      fixture.frame.present_count != 2) {
    return 53;
  }

  return 0;
}

RuntimeFixture* event_result_fixture = nullptr;

void dispatch_event_result_sequence() {
  auto& callback = event_result_fixture->window.callback;
  event_result_fixture->view.consume_next_event = true;
  callback(cgpui::PointerMoved{.position = {7.0F, 8.0F}});
  callback(cgpui::PointerMoved{.position = {9.0F, 10.0F}});
  event_result_fixture->view.cancel_next_event = true;
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "a"});
}

int test_runtime_exposes_last_view_event_result() {
  RuntimeFixture fixture;
  event_result_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_result_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_result_fixture = nullptr;

  if (result != 0) {
    return 55;
  }
  if (fixture.view.pointer_move_count != 2 ||
      fixture.view.keyboard_key_count != 1 ||
      fixture.view.text_input_count != 1) {
    return 56;
  }
  if (!fixture.view.last_event_result_consumed ||
      !fixture.view.last_event_result_cancelled) {
    return 57;
  }

  return 0;
}

RuntimeFixture* event_observability_fixture = nullptr;

void dispatch_event_observability_sequence() {
  auto& callback = event_observability_fixture->window.callback;
  event_observability_fixture->view.consume_next_event = true;
  callback(cgpui::PointerMoved{.position = {11.0F, 12.0F}});
  event_observability_fixture->view.cancel_next_event = true;
  callback(cgpui::KeyboardKey{
      .key_code = 66,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "b"});
}

int test_runtime_reports_each_view_event_dispatch() {
  RuntimeFixture fixture;
  event_observability_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_observability_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  int callback_count = 0;
  cgpui::EventDispatchRecord first_record{};
  cgpui::EventDispatchRecord second_record{};
  cgpui::EventDispatchRecord third_record{};
  bool callback_context_saw_current_record = true;
  bool callback_view_id_matched_context = true;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (callback_count == 1) {
          first_record = record;
        } else if (callback_count == 2) {
          second_record = record;
        } else if (callback_count == 3) {
          third_record = record;
        }
        callback_context_saw_current_record =
            callback_context_saw_current_record &&
            context.last_event_dispatch.has_value() &&
            context.last_event_dispatch->sequence == record.sequence &&
            context.last_event_dispatch->event_kind == record.event_kind &&
            context.last_event_dispatch->view_id == record.view_id &&
            context.last_event_dispatch->result.consumed ==
                record.result.consumed &&
            context.last_event_dispatch->result.cancelled ==
                record.result.cancelled;
        callback_view_id_matched_context =
            callback_view_id_matched_context &&
            record.view_id == context.view_id;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_observability_fixture = nullptr;

  if (result != 0) {
    return 58;
  }
  if (callback_count != 3) {
    return 59;
  }
  if (first_record.sequence != 1 ||
      first_record.event_kind != cgpui::EventKind::pointer_moved ||
      !first_record.result.consumed ||
      first_record.result.cancelled) {
    return 100;
  }
  if (second_record.sequence != 2 ||
      second_record.event_kind != cgpui::EventKind::keyboard_key ||
      !second_record.result.consumed ||
      !second_record.result.cancelled) {
    return 101;
  }
  if (third_record.sequence != 3 ||
      third_record.event_kind != cgpui::EventKind::text_input ||
      third_record.result.consumed ||
      third_record.result.cancelled) {
    return 102;
  }
  if (first_record.view_id.value == 0 ||
      first_record.view_id != second_record.view_id ||
      second_record.view_id != third_record.view_id) {
    return 103;
  }
  if (!callback_context_saw_current_record ||
      !callback_view_id_matched_context) {
    return 104;
  }
  if (!fixture.view.saw_last_event_dispatch ||
      fixture.view.last_event_dispatch.sequence != 2 ||
      fixture.view.last_event_dispatch.event_kind !=
          cgpui::EventKind::keyboard_key ||
      !fixture.view.last_event_dispatch.result.cancelled) {
    return 105;
  }

  return 0;
}

int test_event_router_routes_events_to_root_view() {
  const cgpui::ViewId root_view_id{42};

  const auto pointer_route = cgpui::EventRouter::route_to_root(
      cgpui::PointerMoved{.position = {1.0F, 2.0F}},
      root_view_id);
  const auto key_route = cgpui::EventRouter::route_to_root(
      cgpui::KeyboardKey{
          .key_code = 13,
          .action = cgpui::KeyAction::pressed},
      root_view_id);
  const cgpui::DragDropPayload text_payload{
      .kind = cgpui::DragDropPayloadKind::text,
      .text = "Dragged text",
  };
  const cgpui::DragDropPayload file_payload{
      .kind = cgpui::DragDropPayloadKind::files,
      .files = {"C:\\Temp\\first.txt", "C:\\Temp\\second.cpp"},
  };
  const auto drag_enter_route = cgpui::EventRouter::route_to_root(
      cgpui::DragEntered{
          .position = {3.0F, 4.0F},
          .payload = text_payload},
      root_view_id);
  const auto drag_update_route = cgpui::EventRouter::route_to_root(
      cgpui::DragUpdated{
          .position = {5.0F, 6.0F},
          .payload = text_payload},
      root_view_id);
  const auto drag_drop_route = cgpui::EventRouter::route_to_root(
      cgpui::DragDropped{
          .position = {7.0F, 8.0F},
          .payload = file_payload},
      root_view_id);
  const auto drag_exit_route = cgpui::EventRouter::route_to_root(
      cgpui::DragExited{
          .position = {9.0F, 10.0F},
          .payload = {}},
      root_view_id);

  if (pointer_route.target_view_id != root_view_id ||
      pointer_route.event_kind != cgpui::EventKind::pointer_moved) {
    return 106;
  }
  if (key_route.target_view_id != root_view_id ||
      key_route.event_kind != cgpui::EventKind::keyboard_key) {
    return 107;
  }
  if (drag_enter_route.target_view_id != root_view_id ||
      drag_enter_route.event_kind != cgpui::EventKind::drag_entered) {
    return 130;
  }
  if (drag_update_route.target_view_id != root_view_id ||
      drag_update_route.event_kind != cgpui::EventKind::drag_updated) {
    return 131;
  }
  if (drag_drop_route.target_view_id != root_view_id ||
      drag_drop_route.event_kind != cgpui::EventKind::drag_dropped) {
    return 132;
  }
  if (drag_exit_route.target_view_id != root_view_id ||
      drag_exit_route.event_kind != cgpui::EventKind::drag_exited) {
    return 133;
  }

  return 0;
}

RuntimeFixture* event_route_fixture = nullptr;

void dispatch_event_route_sequence() {
  event_route_fixture->window.callback(cgpui::PointerMoved{
      .position = {21.0F, 22.0F}});
}

int test_runtime_exposes_current_event_route() {
  RuntimeFixture fixture;
  event_route_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_route_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::EventDispatchRecord callback_record{};
  cgpui::EventRoute callback_route{};
  bool callback_saw_route = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_record = record;
        callback_saw_route = context.event_route.has_value();
        if (context.event_route) {
          callback_route = *context.event_route;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_route_fixture = nullptr;

  if (result != 0) {
    return 108;
  }
  if (!fixture.view.saw_event_route ||
      fixture.view.last_event_route.target_view_id != fixture.view.last_view_id ||
      fixture.view.last_event_route.event_kind !=
          cgpui::EventKind::pointer_moved) {
    return 109;
  }
  if (!callback_saw_route ||
      callback_route.target_view_id != fixture.view.last_view_id ||
      callback_route.event_kind != cgpui::EventKind::pointer_moved) {
    return 110;
  }
  if (callback_record.route.target_view_id != fixture.view.last_view_id ||
      callback_record.route.event_kind != cgpui::EventKind::pointer_moved ||
      callback_record.view_id != callback_record.route.target_view_id ||
      callback_record.event_kind != callback_record.route.event_kind) {
    return 111;
  }

  return 0;
}

int test_view_context_current_event_route_helper_matches_route_field() {
  RuntimeFixture fixture;
  event_route_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_route_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::EventRoute callback_helper_route{};
  bool callback_saw_helper_route = false;
  bool callback_helper_route_matched_record = false;
  runtime.set_after_event_callback(
      [&](const cgpui::ViewContext& context,
          const cgpui::EventDispatchRecord& record) {
        const std::optional<cgpui::EventRoute> route =
            context.current_event_route();
        callback_saw_helper_route = route.has_value();
        if (route.has_value()) {
          callback_helper_route = *route;
          callback_helper_route_matched_record =
              route->target_view_id == record.route.target_view_id &&
              route->target_element_id == record.route.target_element_id &&
              route->event_kind == record.route.event_kind;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_route_fixture = nullptr;

  if (result != 0) {
    return 112;
  }
  if (!fixture.view.saw_current_event_route_helper ||
      !fixture.view.current_event_route_helper_matches_field ||
      fixture.view.current_event_route_helper.event_kind !=
          cgpui::EventKind::pointer_moved ||
      fixture.view.current_event_route_helper.target_view_id !=
          fixture.view.last_event_route.target_view_id) {
    return 113;
  }
  if (!callback_saw_helper_route || !callback_helper_route_matched_record ||
      callback_helper_route.event_kind != cgpui::EventKind::pointer_moved ||
      callback_helper_route.target_view_id != fixture.view.last_view_id) {
    return 114;
  }

  return 0;
}

int test_view_context_input_state_helper_matches_input_snapshot() {
  RuntimeFixture fixture;
  event_route_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_route_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::ViewInputState callback_input{};
  bool callback_saw_input_state = false;
  bool callback_input_matched_context = false;
  runtime.set_after_event_callback(
      [&](const cgpui::ViewContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_input = context.input_state();
        callback_saw_input_state = true;
        callback_input_matched_context =
            callback_input.focused == context.input.focused &&
            callback_input.pointer_captured == context.input.pointer_captured &&
            callback_input.pointer_capture_owner ==
                context.input.pointer_capture_owner &&
            callback_input.keyboard_focused ==
                context.input.keyboard_focused &&
            callback_input.keyboard_focus_owner ==
                context.input.keyboard_focus_owner &&
            callback_input.keyboard_focus_element_owner ==
                context.input.keyboard_focus_element_owner &&
            callback_input.hovered_element_id ==
                context.input.hovered_element_id &&
            callback_input.cursor_shape == context.input.cursor_shape &&
            equal(callback_input.pointer_position,
                  context.input.pointer_position) &&
            record.event_kind == cgpui::EventKind::pointer_moved;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_route_fixture = nullptr;

  if (result != 0) {
    return 115;
  }
  if (!fixture.view.saw_input_state_helper ||
      !fixture.view.input_state_helper_matches_field ||
      !equal(fixture.view.input_state_helper.pointer_position,
             cgpui::Point{21.0F, 22.0F})) {
    return 116;
  }
  if (!callback_saw_input_state || !callback_input_matched_context ||
      !equal(callback_input.pointer_position, cgpui::Point{21.0F, 22.0F})) {
    return 117;
  }

  return 0;
}

RuntimeFixture* pointer_hit_route_fixture = nullptr;

void dispatch_pointer_hit_route_sequence() {
  auto& callback = pointer_hit_route_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .position = {5.0F, 15.0F}});
  callback(cgpui::PointerScrolled{
      .delta = {0.0F, -1.0F},
      .position = {30.0F, 15.0F}});
  callback(cgpui::KeyboardKey{
      .key_code = 70,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_routes_pointer_events_to_hit_element() {
  RuntimeFixture fixture;
  pointer_hit_route_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_hit_route_sequence;

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
    return 130;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  int callback_count = 0;
  cgpui::EventDispatchRecord move_record{};
  cgpui::EventDispatchRecord button_record{};
  cgpui::EventDispatchRecord scroll_record{};
  cgpui::EventDispatchRecord key_record{};
  bool callback_context_matched_record_route = true;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (callback_count == 1) {
          move_record = record;
        } else if (callback_count == 2) {
          button_record = record;
        } else if (callback_count == 3) {
          scroll_record = record;
        } else if (callback_count == 4) {
          key_record = record;
        }
        callback_context_matched_record_route =
            callback_context_matched_record_route &&
            context.event_route.has_value() &&
            context.event_route->target_element_id ==
                record.route.target_element_id;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_hit_route_fixture = nullptr;

  if (result != 0) {
    return 123;
  }
  if (callback_count != 4 || fixture.view.event_count != 4) {
    return 124;
  }
  if (!move_record.route.target_element_id.has_value() ||
      *move_record.route.target_element_id != cgpui::ElementId{11}) {
    return 125;
  }
  if (!button_record.route.target_element_id.has_value() ||
      *button_record.route.target_element_id != cgpui::ElementId{12}) {
    return 126;
  }
  if (!scroll_record.route.target_element_id.has_value() ||
      *scroll_record.route.target_element_id != cgpui::ElementId{10}) {
    return 127;
  }
  if (key_record.route.target_element_id.has_value() ||
      fixture.view.last_route_element_id.has_value()) {
    return 128;
  }
  if (!callback_context_matched_record_route) {
    return 129;
  }

  return 0;
}

RuntimeFixture* runtime_drag_drop_fixture = nullptr;

void dispatch_runtime_drag_drop_sequence() {
  auto& callback = runtime_drag_drop_fixture->window.callback;
  callback(cgpui::DragEntered{
      .position = {5.0F, 5.0F},
      .payload =
          cgpui::DragDropPayload{
              .kind = cgpui::DragDropPayloadKind::text,
              .text = "Dragged text",
          },
      .action = cgpui::DragDropAction::copy,
  });
  callback(cgpui::DragUpdated{
      .position = {6.0F, 5.0F},
      .payload =
          cgpui::DragDropPayload{
              .kind = cgpui::DragDropPayloadKind::text,
              .text = "Dragged text",
          },
      .action = cgpui::DragDropAction::move,
  });
  callback(cgpui::DragDropped{
      .position = {7.0F, 5.0F},
      .payload =
          cgpui::DragDropPayload{
              .kind = cgpui::DragDropPayloadKind::files,
              .files = {"C:\\Temp\\first.txt", "C:\\Temp\\second.cpp"},
          },
      .action = cgpui::DragDropAction::move,
  });
  callback(cgpui::DragExited{
      .position = {8.0F, 5.0F},
      .payload = {},
      .action = cgpui::DragDropAction::none,
  });
}

int test_runtime_routes_drag_drop_events_to_hit_element() {
  RuntimeFixture fixture;
  runtime_drag_drop_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_drag_drop_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  RuntimeEventElement* root_ptr = root.get();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::vector<cgpui::EventDispatchRecord> records;
  std::vector<cgpui::Point> pointer_positions;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        records.push_back(record);
        pointer_positions.push_back(context.input.pointer_position);
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  runtime_drag_drop_fixture = nullptr;

  if (result != 0) {
    return 214;
  }
  if (records.size() != 4 || root_ptr->event_count != 4 ||
      !root_ptr->saw_drag_event) {
    return 215;
  }
  const std::vector<cgpui::EventKind> expected_kinds{
      cgpui::EventKind::drag_entered,
      cgpui::EventKind::drag_updated,
      cgpui::EventKind::drag_dropped,
      cgpui::EventKind::drag_exited,
  };
  for (std::size_t index = 0; index < expected_kinds.size(); ++index) {
    if (records[index].event_kind != expected_kinds[index] ||
        records[index].route.event_kind != expected_kinds[index] ||
        !records[index].route.target_element_id.has_value() ||
        *records[index].route.target_element_id != root_id) {
      return 216;
    }
  }
  if (pointer_positions.size() != 4 ||
      !equal(pointer_positions.back(), cgpui::Point{8.0F, 5.0F})) {
    return 217;
  }
  if (root_ptr->last_drag_payload.kind != cgpui::DragDropPayloadKind::files ||
      root_ptr->last_drag_payload.files.size() != 2 ||
      root_ptr->last_drag_payload.files[0] != "C:\\Temp\\first.txt" ||
      root_ptr->last_drag_payload.files[1] != "C:\\Temp\\second.cpp") {
    return 218;
  }
  if (root_ptr->last_drag_action != cgpui::DragDropAction::move) {
    return 219;
  }

  return 0;
}

RuntimeFixture* element_event_dispatch_fixture = nullptr;

void dispatch_element_event_dispatch_sequence() {
  auto& callback = element_event_dispatch_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

RuntimeFixture* hidden_overflow_hit_test_fixture = nullptr;

void dispatch_hidden_overflow_hit_test_sequence() {
  auto& callback = hidden_overflow_hit_test_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {20.0F, 5.0F}});
}

int test_runtime_dispatches_consumed_element_event_before_view_fallback() {
  RuntimeFixture fixture;
  element_event_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_element_event_dispatch_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  RuntimeEventElement* root_ptr = root.get();
  root_ptr->result = cgpui::EventResult::consumed_event();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::EventDispatchRecord dispatch_record{};
  int callback_count = 0;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  element_event_dispatch_fixture = nullptr;

  if (result != 0) {
    return 209;
  }
  if (root_ptr->event_count != 1 || !root_ptr->saw_pointer_event ||
      root_ptr->last_target_element_id != root_id) {
    return 210;
  }
  if (fixture.view.event_count != 0) {
    return 211;
  }
  if (callback_count != 1 || !dispatch_record.result.consumed ||
      dispatch_record.result.cancelled) {
    return 212;
  }
  if (!dispatch_record.route.target_element_id.has_value() ||
      *dispatch_record.route.target_element_id != root_id) {
    return 213;
  }

  return 0;
}

int test_runtime_falls_back_to_view_after_unhandled_element_event() {
  RuntimeFixture fixture;
  element_event_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_element_event_dispatch_sequence;
  fixture.view.consume_next_event = true;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  RuntimeEventElement* root_ptr = root.get();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::EventDispatchRecord dispatch_record{};
  int callback_count = 0;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  element_event_dispatch_fixture = nullptr;

  if (result != 0) {
    return 214;
  }
  if (root_ptr->event_count != 1 || fixture.view.event_count != 1) {
    return 215;
  }
  if (fixture.view.last_route_element_id != root_id ||
      !fixture.view.saw_event_route) {
    return 216;
  }
  if (callback_count != 1 || !dispatch_record.result.consumed ||
      dispatch_record.result.cancelled) {
    return 217;
  }
  if (!dispatch_record.route.target_element_id.has_value() ||
      *dispatch_record.route.target_element_id != root_id) {
    return 218;
  }

  return 0;
}

int test_runtime_skips_disabled_element_event_and_falls_back_to_view() {
  RuntimeFixture fixture;
  element_event_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_element_event_dispatch_sequence;
  fixture.view.consume_next_event = true;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  RuntimeEventElement* root_ptr = root.get();
  root_ptr->set_enabled(false);
  root_ptr->result = cgpui::EventResult::consumed_event();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::EventDispatchRecord dispatch_record{};
  int callback_count = 0;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  element_event_dispatch_fixture = nullptr;

  if (result != 0) {
    return 219;
  }
  if (root_ptr->event_count != 0 || fixture.view.event_count != 1) {
    return 220;
  }
  if (fixture.view.last_route_element_id != root_id ||
      !fixture.view.saw_event_route) {
    return 221;
  }
  if (callback_count != 1 || !dispatch_record.result.consumed ||
      dispatch_record.result.cancelled) {
    return 222;
  }
  if (!dispatch_record.route.target_element_id.has_value() ||
      *dispatch_record.route.target_element_id != root_id) {
    return 223;
  }

  return 0;
}

int test_runtime_respects_hidden_overflow_clip_during_hit_testing() {
  RuntimeFixture fixture;
  hidden_overflow_hit_test_fixture = &fixture;
  fixture.app.on_run = &dispatch_hidden_overflow_hit_test_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id = tree->set_root(
      std::make_unique<cgpui::StyledElement>(
          cgpui::Style{}
              .with_overflow(cgpui::Overflow::hidden)
              .with_clip_rect(cgpui::Rect{
                  .origin = {.x = 0.0F, .y = 0.0F},
                  .size = {.width = 10.0F, .height = 10.0F},
              })
              .with_preferred_size(
                  cgpui::Size{.width = 50.0F, .height = 30.0F})));
  (void)tree->layout_root(cgpui::LayoutInput{});

  std::vector<std::optional<cgpui::ElementId>> routed_ids;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_moved) {
          routed_ids.push_back(record.route.target_element_id);
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  hidden_overflow_hit_test_fixture = nullptr;

  if (result != 0) {
    return 396;
  }
  if (routed_ids.size() != 2 || !routed_ids[0].has_value() ||
      *routed_ids[0] != root_id) {
    return 397;
  }
  return !routed_ids[1].has_value() ? 0 : 398;
}

RuntimeFixture* event_propagation_fixture = nullptr;
cgpui::WindowRuntime* event_propagation_runtime = nullptr;
cgpui::ElementId event_propagation_target_id;

void dispatch_event_propagation_keyboard_sequence() {
  auto& callback = event_propagation_fixture->window.callback;
  event_propagation_runtime->request_keyboard_focus(event_propagation_target_id);
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
}

struct EventPropagationTree {
  std::unique_ptr<cgpui::ElementTree> tree;
  cgpui::ElementId root_id;
  cgpui::ElementId child_id;
  cgpui::ElementId target_id;
  RuntimeEventElement* root = nullptr;
  RuntimeEventElement* child = nullptr;
  RuntimeEventElement* target = nullptr;
};

EventPropagationTree make_event_propagation_tree() {
  EventPropagationTree propagation_tree{
      .tree = std::make_unique<cgpui::ElementTree>()};
  auto root = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 80.0F, .height = 80.0F});
  propagation_tree.root = root.get();
  propagation_tree.root_id = propagation_tree.tree->set_root(std::move(root));

  auto child = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 60.0F, .height = 60.0F});
  propagation_tree.child = child.get();
  propagation_tree.child_id =
      propagation_tree.tree->append_child(propagation_tree.root_id,
                                          std::move(child));

  auto target = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 40.0F, .height = 40.0F});
  propagation_tree.target = target.get();
  propagation_tree.target_id =
      propagation_tree.tree->append_child(propagation_tree.child_id,
                                          std::move(target));
  (void)propagation_tree.tree->layout_root(cgpui::LayoutInput{});
  return propagation_tree;
}

int test_runtime_stops_event_propagation_when_target_consumes() {
  RuntimeFixture fixture;
  event_propagation_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_propagation_keyboard_sequence;
  fixture.view.consume_next_event = true;

  EventPropagationTree propagation_tree = make_event_propagation_tree();
  propagation_tree.target->result = cgpui::EventResult::consumed_event();
  propagation_tree.child->result = cgpui::EventResult::consumed_event();
  propagation_tree.root->result = cgpui::EventResult::consumed_event();

  cgpui::EventDispatchRecord dispatch_record{};
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(propagation_tree.tree));
  event_propagation_runtime = &runtime;
  event_propagation_target_id = propagation_tree.target_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  event_propagation_fixture = nullptr;
  event_propagation_runtime = nullptr;
  event_propagation_target_id = {};

  if (result != 0) {
    return 349;
  }
  if (propagation_tree.target->event_count != 1 ||
      propagation_tree.child->event_count != 0 ||
      propagation_tree.root->event_count != 0 ||
      fixture.view.event_count != 0) {
    return 350;
  }
  if (!dispatch_record.result.consumed || dispatch_record.result.cancelled) {
    return 351;
  }
  if (dispatch_record.route.element_ancestry.size() != 3 ||
      dispatch_record.route.element_ancestry[0] != propagation_tree.target_id ||
      dispatch_record.route.element_ancestry[1] != propagation_tree.child_id ||
      dispatch_record.route.element_ancestry[2] != propagation_tree.root_id) {
    return 352;
  }

  return 0;
}

int test_runtime_bubbles_unhandled_target_event_to_ancestor() {
  RuntimeFixture fixture;
  event_propagation_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_propagation_keyboard_sequence;
  fixture.view.consume_next_event = true;

  EventPropagationTree propagation_tree = make_event_propagation_tree();
  propagation_tree.child->result = cgpui::EventResult::consumed_event();
  propagation_tree.root->result = cgpui::EventResult::consumed_event();

  cgpui::EventDispatchRecord dispatch_record{};
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(propagation_tree.tree));
  event_propagation_runtime = &runtime;
  event_propagation_target_id = propagation_tree.target_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  event_propagation_fixture = nullptr;
  event_propagation_runtime = nullptr;
  event_propagation_target_id = {};

  if (result != 0) {
    return 353;
  }
  if (propagation_tree.target->event_count != 1 ||
      propagation_tree.child->event_count != 1 ||
      propagation_tree.root->event_count != 0 ||
      fixture.view.event_count != 0) {
    return 354;
  }
  if (propagation_tree.child->last_target_element_id !=
      propagation_tree.target_id) {
    return 355;
  }
  if (!dispatch_record.result.consumed || dispatch_record.result.cancelled) {
    return 356;
  }

  return 0;
}

int test_runtime_falls_back_to_view_after_unhandled_event_ancestry() {
  RuntimeFixture fixture;
  event_propagation_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_propagation_keyboard_sequence;
  fixture.view.consume_next_event = true;

  EventPropagationTree propagation_tree = make_event_propagation_tree();

  cgpui::EventDispatchRecord dispatch_record{};
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(propagation_tree.tree));
  event_propagation_runtime = &runtime;
  event_propagation_target_id = propagation_tree.target_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  event_propagation_fixture = nullptr;
  event_propagation_runtime = nullptr;
  event_propagation_target_id = {};

  if (result != 0) {
    return 357;
  }
  if (propagation_tree.target->event_count != 1 ||
      propagation_tree.child->event_count != 1 ||
      propagation_tree.root->event_count != 1 ||
      fixture.view.event_count != 1) {
    return 358;
  }
  if (fixture.view.last_route_element_id != propagation_tree.target_id ||
      !fixture.view.saw_event_route) {
    return 359;
  }
  if (!dispatch_record.result.consumed || dispatch_record.result.cancelled) {
    return 360;
  }

  return 0;
}

int test_runtime_skips_disabled_ancestors_during_event_bubbling() {
  RuntimeFixture fixture;
  event_propagation_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_propagation_keyboard_sequence;
  fixture.view.consume_next_event = true;

  EventPropagationTree propagation_tree = make_event_propagation_tree();
  propagation_tree.child->set_enabled(false);
  propagation_tree.child->result = cgpui::EventResult::consumed_event();
  propagation_tree.root->result = cgpui::EventResult::consumed_event();

  cgpui::EventDispatchRecord dispatch_record{};
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(propagation_tree.tree));
  event_propagation_runtime = &runtime;
  event_propagation_target_id = propagation_tree.target_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  event_propagation_fixture = nullptr;
  event_propagation_runtime = nullptr;
  event_propagation_target_id = {};

  if (result != 0) {
    return 361;
  }
  if (propagation_tree.target->event_count != 1 ||
      propagation_tree.child->event_count != 0 ||
      propagation_tree.root->event_count != 1 ||
      fixture.view.event_count != 0) {
    return 362;
  }
  if (propagation_tree.root->last_target_element_id !=
      propagation_tree.target_id) {
    return 363;
  }
  if (!dispatch_record.result.consumed || dispatch_record.result.cancelled) {
    return 364;
  }

  return 0;
}

RuntimeFixture* hover_state_fixture = nullptr;

void dispatch_hover_state_sequence() {
  auto& callback = hover_state_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
  callback(cgpui::PointerMoved{.position = {100.0F, 100.0F}});
}

int test_runtime_tracks_hovered_pointer_element() {
  RuntimeFixture fixture;
  hover_state_fixture = &fixture;
  fixture.app.on_run = &dispatch_hover_state_sequence;

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
    return 165;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  int callback_count = 0;
  std::optional<cgpui::ElementId> first_hover;
  std::optional<cgpui::ElementId> second_hover;
  std::optional<cgpui::ElementId> third_hover;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        callback_count += 1;
        if (callback_count == 1) {
          first_hover = context.input.hovered_element_id;
        } else if (callback_count == 2) {
          second_hover = context.input.hovered_element_id;
        } else if (callback_count == 3) {
          third_hover = context.input.hovered_element_id;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  hover_state_fixture = nullptr;

  if (result != 0) {
    return 166;
  }
  if (callback_count != 3 || fixture.view.pointer_move_count != 3) {
    return 167;
  }
  if (!first_hover.has_value() ||
      *first_hover != cgpui::ElementId{11}) {
    return 168;
  }
  if (!fixture.view.second_pointer_move_hovered_element_id.has_value() ||
      *fixture.view.second_pointer_move_hovered_element_id !=
          cgpui::ElementId{12}) {
    return 169;
  }
  if (!second_hover.has_value() ||
      *second_hover != cgpui::ElementId{12}) {
    return 170;
  }
  if (third_hover.has_value() ||
      fixture.view.last_hovered_element_id.has_value()) {
    return 171;
  }

  return 0;
}

RuntimeFixture* hover_style_invalidation_fixture = nullptr;
int hover_redraws_after_first_move = 0;
int hover_redraws_after_repeat_move = 0;
int hover_redraws_after_second_target = 0;
int hover_redraws_after_leave = 0;

void dispatch_hover_style_invalidation_sequence() {
  auto& callback = hover_style_invalidation_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  hover_redraws_after_first_move =
      hover_style_invalidation_fixture->window.request_redraw_count;
  callback(cgpui::PointerMoved{.position = {6.0F, 5.0F}});
  hover_redraws_after_repeat_move =
      hover_style_invalidation_fixture->window.request_redraw_count;
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
  hover_redraws_after_second_target =
      hover_style_invalidation_fixture->window.request_redraw_count;
  callback(cgpui::PointerMoved{.position = {100.0F, 100.0F}});
  hover_redraws_after_leave =
      hover_style_invalidation_fixture->window.request_redraw_count;
}

int test_hover_state_changes_request_style_invalidation_redraw() {
  RuntimeFixture fixture;
  hover_style_invalidation_fixture = &fixture;
  fixture.app.on_run = &dispatch_hover_style_invalidation_sequence;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{20});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{21});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  second->assign_id(cgpui::ElementId{22});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));
  (void)stack.layout(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  hover_style_invalidation_fixture = nullptr;

  if (result != 0) {
    return 290;
  }
  if (hover_redraws_after_first_move != 1 ||
      hover_redraws_after_repeat_move != 1 ||
      hover_redraws_after_second_target != 2 ||
      hover_redraws_after_leave != 3) {
    return 291;
  }
  if (fixture.renderer.begin_frame_count != 3 ||
      fixture.view.paint_count != 3) {
    return 292;
  }
  return 0;
}

RuntimeFixture* cursor_shape_fixture = nullptr;

void dispatch_cursor_shape_sequence() {
  auto& callback = cursor_shape_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
  callback(cgpui::PointerMoved{.position = {100.0F, 100.0F}});
}

int test_runtime_routes_cursor_shape_from_hovered_element() {
  RuntimeFixture fixture;
  cursor_shape_fixture = &fixture;
  fixture.app.on_run = &dispatch_cursor_shape_sequence;

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
    return 178;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);
  runtime.set_element_cursor(
      cgpui::ElementId{11},
      cgpui::CursorShape::pointing_hand);
  runtime.set_element_cursor(cgpui::ElementId{12}, cgpui::CursorShape::text);

  int callback_count = 0;
  cgpui::CursorShape first_cursor = cgpui::CursorShape::default_arrow;
  cgpui::CursorShape second_cursor = cgpui::CursorShape::default_arrow;
  cgpui::CursorShape third_cursor = cgpui::CursorShape::pointing_hand;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        callback_count += 1;
        if (callback_count == 1) {
          first_cursor = context.input.cursor_shape;
        } else if (callback_count == 2) {
          second_cursor = context.input.cursor_shape;
        } else if (callback_count == 3) {
          third_cursor = context.input.cursor_shape;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  cursor_shape_fixture = nullptr;

  if (result != 0) {
    return 179;
  }
  if (callback_count != 3 || fixture.view.pointer_move_count != 3) {
    return 180;
  }
  if (first_cursor != cgpui::CursorShape::pointing_hand) {
    return 181;
  }
  if (fixture.view.second_pointer_move_cursor_shape !=
          cgpui::CursorShape::text ||
      second_cursor != cgpui::CursorShape::text) {
    return 182;
  }
  if (fixture.view.last_cursor_shape != cgpui::CursorShape::default_arrow ||
      third_cursor != cgpui::CursorShape::default_arrow) {
    return 183;
  }
  if (fixture.window.set_cursor_count != 3 ||
      fixture.window.last_cursor_shape != cgpui::CursorShape::default_arrow) {
    return 184;
  }

  return 0;
}

RuntimeFixture* platform_cursor_fixture = nullptr;

void dispatch_platform_cursor_sequence() {
  auto& callback = platform_cursor_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
}

int test_runtime_applies_hover_cursor_to_platform_window() {
  RuntimeFixture fixture;
  platform_cursor_fixture = &fixture;
  fixture.app.on_run = &dispatch_platform_cursor_sequence;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{40});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{41});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  second->assign_id(cgpui::ElementId{42});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));
  (void)stack.layout(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);
  runtime.set_element_cursor(cgpui::ElementId{41}, cgpui::CursorShape::text);
  runtime.set_element_cursor(
      cgpui::ElementId{42},
      cgpui::CursorShape::pointing_hand);

  const int result = runtime.run(cgpui::WindowDescriptor{});
  platform_cursor_fixture = nullptr;

  if (result != 0) {
    return 901;
  }
  if (fixture.window.set_cursor_count != 2) {
    return 902;
  }
  if (fixture.window.last_cursor_shape != cgpui::CursorShape::pointing_hand) {
    return 903;
  }

  return 0;
}

RuntimeFixture* disabled_hover_cursor_fixture = nullptr;

void dispatch_disabled_hover_cursor_sequence() {
  auto& callback = disabled_hover_cursor_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_runtime_clears_cursor_when_hovered_element_becomes_disabled() {
  RuntimeFixture fixture;
  disabled_hover_cursor_fixture = &fixture;
  fixture.app.on_run = &dispatch_disabled_hover_cursor_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  cgpui::FixedSizeElement* root_ptr = root.get();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.set_element_cursor(root_id, cgpui::CursorShape::text);

  int pointer_move_count = 0;
  cgpui::CursorShape first_cursor = cgpui::CursorShape::default_arrow;
  cgpui::CursorShape second_cursor = cgpui::CursorShape::text;
  std::optional<cgpui::ElementId> second_hover;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind != cgpui::EventKind::pointer_moved) {
          return;
        }
        pointer_move_count += 1;
        if (pointer_move_count == 1) {
          first_cursor = context.input.cursor_shape;
          root_ptr->set_enabled(false);
        } else if (pointer_move_count == 2) {
          second_cursor = context.input.cursor_shape;
          second_hover = context.input.hovered_element_id;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = true});
  disabled_hover_cursor_fixture = nullptr;

  if (result != 0) {
    return 285;
  }
  if (root_id.value == 0 || pointer_move_count != 2) {
    return 286;
  }
  if (first_cursor != cgpui::CursorShape::text) {
    return 287;
  }
  if (!second_hover.has_value() || *second_hover != root_id) {
    return 288;
  }
  if (second_cursor != cgpui::CursorShape::default_arrow) {
    return 289;
  }
  return 0;
}

RuntimeFixture* view_context_cursor_fixture = nullptr;

void dispatch_view_context_cursor_binding_sequence() {
  auto& callback = view_context_cursor_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_view_context_binds_cursor_shape_to_element() {
  RuntimeFixture fixture;
  view_context_cursor_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_context_cursor_binding_sequence;
  fixture.view.exercise_view_context_cursor_binding = true;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{30});
  auto target = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  target->assign_id(cgpui::ElementId{31});
  stack.append_child(std::move(target));
  (void)stack.layout(cgpui::LayoutInput{});
  fixture.view.view_context_cursor_element_id = cgpui::ElementId{31};

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  view_context_cursor_fixture = nullptr;

  if (result != 0) {
    return 246;
  }
  if (fixture.view.keyboard_key_count != 1 ||
      fixture.view.pointer_move_count != 1) {
    return 247;
  }
  if (!fixture.view.last_hovered_element_id.has_value() ||
      *fixture.view.last_hovered_element_id != cgpui::ElementId{31}) {
    return 248;
  }
  if (fixture.view.last_cursor_shape != cgpui::CursorShape::text) {
    return 249;
  }

  return 0;
}

RuntimeFixture* pointer_capture_fixture = nullptr;

void dispatch_pointer_capture_sequence() {
  auto& callback = pointer_capture_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {1.0F, 1.0F}});
  callback(cgpui::PointerMoved{.position = {2.0F, 2.0F}});
  callback(cgpui::PointerMoved{.position = {3.0F, 3.0F}});
  callback(cgpui::PointerMoved{.position = {4.0F, 4.0F}});
}

int test_view_owner_can_capture_and_release_pointer() {
  RuntimeFixture fixture;
  pointer_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_capture_sequence;
  fixture.view.capture_on_first_pointer_move = true;
  fixture.view.release_on_third_pointer_move = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_capture_fixture = nullptr;

  if (result != 0) {
    return 60;
  }
  if (fixture.view.pointer_move_count != 4) {
    return 61;
  }
  if (!fixture.view.second_pointer_move_saw_capture) {
    return 62;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.last_pointer_captured) {
    return 63;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{4.0F, 4.0F})) {
    return 64;
  }

  return 0;
}

int test_pointer_capture_tracks_owner_view_id() {
  RuntimeFixture fixture;
  pointer_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_capture_sequence;
  fixture.view.capture_pointer_owner_on_first_pointer_move = true;
  fixture.view.release_pointer_with_wrong_owner_on_second_pointer_move = true;
  fixture.view.release_pointer_owner_on_third_pointer_move = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_capture_fixture = nullptr;

  if (result != 0) {
    return 90;
  }
  if (!fixture.view.saw_first_view_id ||
      fixture.view.first_view_id.value == 0 ||
      !fixture.view.view_id_stayed_stable) {
    return 91;
  }
  if (!fixture.view.second_pointer_move_saw_capture ||
      !fixture.view.second_pointer_move_saw_capture_owner) {
    return 92;
  }
  if (!fixture.view.third_pointer_move_saw_capture ||
      !fixture.view.third_pointer_move_saw_capture_owner) {
    return 93;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.fourth_pointer_move_saw_capture_owner ||
      fixture.view.last_pointer_capture_owner_present) {
    return 94;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{4.0F, 4.0F})) {
    return 95;
  }

  return 0;
}

RuntimeFixture* pointer_element_capture_fixture = nullptr;

void dispatch_pointer_element_capture_sequence() {
  auto& callback = pointer_element_capture_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
  callback(cgpui::PointerMoved{.position = {30.0F, 15.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
}

int test_pointer_capture_routes_to_owner_element() {
  RuntimeFixture fixture;
  pointer_element_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_element_capture_sequence;
  fixture.view.capture_route_element_on_first_pointer_move = true;
  fixture.view.release_pointer_element_with_wrong_owner_on_second_pointer_move =
      true;
  fixture.view.release_pointer_element_owner_on_third_pointer_move = true;

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
    return 140;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  int callback_count = 0;
  cgpui::EventDispatchRecord first_record{};
  cgpui::EventDispatchRecord second_record{};
  cgpui::EventDispatchRecord third_record{};
  cgpui::EventDispatchRecord fourth_record{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (callback_count == 1) {
          first_record = record;
        } else if (callback_count == 2) {
          second_record = record;
        } else if (callback_count == 3) {
          third_record = record;
        } else if (callback_count == 4) {
          fourth_record = record;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_element_capture_fixture = nullptr;

  if (result != 0) {
    return 131;
  }
  if (callback_count != 4 || fixture.view.pointer_move_count != 4) {
    return 132;
  }
  if (fixture.view.captured_pointer_element_id != cgpui::ElementId{11}) {
    return 133;
  }
  if (!first_record.route.target_element_id.has_value() ||
      *first_record.route.target_element_id != cgpui::ElementId{11}) {
    return 134;
  }
  if (!second_record.route.target_element_id.has_value() ||
      *second_record.route.target_element_id != cgpui::ElementId{11}) {
    return 135;
  }
  if (!third_record.route.target_element_id.has_value() ||
      *third_record.route.target_element_id != cgpui::ElementId{11}) {
    return 136;
  }
  if (!fourth_record.route.target_element_id.has_value() ||
      *fourth_record.route.target_element_id != cgpui::ElementId{12}) {
    return 137;
  }
  if (!fixture.view.second_pointer_move_saw_capture ||
      !fixture.view.second_pointer_move_saw_element_capture_owner ||
      !fixture.view.third_pointer_move_saw_capture ||
      !fixture.view.third_pointer_move_saw_element_capture_owner) {
    return 138;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.fourth_pointer_move_saw_element_capture_owner ||
      fixture.view.last_pointer_capture_owner_matches_captured_element) {
    return 139;
  }

  return 0;
}

int test_view_context_captures_and_releases_pointer_for_element() {
  RuntimeFixture fixture;
  pointer_element_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_element_capture_sequence;
  fixture.view.exercise_view_context_pointer_capture_helpers = true;

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
  (void)stack.layout(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_element_capture_fixture = nullptr;

  if (result != 0) {
    return 254;
  }
  if (fixture.view.pointer_move_count != 4 ||
      fixture.view.captured_pointer_element_id != cgpui::ElementId{11}) {
    return 255;
  }
  if (!fixture.view.second_pointer_move_saw_capture ||
      !fixture.view.second_pointer_move_saw_element_capture_owner ||
      !fixture.view.third_pointer_move_saw_capture ||
      !fixture.view.third_pointer_move_saw_element_capture_owner) {
    return 256;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.fourth_pointer_move_saw_element_capture_owner ||
      fixture.view.last_pointer_capture_owner_present) {
    return 257;
  }

  return 0;
}

RuntimeFixture* hover_with_capture_fixture = nullptr;

void dispatch_hover_with_capture_sequence() {
  auto& callback = hover_with_capture_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
}

int test_hover_tracks_hit_element_while_pointer_is_captured() {
  RuntimeFixture fixture;
  hover_with_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_hover_with_capture_sequence;
  fixture.view.capture_route_element_on_first_pointer_move = true;

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
    return 172;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  const int result = runtime.run(cgpui::WindowDescriptor{});
  hover_with_capture_fixture = nullptr;

  if (result != 0) {
    return 173;
  }
  if (fixture.view.captured_pointer_element_id != cgpui::ElementId{11}) {
    return 174;
  }
  if (!fixture.view.second_pointer_move_saw_capture ||
      !fixture.view.second_pointer_move_saw_element_capture_owner) {
    return 175;
  }
  if (fixture.view.last_route_element_id != cgpui::ElementId{11}) {
    return 176;
  }
  if (!fixture.view.second_pointer_move_hovered_element_id.has_value() ||
      *fixture.view.second_pointer_move_hovered_element_id !=
          cgpui::ElementId{12} ||
      fixture.view.last_hovered_element_id != cgpui::ElementId{12}) {
    return 177;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_runtime_dispatches_input_events_to_view(); result != 0) {
    return result;
  }
  if (const int result = test_view_event_can_request_redraw(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_exposes_last_view_event_result(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_reports_each_view_event_dispatch(); result != 0) {
    return result;
  }
  if (const int result = test_event_router_routes_events_to_root_view(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_exposes_current_event_route(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_current_event_route_helper_matches_route_field(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_input_state_helper_matches_input_snapshot(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_pointer_events_to_hit_element(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_drag_drop_events_to_hit_element(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_dispatches_consumed_element_event_before_view_fallback(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_falls_back_to_view_after_unhandled_element_event(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_skips_disabled_element_event_and_falls_back_to_view(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_respects_hidden_overflow_clip_during_hit_testing(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_stops_event_propagation_when_target_consumes(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_bubbles_unhandled_target_event_to_ancestor(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_falls_back_to_view_after_unhandled_event_ancestry(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_skips_disabled_ancestors_during_event_bubbling(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_tracks_hovered_pointer_element(); result != 0) {
    return result;
  }
  if (const int result = test_hover_state_changes_request_style_invalidation_redraw(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_cursor_shape_from_hovered_element(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_applies_hover_cursor_to_platform_window(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_clears_cursor_when_hovered_element_becomes_disabled(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_binds_cursor_shape_to_element(); result != 0) {
    return result;
  }
  if (const int result = test_view_owner_can_capture_and_release_pointer(); result != 0) {
    return result;
  }
  if (const int result = test_pointer_capture_tracks_owner_view_id(); result != 0) {
    return result;
  }
  if (const int result = test_pointer_capture_routes_to_owner_element(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_captures_and_releases_pointer_for_element(); result != 0) {
    return result;
  }
  if (const int result = test_hover_tracks_hit_element_while_pointer_is_captured(); result != 0) {
    return result;
  }
  return 0;
}
