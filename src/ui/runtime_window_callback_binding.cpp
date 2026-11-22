#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

WindowRuntimeId WindowRuntime::normalize_window_context_id(WindowRuntimeId id) const {
  // Older aggregate contexts omit the origin. A real (including stale) child
  // id must never fall back to the root.
  return id.value == 0 ? state_->windows.root_window_runtime_id_ : id;
}

void WindowRuntime::invoke_window_callback(WindowRuntimeId id, const DeferredCallback& callback) {
  if (!callback) return;
  id = normalize_window_context_id(id);
  const RuntimeWindowCallbackScope callback_scope(*this);
  const ActionWindowScope action_window{state_->actions.action_window_runtime_id_, id};
  if (id == state_->windows.root_window_runtime_id_) {
    callback(context());
    return;
  }
  const auto* record = find_window_runtime_record(id);
  if (record != nullptr && record->active && record->window != nullptr && record->renderer != nullptr) {
    callback(context_for_record(*record));
  }
}

DeferredCallback WindowRuntime::bind_window_callback(WindowRuntimeId id, DeferredCallback callback) {
  if (!callback) return {};
  return [id, callback = std::move(callback)](const WindowRuntimeContext& context) {
    context.runtime.invoke_window_callback(id, callback);
  };
}

AsyncIoCompletionCallback WindowRuntime::bind_window_io_callback(
    WindowRuntimeId id, AsyncIoCompletionCallback callback) {
  if (!callback) return {};
  return [id, callback = std::move(callback)](const WindowRuntimeContext& context, const AsyncIoResult& result) {
    context.runtime.invoke_window_callback(id, [&](const auto& current) { callback(current, result); });
  };
}

AnimationCallback WindowRuntime::bind_window_animation_callback(WindowRuntimeId id, AnimationCallback callback) {
  if (!callback) return {};
  return [id, callback = std::move(callback)](const WindowRuntimeContext& context, const AnimationSnapshot& snapshot) {
    context.runtime.invoke_window_callback(id, [&](const auto& current) { callback(current, snapshot); });
  };
}

} // namespace cgpui
