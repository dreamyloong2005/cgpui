#include "window_runtime_test_support.hpp"

namespace {
RuntimeFixture* view_identity_fixture = nullptr;

void dispatch_view_identity_sequence() {
  auto& callback = view_identity_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 80,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 81,
      .action = cgpui::KeyAction::pressed});
}

int test_context_allocates_stable_view_ids() {
  RuntimeFixture fixture;
  view_identity_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_identity_sequence;
  fixture.view.exercise_view_identity_allocation = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  view_identity_fixture = nullptr;

  if (result != 0) {
    return 117;
  }
  if (fixture.view.keyboard_key_count != 2) {
    return 118;
  }
  if (!fixture.view.root_view_id_was_allocated ||
      fixture.view.zero_view_id_was_allocated) {
    return 119;
  }
  if (fixture.view.first_allocated_view_id.value <=
          fixture.view.first_view_id.value ||
      fixture.view.second_allocated_view_id.value <=
          fixture.view.first_allocated_view_id.value ||
      fixture.view.third_allocated_view_id.value <=
          fixture.view.second_allocated_view_id.value) {
    return 120;
  }
  if (!fixture.view.first_allocated_view_id_was_allocated ||
      !fixture.view.second_allocated_view_id_was_allocated ||
      !fixture.view.first_allocated_view_id_stayed_allocated ||
      !fixture.view.second_allocated_view_id_stayed_allocated ||
      !fixture.view.third_allocated_view_id_was_allocated) {
    return 121;
  }
  if (!fixture.view.next_unallocated_view_id_was_missing ||
      fixture.view.third_allocated_view_id.value !=
          fixture.view.second_allocated_view_id.value + 1) {
    return 122;
  }

  return 0;
}

int test_runtime_registers_finds_and_removes_views() {
  RuntimeFixture fixture;
  RegistryView stack_view;
  auto owned_child_view = std::make_unique<RegistryView>();
  RegistryView* owned_child_ptr = owned_child_view.get();

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const cgpui::ViewId root_view_id{1};
  const cgpui::View* root_view = runtime.root_view();
  if (root_view != &fixture.view ||
      runtime.find_view(root_view_id) != &fixture.view ||
      !runtime.upgrade_view(cgpui::WeakView(root_view_id)).has_value()) {
    return 327;
  }

  const cgpui::ViewId stack_view_id = runtime.register_view(stack_view);
  const cgpui::ViewId owned_child_view_id =
      runtime.register_view(std::move(owned_child_view));

  if (stack_view_id.value <= root_view_id.value ||
      owned_child_view_id.value <= stack_view_id.value ||
      stack_view_id == owned_child_view_id) {
    return 328;
  }
  if (runtime.find_view(stack_view_id) != &stack_view ||
      runtime.find_view(owned_child_view_id) != owned_child_ptr) {
    return 329;
  }
  if (!runtime.is_view_id_allocated(stack_view_id) ||
      !runtime.is_view_id_allocated(owned_child_view_id)) {
    return 330;
  }

  const bool removed_child = runtime.remove_view(owned_child_view_id);
  if (!removed_child ||
      runtime.find_view(owned_child_view_id) != nullptr ||
      runtime.is_view_id_allocated(owned_child_view_id) ||
      runtime.upgrade_view(cgpui::WeakView(owned_child_view_id)).has_value()) {
    return 331;
  }
  if (runtime.remove_view(root_view_id) ||
      runtime.find_view(root_view_id) != &fixture.view ||
      !runtime.is_view_id_allocated(root_view_id)) {
    return 332;
  }

  const cgpui::ViewId next_view_id = runtime.register_view(
      std::make_unique<RegistryView>());
  if (next_view_id.value <= owned_child_view_id.value ||
      runtime.find_view(next_view_id) == nullptr) {
    return 333;
  }

  return 0;
}

int test_registered_child_view_can_be_embedded_as_placeholder_element() {
  RuntimeFixture fixture;
  RegistryView child_view;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const cgpui::ViewId child_view_id = runtime.register_view(child_view);
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(cgpui::child_view(child_view_id)
                         .size(cgpui::Size{.width = 96.0F, .height = 64.0F})
                         .build());
  runtime.set_element_tree(std::move(tree));

  if (runtime.find_view(child_view_id) != &child_view) {
    return 334;
  }
  const cgpui::ElementTree* installed_tree = runtime.element_tree();
  if (installed_tree == nullptr || installed_tree->root_id() != root_id) {
    return 335;
  }

  const auto* placeholder =
      installed_tree->find_as<cgpui::ChildViewElement>(root_id);
  if (placeholder == nullptr || placeholder->view_id() != child_view_id) {
    return 336;
  }

  const cgpui::LayoutOutput output =
      installed_tree->layout_root(cgpui::LayoutInput{});
  if (output.size.width != 96.0F || output.size.height != 64.0F) {
    return 337;
  }

  if (installed_tree->hit_test_root(cgpui::Point{.x = 8.0F, .y = 8.0F}) !=
      root_id) {
    return 338;
  }
  return installed_tree->hit_test_root(cgpui::Point{.x = 96.0F, .y = 8.0F})
                     .value == 0
             ? 0
             : 339;
}

