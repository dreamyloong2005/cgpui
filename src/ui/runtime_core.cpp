#include "ui_internal.hpp"

namespace cgpui {

WindowRuntime::WindowRuntime(
    PlatformApplication& application,
    View& view,
    RendererFactory renderer_factory)
    : application_(application),
      view_(view),
      renderer_factory_(std::move(renderer_factory)) {
  view_registry_.insert_or_assign(
      root_view_id_.value,
      RegisteredView{.view = &view_});
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = root_window_runtime_id_,
      .descriptor = {},
      .root_view_id = root_view_id_,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = false,
      .owns_root_view = false,
      .active = false});
}

WindowRuntime::~WindowRuntime() {
  std::vector<std::jthread> workers;
  {
    std::lock_guard lock(tasks_mutex_);
    for (RuntimeTask& task : tasks_) {
      if (task.cancellation_requested != nullptr) {
        task.cancellation_requested->store(true);
      }
      if (!task.completed) {
        task.cancelled = true;
      }
      if (task.worker.joinable()) {
        task.worker.request_stop();
        workers.push_back(std::move(task.worker));
      }
    }
    task_completion_queue_.clear();
  }
}

int WindowRuntime::run(
    const WindowDescriptor& descriptor,
    WindowRuntimeOptions options) {
  should_quit_ = false;
  failed_ = false;
  window_ = nullptr;
  renderer_ = nullptr;
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->descriptor = descriptor;
    root_record->root_view_id = root_view_id_;
    root_record->window = nullptr;
    root_record->renderer = nullptr;
    root_record->active = false;
  }
  framebuffer_size_ = descriptor.size;
  viewport_size_ = descriptor.size;
  scale_ = {};
  input_ = {};
  pointer_capture_owner_.reset();
  keyboard_focus_owner_.reset();
  keyboard_focus_element_owner_.reset();
  hovered_element_id_.reset();
  cursor_shape_ = CursorShape::default_arrow;
  applied_cursor_shape_ = CursorShape::default_arrow;
  last_event_result_ = EventResult::unhandled();
  last_render_record_.reset();
  last_frame_statistics_.reset();
  last_event_dispatch_.reset();
  last_action_dispatch_.reset();
  current_event_route_.reset();
  invalidation_state_ = {};
  subscription_query_buffer_.clear();
  entity_count_ = 0;
  dispatching_view_event_ = false;
  firing_timers_ = false;
  draining_task_completions_ = false;
  handling_wakeup_ = false;
  update_batch_depth_ = 0;
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  event_dispatch_sequence_ = 0;
  render_sequence_ = 0;
  frame_index_ = 0;
  applied_ime_text_input_placement_.reset();
  last_platform_accessibility_update_.reset();
  platform_diagnostics_.clear();
  platform_diagnostic_sequence_ = 0;

  auto window_result = application_.create_window(
      descriptor,
      [this](const PlatformEvent& event) { handle_event(event); });
  if (!window_result) {
    if (error_callback_) {
      error_callback_(window_result.error());
    }
    return 1;
  }

  std::unique_ptr<PlatformWindow> window = std::move(*window_result);
  window_ = window.get();
  const WindowState window_state = window_->state();
  framebuffer_size_ = window_state.framebuffer_size;
  scale_ = window_state.scale;
  viewport_size_ = to_logical_pixels(framebuffer_size_, scale_);

  auto renderer_result = renderer_factory_(RenderSurfaceDescriptor{
      .native_surface = window_->native_surface(),
      .framebuffer_size = window_state.framebuffer_size,
      .scale = window_state.scale});
  if (!renderer_result || *renderer_result == nullptr) {
    if (error_callback_) {
      if (renderer_result) {
        error_callback_(Error{
            .code = ErrorCode::renderer_initialization_failed,
            .message = "Renderer factory returned an empty renderer"});
      } else {
        error_callback_(renderer_result.error());
      }
    }
    return 1;
  }
  renderer_ = *renderer_result;
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->window = window_;
    root_record->renderer = renderer_;
    root_record->active = true;
  }

  if (options.request_initial_redraw) {
    redraw_scheduled_ = true;
    window_->request_redraw();
  }

  const int run_result = application_.run();
  deactivate_native_additional_windows();
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->window = nullptr;
    root_record->renderer = nullptr;
    root_record->active = false;
  }
  window_ = nullptr;
  renderer_ = nullptr;

  if (failed_) {
    return 1;
  }
  return run_result;
}

