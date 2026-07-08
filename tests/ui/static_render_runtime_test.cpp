#include "window_runtime_test_support.hpp"

#include <array>
#include <optional>

namespace {

class StaticRuntimeView final : public cgpui::View {
 public:
  bool supports_static_render() const override { return true; }

  cgpui::StaticElementTreeView render_static(
      cgpui::ViewContext& context) override {
    static_render_count += 1;
    saw_context_view_id = context.view_id == cgpui::ViewId{1};
    saw_context_viewport_size =
        equal(context.viewport_size, cgpui::Size{640.0F, 480.0F});
    nodes_[0].bounds.size = context.viewport_size;
    return cgpui::StaticElementTreeView{
        nodes_,
        child_ids_,
        cgpui::ElementId{1},
    };
  }

  cgpui::AnyElement render(cgpui::ViewContext&) override {
    dynamic_render_count += 1;
    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }

  void paint(cgpui::PaintList&, cgpui::Size viewport_size) override {
    paint_count += 1;
    last_paint_viewport_size = viewport_size;
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent&,
      const cgpui::WindowRuntimeContext& context) override {
    event_count += 1;
    saw_event_route = context.event_route.has_value();
    if (context.event_route.has_value()) {
      last_target_element_id = context.event_route->target_element_id;
      last_element_ancestry_size =
          context.event_route->element_ancestry.size();
      if (context.event_route->element_ancestry.size() >= 2) {
        last_first_ancestor = context.event_route->element_ancestry[0];
        last_second_ancestor = context.event_route->element_ancestry[1];
      }
    }
    last_hovered_element_id = context.input.hovered_element_id;
    last_active_element_id = context.input.active_element_id;
    last_keyboard_focus_element_id =
        context.input.keyboard_focus_element_owner;
    return cgpui::EventResult::unhandled();
  }

  int static_render_count = 0;
  int dynamic_render_count = 0;
  int paint_count = 0;
  int event_count = 0;
  bool saw_context_view_id = false;
  bool saw_context_viewport_size = false;
  bool saw_event_route = false;
  cgpui::Size last_paint_viewport_size{};
  std::optional<cgpui::ElementId> last_target_element_id;
  std::optional<cgpui::ElementId> last_hovered_element_id;
  std::optional<cgpui::ElementId> last_active_element_id;
  std::optional<cgpui::ElementId> last_keyboard_focus_element_id;
  cgpui::ElementId last_first_ancestor{};
  cgpui::ElementId last_second_ancestor{};
  std::size_t last_element_ancestry_size = 0;

 private:
  std::array<cgpui::StaticElementNode, 3> nodes_{
      cgpui::StaticElementNode{
          .id = cgpui::ElementId{1},
          .children =
              cgpui::StaticElementChildRange{.first = 0, .count = 2},
          .kind = cgpui::StaticElementKind::container,
          .bounds = cgpui::Rect{
              .origin = {0.0F, 0.0F},
              .size = {640.0F, 480.0F},
          },
      },
      cgpui::StaticElementNode{
          .id = cgpui::ElementId{2},
          .parent_id = cgpui::ElementId{1},
          .kind = cgpui::StaticElementKind::text,
          .bounds = cgpui::Rect{
              .origin = {10.0F, 10.0F},
              .size = {80.0F, 40.0F},
          },
          .intrinsic_size = {80.0F, 40.0F},
          .text = "static child",
          .focusable = true,
      },
      cgpui::StaticElementNode{
          .id = cgpui::ElementId{3},
          .parent_id = cgpui::ElementId{1},
          .kind = cgpui::StaticElementKind::image,
          .bounds = cgpui::Rect{
              .origin = {100.0F, 10.0F},
              .size = {20.0F, 20.0F},
          },
          .enabled = false,
      },
  };
  std::array<cgpui::ElementId, 2> child_ids_{
      cgpui::ElementId{2},
      cgpui::ElementId{3},
  };
};

class DynamicFallbackView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    render_count += 1;
    return cgpui::into_element(cgpui::div().size(4.0F, 5.0F));
  }

  void paint(cgpui::PaintList&, cgpui::Size) override { paint_count += 1; }

  int render_count = 0;
  int paint_count = 0;
};

RuntimeFixture* static_event_fixture = nullptr;