RuntimeFixture* element_route_ancestry_fixture = nullptr;
cgpui::WindowRuntime* element_route_ancestry_runtime = nullptr;
cgpui::ElementId element_route_ancestry_target_id;

void dispatch_element_route_ancestry_sequence() {
  auto& callback = element_route_ancestry_fixture->window.callback;
  element_route_ancestry_runtime->request_keyboard_focus(
      element_route_ancestry_target_id);
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
}

int test_event_route_carries_element_ancestry() {
  RuntimeFixture fixture;
  element_route_ancestry_fixture = &fixture;
  fixture.app.on_run = &dispatch_element_route_ancestry_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 80.0F, .height = 80.0F}));
  const cgpui::ElementId child_id = tree->append_child(
      root_id,
      std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 40.0F, .height = 40.0F}));
  const cgpui::ElementId grandchild_id = tree->append_child(
      child_id,
      std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 20.0F, .height = 20.0F}));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  element_route_ancestry_runtime = &runtime;
  element_route_ancestry_target_id = grandchild_id;

  int callback_count = 0;
  cgpui::EventDispatchRecord dispatch_record{};
  bool callback_context_matched_record_route = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        dispatch_record = record;
        callback_context_matched_record_route =
            context.event_route.has_value() &&
            context.current_event_route().has_value() &&
            context.event_route->element_ancestry ==
                record.route.element_ancestry &&
            context.current_event_route()->view_ancestry ==
                record.route.view_ancestry;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  element_route_ancestry_fixture = nullptr;
  element_route_ancestry_runtime = nullptr;
  element_route_ancestry_target_id = {};

  if (result != 0) {
    return 340;
  }
  if (callback_count != 1 || !dispatch_record.route.target_element_id ||
      *dispatch_record.route.target_element_id != grandchild_id) {
    return 341;
  }
  if (dispatch_record.route.element_ancestry.size() != 3 ||
      dispatch_record.route.element_ancestry[0] != grandchild_id ||
      dispatch_record.route.element_ancestry[1] != child_id ||
      dispatch_record.route.element_ancestry[2] != root_id) {
    return 342;
  }
  if (dispatch_record.route.view_ancestry.size() != 1 ||
      dispatch_record.route.view_ancestry[0] != cgpui::ViewId{1}) {
    return 343;
  }
  return callback_context_matched_record_route ? 0 : 344;
}

RuntimeFixture* child_view_route_ancestry_fixture = nullptr;

void dispatch_child_view_route_ancestry_sequence() {
  auto& callback = child_view_route_ancestry_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {8.0F, 8.0F}});
}

int test_event_route_carries_child_view_ancestry() {
  RuntimeFixture fixture;
  child_view_route_ancestry_fixture = &fixture;
  fixture.app.on_run = &dispatch_child_view_route_ancestry_sequence;
  RegistryView child_view;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  const cgpui::ViewId child_view_id = runtime.register_view(child_view);

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(cgpui::child_view(child_view_id)
                         .size(cgpui::Size{.width = 96.0F, .height = 64.0F})
                         .build());
  (void)tree->layout_root(cgpui::LayoutInput{});
  runtime.set_element_tree(std::move(tree));

  int callback_count = 0;
  cgpui::EventDispatchRecord dispatch_record{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  child_view_route_ancestry_fixture = nullptr;

  if (result != 0) {
    return 345;
  }
  if (callback_count != 1 || dispatch_record.route.target_view_id !=
                                 child_view_id) {
    return 346;
  }
  if (!dispatch_record.route.target_element_id ||
      *dispatch_record.route.target_element_id != root_id ||
      dispatch_record.route.element_ancestry.size() != 1 ||
      dispatch_record.route.element_ancestry[0] != root_id) {
    return 347;
  }
  if (dispatch_record.route.view_ancestry.size() != 2 ||
      dispatch_record.route.view_ancestry[0] != child_view_id ||
      dispatch_record.route.view_ancestry[1] != cgpui::ViewId{1}) {
    return 348;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_context_allocates_stable_view_ids(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_registers_finds_and_removes_views(); result != 0) {
    return result;
  }
  if (const int result = test_registered_child_view_can_be_embedded_as_placeholder_element(); result != 0) {
    return result;
  }
  if (const int result = test_event_route_carries_element_ancestry(); result != 0) {
    return result;
  }
  if (const int result = test_event_route_carries_child_view_ancestry(); result != 0) {
    return result;
  }
  return 0;
}
