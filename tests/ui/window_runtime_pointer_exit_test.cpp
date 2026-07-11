#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* pointer_exit_fixture = nullptr;

void dispatch_pointer_exit_sequence() {
  auto& callback = pointer_exit_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerExited{.position = {5.0F, 5.0F}});
}

} // namespace

int main() {
  RuntimeFixture fixture;
  pointer_exit_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_exit_sequence;

  cgpui::FixedSizeElement root(
      cgpui::Size{.width = 40.0F, .height = 40.0F});
  const cgpui::ElementId root_id{71};
  root.assign_id(root_id);
  (void)root.layout(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&root);
  runtime.set_element_cursor(root_id, cgpui::CursorShape::text);

  int callback_count = 0;
  bool move_hovered = false;
  bool exit_cleared_hover = false;
  cgpui::CursorShape move_cursor = cgpui::CursorShape::default_arrow;
  cgpui::CursorShape exit_cursor = cgpui::CursorShape::text;
  cgpui::Point exit_position{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (record.event_kind == cgpui::EventKind::pointer_moved) {
          move_hovered = context.input.hovered_element_id == root_id;
          move_cursor = context.input.cursor_shape;
        } else if (record.event_kind == cgpui::EventKind::pointer_exited) {
          exit_cleared_hover = !context.input.hovered_element_id.has_value();
          exit_cursor = context.input.cursor_shape;
          exit_position = context.input.pointer_position;
        }
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  pointer_exit_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (callback_count != 2 || fixture.view.event_count != 2) {
    return 2;
  }
  if (!move_hovered || move_cursor != cgpui::CursorShape::text) {
    return 3;
  }
  if (!exit_cleared_hover ||
      exit_cursor != cgpui::CursorShape::default_arrow ||
      !equal(exit_position, cgpui::Point{5.0F, 5.0F})) {
    return 4;
  }
  if (fixture.window.set_cursor_count != 2 ||
      fixture.window.last_cursor_shape != cgpui::CursorShape::default_arrow) {
    return 5;
  }
  return 0;
}
