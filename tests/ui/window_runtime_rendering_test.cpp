#include "window_runtime_test_support.hpp"

namespace {
int test_redraw_paints_initial_viewport() {
  RuntimeFixture fixture;
  fixture.app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor) {
        if (!equal(descriptor.framebuffer_size, cgpui::Size{640.0F, 480.0F})) {
          return cgpui::Result<cgpui::Renderer*>{std::unexpected(cgpui::Error{
              .code = cgpui::ErrorCode::renderer_initialization_failed,
              .message = "wrong descriptor size"})};
        }
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .title = "Runtime Test",
      .size = {640.0F, 480.0F}});

  if (result != 0) {
    return 1;
  }
  if (fixture.app.create_window_count != 1 || fixture.app.run_count != 1) {
    return 2;
  }
  if (fixture.window.request_redraw_count != 1) {
    return 3;
  }
  if (fixture.renderer.begin_frame_count != 1 ||
      fixture.view.paint_count != 1 ||
      fixture.frame.present_count != 1) {
    return 4;
  }
  if (fixture.frame.draw_count != 1) {
    return 6;
  }
  if (!equal(fixture.view.last_viewport_size, cgpui::Size{640.0F, 480.0F})) {
    return 5;
  }

  return 0;
}

int test_view_render_hook_defaults_empty_and_can_be_overridden() {
  RuntimeFixture fixture;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  cgpui::WindowRuntimeContext context{
      .runtime = runtime,
      .application = fixture.app,
      .platform_window = fixture.window,
      .renderer = fixture.renderer,
      .view_id = cgpui::ViewId{9},
      .viewport_size = cgpui::Size{123.0F, 45.0F},
      .scale = cgpui::DpiScale{1.0F},
      .input = {},
      .event_route = {},
      .last_event_result = {},
      .last_event_dispatch = {},
      .frame_index = 3};

  cgpui::AnyElement default_rendered = fixture.view.render(context);
  if (default_rendered != nullptr) {
    return 300;
  }

  RenderHookView render_hook_view;
  cgpui::AnyElement rendered = render_hook_view.render(context);
  const auto* styled = dynamic_cast<const cgpui::StyledElement*>(rendered.get());
  if (render_hook_view.render_count != 1 || render_hook_view.paint_count != 0) {
    return 301;
  }
  if (!render_hook_view.saw_context_view_id ||
      !render_hook_view.saw_context_viewport_size) {
    return 302;
  }
  if (styled == nullptr ||
      !equal(styled->style().preferred_size, cgpui::Size{10.0F, 20.0F})) {
    return 303;
  }
  if (!runtime.invalidation_state().paint) {
    return 304;
  }

  return 0;
}

RuntimeFixture* runtime_render_pass_fixture = nullptr;

void dispatch_runtime_render_pass_sequence() {
  runtime_render_pass_fixture->window.request_redraw();
  auto& callback = runtime_render_pass_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_runtime_render_pass_installs_rendered_element_tree() {
  RuntimeFixture fixture;
  RuntimeRenderView view;
  runtime_render_pass_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_render_pass_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  std::optional<cgpui::ElementId> routed_element_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::pointer_moved) {
          routed_element_id = record.route.target_element_id;
        }
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  runtime_render_pass_fixture = nullptr;

  if (result != 0) {
    return 305;
  }
  if (view.render_count != 2 || view.paint_count != 2) {
    return 306;
  }
  if (!view.saw_context_view_id || !view.saw_context_viewport_size ||
      !equal(view.last_paint_viewport_size, cgpui::Size{640.0F, 480.0F})) {
    return 307;
  }
  if (runtime.element_tree() == nullptr || runtime.element_root() == nullptr) {
    return 308;
  }
  const cgpui::ElementId root_id = runtime.element_tree()->root_id();
  if (root_id.value == 0 || runtime.element_root()->id() != root_id) {
    return 309;
  }
  const std::optional<cgpui::Rect> bounds =
      runtime.element_root()->layout_bounds();
  if (!bounds.has_value() || bounds->size.width != 640.0F ||
      bounds->size.height != 480.0F) {
    return 310;
  }
  if (!routed_element_id.has_value() || *routed_element_id != root_id) {
    return 311;
  }

  return 0;
}