AppOpenedWindow WindowRuntime::open_window(WindowOptions options) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = {}};
  app_opened_windows_.push_back(opened);
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = {},
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = true,
      .owns_root_view = false,
      .active = false});
  activate_native_window_for_record(window_runtime_records_.back());
  return opened;
}

AppOpenedWindow WindowRuntime::open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  const ViewId root_view_id = register_view(std::move(root_view));
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = root_view_id};
  app_opened_windows_.push_back(opened);
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = root_view_id,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = true,
      .owns_root_view = root_view_id.value != 0,
      .active = false});
  activate_native_window_for_record(window_runtime_records_.back());
  return opened;
}

std::span<const AppOpenedWindow> WindowRuntime::app_opened_windows() const {
  return app_opened_windows_;
}

WindowRuntimeId WindowRuntime::root_window_runtime_id() const {
  return root_window_runtime_id_;
}

std::span<const WindowRuntimeRecord> WindowRuntime::window_runtime_records()
    const {
  return window_runtime_records_;
}

const WindowRuntimeRecord* WindowRuntime::window_runtime_record(
    WindowRuntimeId runtime_id) const {
  return find_window_runtime_record(runtime_id);
}

const View* WindowRuntime::app_opened_window_root_view(
    ViewId root_view_id) const {
  return find_view(root_view_id);
}

View* WindowRuntime::root_view() {
  return find_view(root_view_id_);
}

const View* WindowRuntime::root_view() const {
  return find_view(root_view_id_);
}

ViewId WindowRuntime::register_view(View& view) {
  const ViewId view_id = allocate_view_id();
  removed_view_ids_.erase(view_id.value);
  view_registry_.insert_or_assign(
      view_id.value,
      RegisteredView{.view = &view});
  return view_id;
}

ViewId WindowRuntime::register_view(std::unique_ptr<View> view) {
  const ViewId view_id = allocate_view_id();
  removed_view_ids_.erase(view_id.value);
  if (view != nullptr) {
    View* view_ptr = view.get();
    view_registry_.insert_or_assign(
        view_id.value,
        RegisteredView{.view = view_ptr, .owned_view = std::move(view)});
  }
  return view_id;
}

View* WindowRuntime::find_view(ViewId view_id) {
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return nullptr;
  }
  return entry->second.view;
}

const View* WindowRuntime::find_view(ViewId view_id) const {
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return nullptr;
  }
  return entry->second.view;
}

bool WindowRuntime::remove_view(ViewId view_id) {
  if (view_id.value == 0 || view_id == root_view_id_) {
    return false;
  }
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return false;
  }
  view_registry_.erase(entry);
  removed_view_ids_.insert(view_id.value);
  return true;
}


void WindowRuntime::handle_resize(const WindowResized& event) {
  (void)resize_surface(event.size, event.scale);
}

void WindowRuntime::handle_redraw() {
  if (renderer_ == nullptr || should_quit_) {
    return;
  }

  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context();
  AnyElement rendered = view_.render(render_context);
  std::optional<ElementId> rendered_root_id;
  if (rendered != nullptr) {
    auto tree = std::make_unique<ElementTree>();
    rendered_root_id = tree->set_root(std::move(rendered));
    set_element_tree(std::move(tree));
  }

  last_render_record_ = RenderRecord{
      .sequence = ++render_sequence_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
      .root_element_id = rendered_root_id,
  };
  if (after_render_callback_) {
    after_render_callback_(context(), *last_render_record_);
  }

  if (owned_element_tree_ != nullptr) {
    (void)owned_element_tree_->layout_root(LayoutInput{
        .constraints =
            {
                .max_size = viewport_size_,
            },
        .scale = scale_,
    });
    frame_statistics.layout_pass_count += 1;
  }
  update_platform_accessibility_tree();
  apply_focused_text_ime_placement();

  auto result = render_view(
      *renderer_,
      view_,
      viewport_size_,
      scale_,
      &frame_statistics);
  if (!result) {
    fail_and_quit(result.error());
    return;
  }

  clear_invalidation();
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  frame_index_ += 1;
  frame_statistics.frame_index = frame_index_;
  if (last_render_record_.has_value()) {
    last_render_record_->statistics = frame_statistics;
  }
  last_frame_statistics_ = frame_statistics;
  if (after_frame_callback_) {
    after_frame_callback_(context());
  }
}

