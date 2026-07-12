#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>

namespace cgpui {

class WindowRuntime::RuntimeTaskPool {
 public:
  using Work = std::function<void()>;

  struct Snapshot {
    std::size_t worker_count = 0;
    std::size_t queued_work_count = 0;
    std::size_t active_work_count = 0;
    std::size_t peak_active_work_count = 0;
    std::size_t completed_work_count = 0;
  };

  RuntimeTaskPool();
  ~RuntimeTaskPool();
  RuntimeTaskPool(const RuntimeTaskPool&) = delete;
  RuntimeTaskPool& operator=(const RuntimeTaskPool&) = delete;

  [[nodiscard]] bool submit(Work work);
  [[nodiscard]] Snapshot snapshot() const;
  void shutdown();

 private:
  void run_worker(std::stop_token stop_token);

  mutable std::mutex mutex_;
  std::condition_variable_any condition_;
  std::deque<Work> queue_;
  std::vector<std::jthread> workers_;
  std::size_t active_work_count_ = 0;
  std::size_t peak_active_work_count_ = 0;
  std::size_t completed_work_count_ = 0;
  bool accepting_ = true;
  bool stopping_ = false;
};

} // namespace cgpui
