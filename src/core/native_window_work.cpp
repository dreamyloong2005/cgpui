#include "native_window_work_registry_internal.hpp"

namespace cgpui {

bool native_window_work_post(NativeWindowWorkToken token) noexcept {
  if (!token) return false;
  auto& records = native_window_work_detail::registry();
  std::lock_guard lock(records.mutex);
  const auto endpoint = records.endpoints.find(token.instance);
  if (endpoint == records.endpoints.end()) return false;
  const auto found = endpoint->second->work.find(token.work);
  if (found == endpoint->second->work.end() || found->second.callback.expired()) return false;
  auto& work = found->second;
  if (work.pending) return true;
  // Serialize enqueue success with coalescing. A failed PostMessage must not let
  // another sender mistake an in-flight, ultimately failed enqueue for a wake.
  if (!endpoint->second->post(endpoint->second->window, token)) return false;
  work.pending = true;
  return true;
}

NativeWindowWorkDispatch native_window_work_dispatch(
    void* window, NativeWindowWorkToken token) noexcept {
  std::weak_ptr<NativeWindowWork> weak;
  auto& records = native_window_work_detail::registry();
  {
    std::lock_guard lock(records.mutex);
    const auto endpoint = records.endpoints.find(token.instance);
    if (endpoint == records.endpoints.end() || endpoint->second->window != window ||
        endpoint->second->owner != std::this_thread::get_id()) return NativeWindowWorkDispatch::ignored;
    const auto found = endpoint->second->work.find(token.work);
    if (found == endpoint->second->work.end() || !found->second.pending)
      return NativeWindowWorkDispatch::ignored;
    auto& work = found->second;
    work.pending = false;
    if (work.running) {
      // A nested message pump consumed an explicitly posted wake. Preserve it,
      // without reentering the callback or creating a retry from a busy result.
      work.explicit_repost = true;
      return NativeWindowWorkDispatch::deferred;
    }
    work.running = true;
    weak = work.callback;
  }
  const auto callback = weak.lock(); // Promotion and last-owner destruction stay outside the mutex.
  const auto result = callback ? callback->run() : NativeWindowWorkResult::complete;
  bool repost = false;
  {
    std::lock_guard lock(records.mutex);
    const auto endpoint = records.endpoints.find(token.instance);
    if (endpoint != records.endpoints.end()) {
      const auto found = endpoint->second->work.find(token.work);
      if (found != endpoint->second->work.end()) {
        auto& work = found->second;
        work.running = false;
        repost = work.explicit_repost && !work.pending;
        work.explicit_repost = false;
      }
    }
  }
  if (repost) (void)native_window_work_post(token);
  if (!callback) return NativeWindowWorkDispatch::ignored;
  return result == NativeWindowWorkResult::complete
      ? NativeWindowWorkDispatch::complete : NativeWindowWorkDispatch::deferred;
}
} // namespace cgpui