void dispatch_static_pointer_sequence() {
  auto& callback = static_event_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {20.0F, 20.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .position = {20.0F, 20.0F},
  });
}

int test_static_render_installs_tree_without_dynamic_render() {
  RuntimeFixture fixture;
  StaticRuntimeView view;
  fixture.app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  if (result != 0) {
    return 1;
  }
  if (view.static_render_count != 1 || view.dynamic_render_count != 0 ||
      view.paint_count != 1 || !view.saw_context_view_id ||
      !view.saw_context_viewport_size ||
      !equal(view.last_paint_viewport_size, cgpui::Size{640.0F, 480.0F})) {
    return 2;
  }
  if (runtime.element_tree() != nullptr || runtime.element_root() != nullptr) {
    return 3;
  }
  const cgpui::StaticElementTreeView* static_tree =
      runtime.static_element_tree();
  if (static_tree == nullptr || !static_tree->valid() ||
      static_tree->root_id() != cgpui::ElementId{1} ||
      static_tree->size() != 3) {
    return 4;
  }

  const std::optional<cgpui::RenderRecord> record =
      runtime.last_render_record();
  if (!record.has_value() ||
      record->tree_kind != cgpui::RenderTreeKind::static_element_tree ||
      record->root_element_id != cgpui::ElementId{1} ||
      record->static_node_count != 3) {
    return 5;
  }
  const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
      runtime.diagnostics_snapshot();
  if (!diagnostics.last_render_record.has_value() ||
      diagnostics.last_render_record->tree_kind !=
          cgpui::RenderTreeKind::static_element_tree ||
      diagnostics.last_render_record->static_node_count != 3) {
    return 6;
  }
  if (fixture.window.accessibility_update_count != 1 ||
      !fixture.window.last_accessibility_update.has_value() ||
      fixture.window.last_accessibility_update->root_element_id != 1 ||
      fixture.window.last_accessibility_update->node_count != 3) {
    return 7;
  }

  return 0;
}

int test_static_render_routes_events_through_static_tree() {
  RuntimeFixture fixture;
  StaticRuntimeView view;
  static_event_fixture = &fixture;
  fixture.app.on_run = &dispatch_static_pointer_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  static_event_fixture = nullptr;
  if (result != 0) {
    return 8;
  }
  if (view.dynamic_render_count != 0 || view.static_render_count < 2 ||
      view.event_count != 2 || !view.saw_event_route) {
    return 9;
  }
  if (view.last_target_element_id != cgpui::ElementId{2} ||
      view.last_hovered_element_id != cgpui::ElementId{2} ||
      view.last_active_element_id != cgpui::ElementId{2} ||
      view.last_keyboard_focus_element_id != cgpui::ElementId{2}) {
    return 10;
  }
  if (view.last_element_ancestry_size != 2 ||
      view.last_first_ancestor != cgpui::ElementId{2} ||
      view.last_second_ancestor != cgpui::ElementId{1}) {
    return 11;
  }
  const cgpui::ViewInputState input = runtime.input_state();
  if (input.hovered_element_id != cgpui::ElementId{2} ||
      input.active_element_id != cgpui::ElementId{2} ||
      input.keyboard_focus_element_owner != cgpui::ElementId{2}) {
    return 12;
  }

  return 0;
}

int test_dynamic_render_remains_explicit_fallback() {
  RuntimeFixture fixture;
  DynamicFallbackView view;
  fixture.app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  if (result != 0) {
    return 13;
  }
  if (view.render_count != 1 || view.paint_count != 1 ||
      runtime.static_element_tree() != nullptr ||
      runtime.element_tree() == nullptr || runtime.element_root() == nullptr) {
    return 14;
  }
  const std::optional<cgpui::RenderRecord> record =
      runtime.last_render_record();
  return record.has_value() &&
                 record->tree_kind ==
                     cgpui::RenderTreeKind::dynamic_element_tree &&
                 record->root_element_id.has_value()
             ? 0
             : 15;
}

} // namespace

int main() {
  if (const int result =
          test_static_render_installs_tree_without_dynamic_render();
      result != 0) {
    return result;
  }
  if (const int result = test_static_render_routes_events_through_static_tree();
      result != 0) {
    return result;
  }
  return test_dynamic_render_remains_explicit_fallback();
}
