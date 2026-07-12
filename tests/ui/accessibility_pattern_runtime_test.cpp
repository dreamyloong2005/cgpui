#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* action_fixture = nullptr;
cgpui::ElementId action_element_id;

void dispatch_accessibility_action() {
  action_fixture->window.callback(cgpui::AccessibilityActionRequested{
      .kind = cgpui::AccessibilityActionKind::invoke,
      .element_id = action_element_id.value,
  });
}

} // namespace

int main() {
  RuntimeFixture fixture;
  action_fixture = &fixture;
  fixture.app.on_run = &dispatch_accessibility_action;

  auto tree = std::make_unique<cgpui::ElementTree>();
  action_element_id = tree->set_root(std::make_unique<cgpui::ButtonElement>(
      "dialog.accept", cgpui::StyleState{}));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  int action_count = 0;
  runtime.register_view_action(
      cgpui::ViewId{1},
      "dialog.accept",
      [&](const cgpui::WindowRuntimeContext&) {
        action_count += 1;
        return cgpui::EventResult::consumed_event();
      });

  std::optional<cgpui::EventDispatchRecord> accessibility_dispatch;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::accessibility_action) {
          accessibility_dispatch = record;
        }
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  action_fixture = nullptr;

  if (result != 0 || action_count != 1) return 1;
  if (!accessibility_dispatch.has_value() ||
      accessibility_dispatch->route.target_element_id != action_element_id ||
      !accessibility_dispatch->result.consumed ||
      accessibility_dispatch->result.cancelled) return 2;
  if (fixture.window.accessibility_update_count == 0 ||
      fixture.window.request_redraw_count == 0) return 3;
  return 0;
}
