#include "../ui/window_runtime_test_support.hpp"
#include "win32_accessibility_internal.hpp"
#include "win32_uia_event_test_support.hpp"

namespace {

RuntimeFixture* integration_fixture = nullptr;
cgpui::WindowRuntime* integration_runtime = nullptr;
cgpui::TextModel* integration_model = nullptr;
cgpui::ElementId integration_input_id;

void dispatch_runtime_changes() {
  integration_runtime->request_keyboard_focus(integration_input_id);
  integration_model->insert_text("!");
  integration_fixture->window.request_redraw();
}

} // namespace

int main() {
  using namespace cgpui::test;
  reset_uia_event_calls();

  RuntimeFixture fixture;
  fixture.app.on_run = dispatch_runtime_changes;
  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.set_event_operations(test_uia_event_operations());
  fixture.window.accessibility_update_callback =
      [&](const cgpui::PlatformAccessibilityTreeUpdate& update) {
        adapter.update(update);
      };

  cgpui::TextModel model("query");
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(cgpui::into_element(cgpui::v_stack()));
  const cgpui::ElementId input_id =
      tree->append_child(root_id, cgpui::text_input(model).build());
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  integration_fixture = &fixture;
  integration_runtime = &runtime;
  integration_model = &model;
  integration_input_id = input_id;
  const int result = runtime.run(cgpui::WindowDescriptor{});
  integration_fixture = nullptr;
  integration_runtime = nullptr;
  integration_model = nullptr;
  integration_input_id = {};

  if (result != 0 || fixture.window.accessibility_update_count != 3 ||
      model.text() != "query!") return 1;
  if (uia_property_calls.size() != 2 || uia_event_calls.size() != 2) return 2;
  if (uia_property_calls[0].property != UIA_HasKeyboardFocusPropertyId ||
      !uia_bool_equals(uia_property_calls[0].old_value, false) ||
      !uia_bool_equals(uia_property_calls[0].new_value, true) ||
      uia_property_calls[1].property != UIA_ValueValuePropertyId ||
      !uia_bstr_equals(uia_property_calls[1].old_value, L"query") ||
      !uia_bstr_equals(uia_property_calls[1].new_value, L"query!")) return 3;
  if (uia_event_calls[0].event != UIA_AutomationFocusChangedEventId ||
      uia_event_calls[1].event != UIA_Text_TextChangedEventId) return 4;
  if (adapter.last_live_updates().size() != 2 ||
      adapter.last_event_publication().property_changed_count != 1 ||
      adapter.last_event_publication().automation_event_count != 1) return 5;
  reset_uia_event_calls();
  return 0;
}
