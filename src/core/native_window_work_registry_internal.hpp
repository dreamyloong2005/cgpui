#pragma once

#include "native_window_work_internal.hpp"
#include <mutex>
#include <thread>
#include <unordered_map>

namespace cgpui::native_window_work_detail {

struct Work {
  std::weak_ptr<NativeWindowWork> callback;
  bool pending = false;
  bool running = false;
  bool explicit_repost = false;
};
struct Endpoint {
  void* window = nullptr;
  std::thread::id owner;
  NativeWindowWorkPost post = nullptr;
  std::unordered_map<std::uintptr_t, Work> work;
};
struct Registry {
  std::mutex mutex;
  std::uintptr_t next = 0;
  std::unordered_map<std::uintptr_t, std::shared_ptr<Endpoint>> endpoints;
};
Registry& registry();
std::uintptr_t next_identity(Registry&) noexcept;

} // namespace cgpui::native_window_work_detail
