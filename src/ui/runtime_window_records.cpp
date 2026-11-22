#include "ui_internal.hpp"
#include "runtime_window_store_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

WindowRuntimeId WindowRuntime::root_window_runtime_id() const {
  return state_->windows.root_window_runtime_id_;
}

std::span<const WindowRuntimeRecord> WindowRuntime::window_runtime_records()
    const {
  return state_->windows.window_runtime_records_->snapshot();
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
      .application = state_->core.application_,
      .platform_window = *record.window,
      .renderer = *record.renderer,
      .window_runtime_id = record.runtime_id,
      .view_id = record.root_view_id,
      .viewport_size = record.viewport_size,
      .scale = record.scale,
      .input = record.input,
      .event_route = record.event_route,
      .last_event_result = record.last_event_result,
      .last_event_dispatch = record.last_event_dispatch,
      .frame_index = state_->diagnostics.frame_index_};
}

void WindowRuntime::cleanup_closed_additional_window(
    WindowRuntimeRecord& record) {
  const RuntimeWindowCallbackScope cleanup_scope(*this);
  const ViewId root_view_id = record.root_view_id;
  record.active = false;
  cancel_window_timers(record.runtime_id);
  if (auto host = state_->elements.window_element_hosts_.extract(record.runtime_id.value); !host.empty()) {
    state_->windows.window_lifetime_->retire_tree(std::move(host.mapped().tree));
  }
  const PlatformWindow* window = record.window;

  if (window != nullptr) {
    const auto owned_window = std::find_if(
        state_->windows.native_additional_windows_.begin(),
        state_->windows.native_additional_windows_.end(),
        [window](const std::unique_ptr<PlatformWindow>& owned) {
          return owned.get() == window;
        });
    if (owned_window != state_->windows.native_additional_windows_.end()) {
      state_->windows.retired_native_windows_.push_back(std::move(*owned_window));
      state_->windows.native_additional_windows_.erase(owned_window);
    }
  }

  if (record.owns_root_view && root_view_id.value != 0) {
    retire_window_view(root_view_id);
    remove_subscriptions_for_view(root_view_id);
    remove_observers_for_view(root_view_id);
  }

  state_->windows.window_lifetime_->retire(record.renderer);
  record.window = nullptr;
  record.renderer = nullptr;
  record.active = false;
  record.owns_window = false;
  record.owns_renderer = false;
  record.owns_root_view = false;
  record.redraw_scheduled = false;
  record.input = {};
  reset_event_state_for_record(record);
  reset_accessibility_state_for_record(record);
  request_platform_wakeup();
}

void WindowRuntime::remove_subscriptions_for_view(ViewId view_id) {
  if (view_id.value == 0) {
    return;
  }

  state_->entities.entity_subscriptions_.erase(
      std::remove_if(
          state_->entities.entity_subscriptions_.begin(),
          state_->entities.entity_subscriptions_.end(),
          [view_id](const EntitySubscription& subscription) {
            return subscription.view_id == view_id;
          }),
      state_->entities.entity_subscriptions_.end());
}

WindowRuntimeId WindowRuntime::allocate_window_runtime_id() {
  const WindowRuntimeId runtime_id{state_->windows.next_window_runtime_id_};
  state_->windows.next_window_runtime_id_ += 1;
  return runtime_id;
}

WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(
    WindowRuntimeId runtime_id) {
  return state_->windows.window_runtime_records_->find(runtime_id);
}

const WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(
    WindowRuntimeId runtime_id) const {
  return state_->windows.window_runtime_records_->find(runtime_id);
}

} // namespace cgpui
