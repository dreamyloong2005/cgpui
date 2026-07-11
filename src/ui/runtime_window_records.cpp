#include "ui_internal.hpp"

namespace cgpui {

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

WindowRuntimeContext WindowRuntime::context_for_record(
    const WindowRuntimeRecord& record) {
  return WindowRuntimeContext{
      .runtime = *this,
      .application = application_,
      .platform_window = *record.window,
      .renderer = *record.renderer,
      .window_runtime_id = record.runtime_id,
      .view_id = record.root_view_id,
      .viewport_size = record.viewport_size,
      .scale = record.scale,
      .input = record.input,
      .event_route = current_event_route_,
      .last_event_result = last_event_result_,
      .last_event_dispatch = last_event_dispatch_,
      .frame_index = frame_index_};
}

void WindowRuntime::cleanup_closed_additional_window(
    WindowRuntimeRecord& record) {
  const ViewId root_view_id = record.root_view_id;
  const PlatformWindow* window = record.window;

  if (window != nullptr) {
    const auto owned_window = std::find_if(
        native_additional_windows_.begin(),
        native_additional_windows_.end(),
        [window](const std::unique_ptr<PlatformWindow>& owned) {
          return owned.get() == window;
        });
    if (owned_window != native_additional_windows_.end()) {
      retired_native_windows_.push_back(std::move(*owned_window));
      native_additional_windows_.erase(owned_window);
    }
  }

  if (record.owns_root_view && root_view_id.value != 0) {
    (void)remove_view(root_view_id);
    remove_subscriptions_for_view(root_view_id);
    remove_observers_for_view(root_view_id);
  }

  record.window = nullptr;
  record.renderer = nullptr;
  record.active = false;
  record.owns_window = false;
  record.owns_renderer = false;
  record.owns_root_view = false;
  record.redraw_scheduled = false;
  record.input = {};
  request_platform_wakeup();
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
    record.redraw_scheduled = false;
    record.input = {};
  }
  native_additional_windows_.clear();
  retired_native_windows_.clear();
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

} // namespace cgpui
