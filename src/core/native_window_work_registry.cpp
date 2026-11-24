#include "native_window_work_registry_internal.hpp"
#include <limits>
#include <utility>

namespace cgpui::native_window_work_detail {
Registry& registry() {
  static Registry value;
  return value;
}
std::uintptr_t next_identity(Registry& records) noexcept {
  if (records.next == std::numeric_limits<std::uintptr_t>::max()) return 0;
  return ++records.next; // Never reuse an exhausted identity, including HWND reuse.
}
} // namespace cgpui::native_window_work_detail

namespace cgpui {
NativeWindowWork::~NativeWindowWork() = default;

std::uintptr_t native_window_work_open(void* window, NativeWindowWorkPost post) noexcept {
  if (!window || !post) return 0;
  try {
    auto endpoint = std::make_shared<native_window_work_detail::Endpoint>();
    endpoint->window = window;
    endpoint->owner = std::this_thread::get_id();
    endpoint->post = post;
    auto& records = native_window_work_detail::registry();
    std::lock_guard lock(records.mutex);
    for (const auto& [identity, existing] : records.endpoints)
      if (existing->window == window) return 0;
    const auto identity = native_window_work_detail::next_identity(records);
    if (!identity) return 0;
    records.endpoints.emplace(identity, std::move(endpoint));
    return identity;
  } catch (...) { return 0; }
}

void native_window_work_close(void* window, std::uintptr_t instance) noexcept {
  std::shared_ptr<native_window_work_detail::Endpoint> removed;
  auto& records = native_window_work_detail::registry();
  {
    std::lock_guard lock(records.mutex);
    const auto found = records.endpoints.find(instance);
    if (found == records.endpoints.end() || found->second->window != window ||
        found->second->owner != std::this_thread::get_id()) return;
    removed = std::move(found->second);
    records.endpoints.erase(found);
  } // Weak callbacks/control blocks are destroyed outside the registry mutex.
}

NativeWindowWorkToken native_window_work_register(
    void* window, std::weak_ptr<NativeWindowWork> callback) noexcept {
  if (!window || callback.expired()) return {};
  try {
    auto& records = native_window_work_detail::registry();
    std::lock_guard lock(records.mutex);
    for (const auto& [instance, endpoint] : records.endpoints) {
      if (endpoint->window != window) continue;
      const auto work = native_window_work_detail::next_identity(records);
      if (!work) return {};
      endpoint->work.emplace(work, native_window_work_detail::Work{.callback = std::move(callback)});
      return {instance, work};
    }
  } catch (...) {}
  return {};
}

void native_window_work_cancel(NativeWindowWorkToken token) noexcept {
  std::weak_ptr<NativeWindowWork> removed;
  auto& records = native_window_work_detail::registry();
  {
    std::lock_guard lock(records.mutex);
    const auto endpoint = records.endpoints.find(token.instance);
    if (endpoint == records.endpoints.end()) return;
    const auto work = endpoint->second->work.find(token.work);
    if (work == endpoint->second->work.end()) return;
    removed = std::move(work->second.callback);
    endpoint->second->work.erase(work);
  }
}
} // namespace cgpui
