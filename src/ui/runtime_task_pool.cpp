#include "ui_internal.hpp"
#include "runtime_task_pool_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

std::size_t task_pool_worker_count() {
  const unsigned int hardware = std::thread::hardware_concurrency();
  return std::clamp<std::size_t>(hardware == 0 ? 2U : hardware, 1U, 4U);
}

} // namespace

WindowRuntime::RuntimeTaskPool::RuntimeTaskPool() {
  const std::size_t worker_count = task_pool_worker_count();
  workers_.reserve(worker_count);
  for (std::size_t index = 0; index < worker_count; ++index) {
    workers_.emplace_back(
        [this](std::stop_token stop_token) { run_worker(stop_token); });
  }
}

WindowRuntime::RuntimeTaskPool::~RuntimeTaskPool() {
  shutdown();
}

bool WindowRuntime::RuntimeTaskPool::submit(Work work) {
  if (!work) return false;
  {
    std::lock_guard lock(mutex_);
    if (!accepting_) return false;
    queue_.push_back(std::move(work));
  }
  condition_.notify_one();
  return true;
}

WindowRuntime::RuntimeTaskPool::Snapshot
WindowRuntime::RuntimeTaskPool::snapshot() const {
  std::lock_guard lock(mutex_);
  return Snapshot{
      .worker_count = workers_.size(),
      .queued_work_count = queue_.size(),
      .active_work_count = active_work_count_,
      .peak_active_work_count = peak_active_work_count_,
      .completed_work_count = completed_work_count_,
  };
}

void WindowRuntime::RuntimeTaskPool::shutdown() {
  std::vector<std::jthread> workers;
  {
    std::lock_guard lock(mutex_);
    if (stopping_ && workers_.empty()) return;
    accepting_ = false;
    stopping_ = true;
    workers.swap(workers_);
  }
  condition_.notify_all();
  workers.clear();
}

void WindowRuntime::RuntimeTaskPool::run_worker(std::stop_token stop_token) {
  while (true) {
    Work work;
    {
      std::unique_lock lock(mutex_);
      condition_.wait(lock, stop_token, [this] {
        return stopping_ || !queue_.empty();
      });
      if (queue_.empty()) {
        if (stopping_ || stop_token.stop_requested()) return;
        continue;
      }
      work = std::move(queue_.front());
      queue_.pop_front();
      active_work_count_ += 1;
      peak_active_work_count_ =
          std::max(peak_active_work_count_, active_work_count_);
    }

    try {
      work();
    } catch (...) {
    }

    {
      std::lock_guard lock(mutex_);
      active_work_count_ -= 1;
      completed_work_count_ += 1;
      if (stopping_ && queue_.empty() && active_work_count_ == 0) {
        condition_.notify_all();
      }
    }
  }
}

} // namespace cgpui