RuntimeFixture* render_invalidation_fixture = nullptr;

void dispatch_render_invalidation_sequence() {
  auto& callback = render_invalidation_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 82,
      .action = cgpui::KeyAction::pressed});
}

int test_view_context_requests_render_invalidation() {
  RuntimeFixture fixture;
  RenderInvalidationView view;
  render_invalidation_fixture = &fixture;
  fixture.app.on_run = &dispatch_render_invalidation_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::InvalidationState after_event_invalidation{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        after_event_invalidation = context.invalidation_state();
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  render_invalidation_fixture = nullptr;

  if (result != 0) {
    return 312;
  }
  if (view.event_count != 1 || view.paint_count != 1) {
    return 313;
  }
  if (view.before_request.render || view.before_request.layout ||
      view.before_request.paint) {
    return 314;
  }
  if (!view.after_request.render || !view.after_request.layout ||
      !view.after_request.paint) {
    return 315;
  }
  if (!view.after_runtime_request.render ||
      !view.after_runtime_request.layout ||
      !view.after_runtime_request.paint) {
    return 316;
  }
  if (!after_event_invalidation.render || !after_event_invalidation.layout ||
      !after_event_invalidation.paint) {
    return 317;
  }
  const cgpui::InvalidationState final_invalidation =
      runtime.invalidation_state();
  if (final_invalidation.render || final_invalidation.layout ||
      final_invalidation.paint) {
    return 318;
  }
  if (fixture.window.request_redraw_count != 1) {
    return 319;
  }

  return 0;
}

int test_runtime_reports_render_records_after_render() {
  RuntimeFixture fixture;
  RuntimeRenderView view;
  fixture.app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  int callback_count = 0;
  std::optional<cgpui::RenderRecord> callback_record;
  cgpui::ViewId callback_view_id{};
  int callback_frame_index = -1;
  runtime.set_after_render_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::RenderRecord& record) {
        callback_count += 1;
        callback_record = record;
        callback_view_id = context.view_id;
        callback_frame_index = context.frame_index;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});

  if (result != 0) {
    return 320;
  }
  if (view.render_count != 1 || view.paint_count != 1) {
    return 321;
  }
  if (callback_count != 1 || !callback_record.has_value()) {
    return 322;
  }
  if (callback_record->sequence != 1 ||
      callback_record->view_id != cgpui::ViewId{1} ||
      !equal(callback_record->viewport_size, cgpui::Size{640.0F, 480.0F})) {
    return 323;
  }
  if (runtime.element_tree() == nullptr ||
      !callback_record->root_element_id.has_value() ||
      *callback_record->root_element_id != runtime.element_tree()->root_id()) {
    return 324;
  }
  const std::optional<cgpui::RenderRecord> last_record =
      runtime.last_render_record();
  if (!last_record.has_value() ||
      last_record->sequence != callback_record->sequence ||
      last_record->root_element_id != callback_record->root_element_id) {
    return 325;
  }
  if (callback_view_id != cgpui::ViewId{1} || callback_frame_index != 0) {
    return 326;
  }

  return 0;
}

RuntimeFixture* resize_fixture = nullptr;

void dispatch_runtime_resize() {
  resize_fixture->window.dispatch_resize(
      cgpui::Size{320.0F, 240.0F},
      cgpui::DpiScale{2.0F});
  resize_fixture->window.request_redraw();
}

int test_resize_updates_renderer_and_viewport() {
  RuntimeFixture fixture;
  resize_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_resize;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .title = "Runtime Test",
      .size = {640.0F, 480.0F}});
  resize_fixture = nullptr;

  if (result != 0) {
    return 10;
  }
  if (fixture.renderer.resize_count != 1) {
    return 11;
  }
  if (!equal(fixture.renderer.last_resize_size, cgpui::Size{320.0F, 240.0F}) ||
      !equal(fixture.renderer.last_resize_scale, cgpui::DpiScale{2.0F})) {
    return 12;
  }
  if (!equal(fixture.view.last_viewport_size, cgpui::Size{160.0F, 120.0F})) {
    return 13;
  }
  if (fixture.renderer.begin_frame_count != 2 || fixture.view.paint_count != 2) {
    return 14;
  }

  return 0;
}

