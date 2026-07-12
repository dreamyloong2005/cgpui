#pragma once

#include "cgpui/ui/async_io_hook.hpp"
#include "cgpui/ui/task_priority.hpp"

#include <atomic>
#include <mutex>
#include <vector>

namespace cgpui {

namespace detail {

enum class AsyncIoHookStatus { active, queued, complete, cancelled };

struct AsyncIoHookState {
  mutable std::mutex mutex;
  WindowRuntime* runtime = nullptr;
  AsyncIoId id{};
  std::atomic<AsyncIoHookStatus> status = AsyncIoHookStatus::active;
};

} // namespace detail

class WindowRuntime::RuntimeAsyncIoRegistry {
 public:
  struct Completion {
    AsyncIoId id;
    TaskPriority priority = TaskPriority::normal;
    AsyncIoResult result;
  };

  [[nodiscard]] AsyncIoHook create(
      WindowRuntime& runtime,
      TaskPriority priority,
      AsyncIoCompletionCallback callback);
  [[nodiscard]] bool notify(AsyncIoId id, AsyncIoResult result);
  [[nodiscard]] bool cancel(AsyncIoId id);
  [[nodiscard]] std::vector<Completion> take_completions();
  [[nodiscard]] AsyncIoCompletionCallback complete(AsyncIoId id);
  void shutdown();

 private:
  struct Record {
    AsyncIoId id;
    TaskPriority priority = TaskPriority::normal;
    AsyncIoCompletionCallback callback;
    std::shared_ptr<detail::AsyncIoHookState> state;
  };

  mutable std::mutex mutex_;
  std::vector<Record> records_;
  std::vector<Completion> completions_;
  std::uint64_t next_id_ = 1;
};

} // namespace cgpui