WindowRuntimeContext WindowRuntime::context_for_record(
    const WindowRuntimeRecord& record) {
  const WindowState state = record.window->state();
  const ViewInputState input = input_state();

  return WindowRuntimeContext{
      .runtime = *this,
      .application = application_,
      .window = *record.window,
      .renderer = *record.renderer,
      .window_runtime_id = record.runtime_id,
      .view_id = record.root_view_id,
      .viewport_size = to_logical_pixels(state.framebuffer_size, state.scale),
      .scale = state.scale,
      .input = input,
      .event_route = current_event_route_,
      .last_event_result = last_event_result_,
      .last_event_dispatch = last_event_dispatch_,
      .frame_index = frame_index_};
}

void WindowRuntime::handle_redraw_for_record(
    WindowRuntimeRecord& record,
    View& view) {
  if (!record.active || record.window == nullptr || record.renderer == nullptr ||
      should_quit_) {
    return;
  }

  const WindowState state = record.window->state();
  const Size viewport_size = to_logical_pixels(state.framebuffer_size, state.scale);
  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context_for_record(record);
  (void)view.render(render_context);

  last_render_record_ = RenderRecord{
      .sequence = ++render_sequence_,
      .view_id = record.root_view_id,
      .viewport_size = viewport_size,
  };
  if (after_render_callback_) {
    after_render_callback_(context_for_record(record), *last_render_record_);
  }

  auto result = render_view(
      *record.renderer,
      view,
      viewport_size,
      state.scale,
      &frame_statistics);
  if (!result) {
    fail_and_quit(result.error());
    return;
  }

  clear_invalidation();
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  frame_index_ += 1;
  frame_statistics.frame_index = frame_index_;
  if (last_render_record_.has_value()) {
    last_render_record_->statistics = frame_statistics;
  }
  last_frame_statistics_ = frame_statistics;
  if (after_frame_callback_) {
    after_frame_callback_(context_for_record(record));
  }
}

void WindowRuntime::fail_and_quit(Error error) {
  failed_ = true;
  should_quit_ = true;
  if (error_callback_) {
    error_callback_(error);
  }
  application_.quit();
}

void WindowRuntime::activate_native_window_for_record(
    WindowRuntimeRecord& record) {
  record.native_window_error.reset();
  auto window_result = application_.create_window(
      record.descriptor,
      [this, runtime_id = record.runtime_id](const PlatformEvent& event) {
        handle_native_additional_window_event(runtime_id, event);
      });
  if (!window_result) {
    record.window = nullptr;
    record.renderer = nullptr;
    record.active = false;
    record.native_window_error = window_result.error();
    return;
  }

  std::unique_ptr<PlatformWindow> window = std::move(*window_result);
  const WindowState window_state = window->state();
  auto renderer_result = renderer_factory_(RenderSurfaceDescriptor{
      .native_surface = window->native_surface(),
      .framebuffer_size = window_state.framebuffer_size,
      .scale = window_state.scale});
  if (!renderer_result || *renderer_result == nullptr) {
    record.window = nullptr;
    record.renderer = nullptr;
    record.active = false;
    record.native_window_error =
        renderer_result
            ? Error{
                  .code = ErrorCode::renderer_initialization_failed,
                  .message =
                      "Renderer factory returned an empty child renderer"}
            : renderer_result.error();
    return;
  }

  record.window = window.get();
  record.renderer = *renderer_result;
  record.active = true;
  native_additional_windows_.push_back(std::move(window));
}


void WindowRuntime::cleanup_closed_additional_window(
    WindowRuntimeRecord& record) {
  const ViewId root_view_id = record.root_view_id;
  const PlatformWindow* window = record.window;

  if (window != nullptr) {
    native_additional_windows_.erase(
        std::remove_if(
            native_additional_windows_.begin(),
            native_additional_windows_.end(),
            [window](const std::unique_ptr<PlatformWindow>& owned_window) {
              return owned_window.get() == window;
            }),
        native_additional_windows_.end());
  }

  if (record.owns_root_view && root_view_id.value != 0) {
    (void)remove_view(root_view_id);
    remove_subscriptions_for_view(root_view_id);
  }

  record.window = nullptr;
  record.renderer = nullptr;
  record.active = false;
  record.owns_window = false;
  record.owns_renderer = false;
  record.owns_root_view = false;
}

void WindowRuntime::remove_subscriptions_for_view(ViewId view_id) {
  if (view_id.value == 0) {
    return;
  }

  entity_subscriptions_.erase(
      std::remove_if(
          entity_subscriptions_.begin(),
          entity_subscriptions_.end(),
          [view_id](const EntitySubscription& subscription) {
            return subscription.view_id == view_id;
          }),
      entity_subscriptions_.end());
}