RuntimeFixture* hidpi_scale_fixture = nullptr;

void dispatch_hidpi_scale_sequence() {
  hidpi_scale_fixture->window.dispatch_resize(
      cgpui::Size{320.0F, 240.0F},
      cgpui::DpiScale{2.0F});
  hidpi_scale_fixture->window.request_redraw();
}

int test_hidpi_scale_flows_to_layout_text_and_renderer_resize() {
  RuntimeFixture fixture;
  HidpiRuntimeView view;
  hidpi_scale_fixture = &fixture;
  fixture.app.on_run = &dispatch_hidpi_scale_sequence;

  cgpui::DpiScale descriptor_scale{};
  cgpui::Size descriptor_framebuffer_size{};
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor) {
        descriptor_scale = descriptor.scale;
        descriptor_framebuffer_size = descriptor.framebuffer_size;
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .title = "HiDPI Runtime Test",
      .size = {640.0F, 480.0F}});
  hidpi_scale_fixture = nullptr;

  if (result != 0) {
    return 402;
  }
  if (!equal(descriptor_framebuffer_size, cgpui::Size{640.0F, 480.0F}) ||
      !equal(descriptor_scale, cgpui::DpiScale{1.0F})) {
    return 403;
  }
  if (fixture.renderer.resize_count != 1 ||
      !equal(fixture.renderer.last_resize_size, cgpui::Size{320.0F, 240.0F}) ||
      !equal(fixture.renderer.last_resize_scale, cgpui::DpiScale{2.0F})) {
    return 404;
  }
  if (view.render_count != 2 || view.paint_count != 2) {
    return 405;
  }
  if (!equal(view.render_viewport_size, cgpui::Size{160.0F, 120.0F}) ||
      !equal(view.paint_viewport_size, cgpui::Size{160.0F, 120.0F}) ||
      !equal(view.render_scale, cgpui::DpiScale{2.0F})) {
    return 406;
  }
  const cgpui::Element* root = runtime.element_root();
  if (root == nullptr || !root->layout_bounds().has_value()) {
    return 407;
  }
  const cgpui::Rect root_bounds = *root->layout_bounds();
  if (root_bounds.size.width != 20.0F ||
      root_bounds.size.height != 20.0F) {
    return 408;
  }
  if (fixture.frame.text_draw_count != 2 ||
      !equal(fixture.frame.last_text.scale, cgpui::DpiScale{2.0F}) ||
      fixture.frame.last_text.font_size != 20.0F ||
      fixture.frame.last_text.device_font_size != 40.0F ||
      fixture.frame.last_text.glyphs.size() != 2 ||
      fixture.frame.last_text.glyphs[0].advance != 10.0F ||
      fixture.frame.last_text.glyphs[0].device_advance != 20.0F ||
      fixture.frame.last_text.glyphs[0].key.scale != 2.0F ||
      fixture.frame.last_text.glyphs[0].key.device_font_size != 40.0F) {
    return 409;
  }

  return 0;
}

RuntimeFixture* close_fixture = nullptr;

void dispatch_runtime_close() {
  close_fixture->window.request_close();
}

int test_close_request_quits_application() {
  RuntimeFixture fixture;
  close_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_close;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  close_fixture = nullptr;

  if (result != 0) {
    return 20;
  }
  if (fixture.app.quit_count != 1) {
    return 21;
  }

  return 0;
}

RuntimeFixture* lifecycle_fixture = nullptr;

void dispatch_window_lifecycle_events() {
  auto& callback = lifecycle_fixture->window.callback;
  callback(cgpui::WindowActivated{.active = true});
  callback(cgpui::WindowFocused{.focused = true});
  callback(cgpui::WindowMoved{.position = {12.0F, 18.0F}});
  callback(cgpui::WindowMinimized{.minimized = true});
  callback(cgpui::WindowRestored{});
  callback(cgpui::WindowCloseRequested{});
}

