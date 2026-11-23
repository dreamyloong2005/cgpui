#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

void WindowRuntime::reset_root_window_resources() {
  state_->scheduling.frame_readiness_.clear(*this, state_->windows.root_window_runtime_id_);
  Renderer* renderer = state_->windows.renderer_;
  state_->windows.window_ = nullptr;
  state_->windows.renderer_ = nullptr;
  if (auto* record = find_window_runtime_record(state_->windows.root_window_runtime_id_)) {
    record->window = nullptr;
    record->renderer = nullptr;
    record->active = false;
    record->owns_renderer = false;
  }
  state_->windows.window_lifetime_->release(renderer);
}

void WindowRuntime::retire_window_view(ViewId view_id) {
  const auto view = state_->views.view_registry_.find(view_id.value);
  if (view != state_->views.view_registry_.end()) {
    state_->windows.window_lifetime_->retire_view(std::move(view->second.owned_view));
  }
  (void)remove_view(view_id);
}

} // namespace cgpui