void WindowRuntime::deactivate_native_additional_windows() {
  for (WindowRuntimeRecord& record : window_runtime_records_) {
    if (record.runtime_id == root_window_runtime_id_) {
      continue;
    }
    record.window = nullptr;
    record.renderer = nullptr;
    record.active = false;
  }
  native_additional_windows_.clear();
}

WindowRuntimeId WindowRuntime::allocate_window_runtime_id() {
  const WindowRuntimeId runtime_id{next_window_runtime_id_};
  next_window_runtime_id_ += 1;
  return runtime_id;
}

WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(
    WindowRuntimeId runtime_id) {
  const auto record = std::find_if(
      window_runtime_records_.begin(),
      window_runtime_records_.end(),
      [runtime_id](const WindowRuntimeRecord& record) {
        return record.runtime_id == runtime_id;
      });
  return record == window_runtime_records_.end() ? nullptr : &*record;
}

const WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(
    WindowRuntimeId runtime_id) const {
  const auto record = std::find_if(
      window_runtime_records_.begin(),
      window_runtime_records_.end(),
      [runtime_id](const WindowRuntimeRecord& record) {
        return record.runtime_id == runtime_id;
      });
  return record == window_runtime_records_.end() ? nullptr : &*record;
}


void WindowRuntime::set_after_frame_callback(
    WindowRuntimeFrameCallback callback) {
  after_frame_callback_ = std::move(callback);
}

void WindowRuntime::set_after_render_callback(
    WindowRuntimeRenderCallback callback) {
  after_render_callback_ = std::move(callback);
}

void WindowRuntime::set_after_event_callback(
    WindowRuntimeEventCallback callback) {
  after_event_callback_ = std::move(callback);
}

void WindowRuntime::set_close_requested_callback(
    WindowRuntimeFrameCallback callback) {
  close_requested_callback_ = std::move(callback);
}

void WindowRuntime::set_error_callback(WindowRuntimeErrorCallback callback) {
  error_callback_ = std::move(callback);
}

void WindowRuntime::set_element_root(const Element* element) {
  owned_element_tree_.reset();
  element_root_ = element;
  apply_focused_text_ime_placement();
}

void WindowRuntime::set_element_tree(std::unique_ptr<ElementTree> tree) {
  owned_element_tree_ = std::move(tree);
  element_root_ = nullptr;
  apply_focused_text_ime_placement();
}

const ElementTree* WindowRuntime::element_tree() const {
  return owned_element_tree_.get();
}

const Element* WindowRuntime::element_root() const {
  if (owned_element_tree_ != nullptr) {
    return owned_element_tree_->get(owned_element_tree_->root_id());
  }
  return element_root_;
}

void WindowRuntime::capture_pointer(PointerCaptureOwner owner) {
  if (is_valid_pointer_capture_owner(owner)) {
    pointer_capture_owner_ = owner;
  }
}

void WindowRuntime::release_pointer(PointerCaptureOwner owner) {
  if (pointer_capture_owner_ == owner) {
    pointer_capture_owner_.reset();
  }
}


ViewId WindowRuntime::allocate_view_id() {
  const ViewId view_id{next_view_id_};
  next_view_id_ += 1;
  return view_id;
}

bool WindowRuntime::is_view_id_allocated(ViewId view_id) const {
  return view_id.value != 0 && view_id.value < next_view_id_ &&
         !removed_view_ids_.contains(view_id.value);
}

std::optional<ViewId> WindowRuntime::upgrade_view(WeakView view) const {
  if (view.empty() || !is_view_id_allocated(view.id())) {
    return std::nullopt;
  }
  return view.id();
}

bool WindowRuntime::notify_entity_changed(
    std::type_index entity_type,
    std::uint64_t entity_id_value) {
  bool notified = false;
  for (const EntitySubscription& subscription : entity_subscriptions_) {
    if (subscription.entity_type == entity_type &&
        subscription.entity_id_value == entity_id_value) {
      notified = true;
    }
  }
  for (const EntityObserver& observer : entity_observers_) {
    if (observer.entity_type == entity_type &&
        observer.entity_id_value == entity_id_value &&
        observer.callback) {
      notified = true;
      observer.callback(context(), entity_id_value);
    }
  }
  if (notified) {
    request_render();
  }
  return notified;
}

Result<void> WindowRuntime::resize_surface(Size size, DpiScale scale) {
  framebuffer_size_ = size;
  scale_ = scale;
  viewport_size_ = to_logical_pixels(framebuffer_size_, scale_);
  if (renderer_ == nullptr) {
    return {};
  }

  auto result = renderer_->resize(size, scale);
  if (!result) {
    fail_and_quit(result.error());
  }
  return result;
}


} // namespace cgpui