int test_window_lifecycle_events_update_dispatch_records() {
  RuntimeFixture fixture;
  lifecycle_fixture = &fixture;
  fixture.app.on_run = &dispatch_window_lifecycle_events;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  std::vector<cgpui::EventDispatchRecord> records;
  std::vector<cgpui::ViewInputState> input_snapshots;
  bool close_callback_called = false;
  bool close_callback_saw_last_dispatch = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        records.push_back(record);
        input_snapshots.push_back(context.input);
      });
  runtime.set_close_requested_callback(
      [&](const cgpui::WindowRuntimeContext& context) {
        close_callback_called = true;
        close_callback_saw_last_dispatch =
            context.last_event_dispatch.has_value() &&
            context.last_event_dispatch->event_kind ==
                cgpui::EventKind::window_close_requested;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{},
                                 {.request_initial_redraw = false});
  lifecycle_fixture = nullptr;

  if (result != 0) {
    return 203;
  }
  if (fixture.app.quit_count != 1 || !close_callback_called ||
      !close_callback_saw_last_dispatch) {
    return 204;
  }
  if (records.size() != 6 || input_snapshots.size() != records.size()) {
    return 205;
  }

  const std::vector<cgpui::EventKind> expected_kinds{
      cgpui::EventKind::window_activated,
      cgpui::EventKind::window_focused,
      cgpui::EventKind::window_moved,
      cgpui::EventKind::window_minimized,
      cgpui::EventKind::window_restored,
      cgpui::EventKind::window_close_requested};
  for (std::size_t index = 0; index < expected_kinds.size(); ++index) {
    const cgpui::EventDispatchRecord& record = records[index];
    if (record.sequence != static_cast<int>(index + 1) ||
        record.view_id != cgpui::ViewId{1} ||
        record.event_kind != expected_kinds[index] ||
        record.route.target_view_id != cgpui::ViewId{1} ||
        record.route.event_kind != expected_kinds[index] ||
        record.result.consumed || record.result.cancelled) {
      return 206;
    }
  }
  if (!input_snapshots[0].focused || !input_snapshots[1].focused ||
      !input_snapshots[2].focused || input_snapshots[3].focused ||
      input_snapshots[4].focused || input_snapshots[5].focused) {
    return 207;
  }
  if (fixture.view.event_count != 1 || fixture.view.focus_count != 1 ||
      !fixture.view.focus_event_saw_focused) {
    return 208;
  }
  if (records.back().event_kind != cgpui::EventKind::window_close_requested) {
    return 209;
  }

  return 0;
}

RuntimeFixture* render_failure_fixture = nullptr;

void dispatch_render_failure() {
  render_failure_fixture->renderer.fail_begin_frame = true;
  render_failure_fixture->window.request_redraw();
}

int test_render_failure_quits_and_returns_failure() {
  RuntimeFixture fixture;
  render_failure_fixture = &fixture;
  fixture.app.on_run = &dispatch_render_failure;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  render_failure_fixture = nullptr;

  if (result == 0) {
    return 30;
  }
  if (fixture.app.quit_count != 1) {
    return 31;
  }

  return 0;
}

RuntimeFixture* event_dispatch_fixture = nullptr;

RuntimeFixture* owned_element_tree_fixture = nullptr;

void dispatch_owned_element_tree_sequence() {
  auto& callback = owned_element_tree_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_runtime_owns_installed_element_tree() {
  RuntimeFixture fixture;
  owned_element_tree_fixture = &fixture;
  fixture.app.on_run = &dispatch_owned_element_tree_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 40.0F, .height = 20.0F}));
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  if (tree != nullptr || runtime.element_tree() == nullptr ||
      runtime.element_root() == nullptr) {
    return 200;
  }
  if (runtime.element_tree()->root_id() != root_id ||
      runtime.element_root()->id() != root_id) {
    return 201;
  }

  std::optional<cgpui::ElementId> routed_element_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        routed_element_id = record.route.target_element_id;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  owned_element_tree_fixture = nullptr;

  if (result != 0) {
    return 202;
  }
  if (!routed_element_id.has_value() || *routed_element_id != root_id) {
    return 203;
  }

  runtime.set_element_tree(nullptr);
  return runtime.element_tree() == nullptr && runtime.element_root() == nullptr
      ? 0
      : 204;
}

RuntimeFixture* runtime_element_state_fixture = nullptr;

void dispatch_runtime_element_state_sequence() {
  auto& callback = runtime_element_state_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_exposes_element_state_for_installed_tree() {
  RuntimeFixture fixture;
  runtime_element_state_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_element_state_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 40.0F, .height = 20.0F}));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  bool missing_before_init = false;
  bool context_initialized = false;
  bool runtime_read_after_context_mutation = false;
  bool wrong_type_soft_failed = false;
  bool missing_element_soft_failed = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        missing_before_init =
            context.element_state<RuntimeElementState>(root_id) == nullptr;
        RuntimeElementState* state =
            context.element_state_or_init<RuntimeElementState>(root_id, 11);
        context_initialized = state != nullptr && state->value == 11;
        if (state != nullptr) {
          state->value = 17;
        }
        RuntimeElementState* runtime_state =
            context.runtime.element_state<RuntimeElementState>(root_id);
        runtime_read_after_context_mutation =
            runtime_state != nullptr && runtime_state->value == 17;
        wrong_type_soft_failed =
            context.element_state<RuntimeEntity>(root_id) == nullptr;
        missing_element_soft_failed =
            context.element_state_or_init<RuntimeElementState>(
                cgpui::ElementId{root_id.value + 1000},
                5) == nullptr;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  runtime_element_state_fixture = nullptr;

  if (result != 0) {
    return 301;
  }
  const RuntimeElementState* retained_state =
      runtime.element_state<RuntimeElementState>(root_id);
  if (!missing_before_init || !context_initialized ||
      !runtime_read_after_context_mutation || !wrong_type_soft_failed ||
      !missing_element_soft_failed) {
    return 302;
  }
  return retained_state != nullptr && retained_state->value == 17 ? 0 : 303;
}

RuntimeFixture* runtime_layout_owned_tree_fixture = nullptr;

void dispatch_runtime_layout_owned_tree_sequence() {
  runtime_layout_owned_tree_fixture->window.request_redraw();
  auto& callback = runtime_layout_owned_tree_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_runtime_lays_out_owned_element_tree_on_redraw() {
  RuntimeFixture fixture;
  runtime_layout_owned_tree_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_layout_owned_tree_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 700.0F, .height = 600.0F}));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> routed_element_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        routed_element_id = record.route.target_element_id;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  runtime_layout_owned_tree_fixture = nullptr;

  if (result != 0) {
    return 205;
  }
  if (fixture.renderer.begin_frame_count != 2 ||
      runtime.element_root() == nullptr ||
      !runtime.element_root()->layout_bounds().has_value()) {
    return 206;
  }
  const cgpui::Rect bounds = *runtime.element_root()->layout_bounds();
  if (bounds.size.width != 640.0F || bounds.size.height != 480.0F) {
    return 207;
  }
  if (!routed_element_id.has_value() || *routed_element_id != root_id) {
    return 208;
  }

  return 0;
}


class FrameStatisticsView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    render_count += 1;
    return cgpui::into_element(cgpui::div().size(20.0F, 12.0F));
  }

  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport_size) override {
    paint_count += 1;
    last_viewport_size = viewport_size;
    paint_list.fill_rect(
        cgpui::Rect{.origin = {1.0F, 2.0F}, .size = {3.0F, 4.0F}},
        cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 1.0F});
    paint_list.fill_text(
        cgpui::Rect{.origin = {5.0F, 6.0F}, .size = {24.0F, 16.0F}},
        cgpui::Color{.r = 0.8F, .g = 0.9F, .b = 1.0F, .a = 1.0F},
        "abc");
    paint_list.fill_text_selection(
        cgpui::Rect{.origin = {5.0F, 6.0F}, .size = {8.0F, 16.0F}},
        cgpui::Color{.r = 0.1F, .g = 0.3F, .b = 0.7F, .a = 1.0F},
        cgpui::TextSelectionRange{.start = 0, .end = 1, .collapsed = false});
    paint_list.fill_text_caret(
        cgpui::Rect{.origin = {13.0F, 6.0F}, .size = {1.0F, 16.0F}},
        cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
        2,
        16.0F);
  }

  int render_count = 0;
  int paint_count = 0;
  cgpui::Size last_viewport_size{};
};

int test_frame_statistics_report_render_layout_paint_and_command_counts() {
  FakeWindow window{cgpui::WindowState{
      .framebuffer_size = {640.0F, 480.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false}};
  FakeApplication app{window};
  RecordingFrame frame;
  RecordingRenderer renderer{frame};
  FrameStatisticsView view;
  app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  if (result != 0) {
    return 394;
  }

  const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
      runtime.diagnostics_snapshot();
  if (!diagnostics.last_frame_statistics.has_value()) {
    return 395;
  }
  const cgpui::FrameStatistics& statistics =
      *diagnostics.last_frame_statistics;
  if (statistics.frame_index != 1 || statistics.render_pass_count != 1 ||
      statistics.layout_pass_count != 1 || statistics.paint_pass_count != 1) {
    return 396;
  }
  if (statistics.paint_command_count != 4 ||
      statistics.submitted_command_count != 4 ||
      statistics.skipped_command_count != 0 ||
      statistics.solid_rect_command_count != 1 ||
      statistics.text_command_count != 1 ||
      statistics.text_selection_command_count != 1 ||
      statistics.text_caret_command_count != 1) {
    return 397;
  }
  if (statistics.begin_frame_count != 1 || statistics.clear_count != 1 ||
      statistics.present_count != 1) {
    return 398;
  }
  if (statistics.frame_time_ms != 0.0 || statistics.render_time_ms != 0.0 ||
      statistics.layout_time_ms != 0.0 || statistics.paint_time_ms != 0.0) {
    return 399;
  }
  if (!diagnostics.last_render_record.has_value() ||
      !diagnostics.last_render_record->statistics.has_value() ||
      diagnostics.last_render_record->statistics->paint_command_count != 4) {
    return 400;
  }
  if (view.render_count != 1 || view.paint_count != 1 ||
      frame.draw_count != 1 || frame.text_draw_count != 1 ||
      frame.text_selection_draw_count != 1 ||
      frame.text_caret_draw_count != 1 ||
      frame.clear_count != 1 || frame.present_count != 1) {
    return 401;
  }

  return 0;
}

RuntimeFixture* key_binding_fixture = nullptr;

} // namespace

int main() {
  if (const int result = test_redraw_paints_initial_viewport(); result != 0) {
    return result;
  }
  if (const int result = test_view_render_hook_defaults_empty_and_can_be_overridden(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_render_pass_installs_rendered_element_tree(); result != 0) {
    return result;
  }
  if (const int result = test_view_context_requests_render_invalidation(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_reports_render_records_after_render(); result != 0) {
    return result;
  }
  if (const int result = test_resize_updates_renderer_and_viewport(); result != 0) {
    return result;
  }
  if (const int result = test_hidpi_scale_flows_to_layout_text_and_renderer_resize(); result != 0) {
    return result;
  }
  if (const int result = test_close_request_quits_application(); result != 0) {
    return result;
  }
  if (const int result = test_window_lifecycle_events_update_dispatch_records(); result != 0) {
    return result;
  }
  if (const int result = test_render_failure_quits_and_returns_failure(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_owns_installed_element_tree(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_exposes_element_state_for_installed_tree(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_lays_out_owned_element_tree_on_redraw(); result != 0) {
    return result;
  }
  if (const int result = test_frame_statistics_report_render_layout_paint_and_command_counts(); result != 0) {
    return result;
  }
  return 0;
}
