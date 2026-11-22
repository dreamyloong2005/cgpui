#pragma once

#include "cgpui/ui/runtime_handles.hpp"
#include "cgpui/ui/task_priority.hpp"

#include <cstddef>
#include <memory>

namespace cgpui {

class WindowRuntime;
namespace detail { struct TaskGroupLifetime; }

class TaskGroup {
 public:
  TaskGroup() = default;
  ~TaskGroup();
  TaskGroup(const TaskGroup&) = delete;
  TaskGroup& operator=(const TaskGroup&) = delete;
  TaskGroup(TaskGroup&& other) noexcept;
  TaskGroup& operator=(TaskGroup&& other) noexcept;

  [[nodiscard]] TaskGroupId id() const { return id_; }
  [[nodiscard]] TaskGroup create_child_group() const;
  [[nodiscard]] std::size_t task_count() const;
  [[nodiscard]] std::size_t active_task_count() const;
  [[nodiscard]] bool complete() const;
  [[nodiscard]] bool cancelled() const;
  [[nodiscard]] bool cancel();

  [[nodiscard]] TaskHandle spawn_task(TaskCompletionCallback callback) const;
  [[nodiscard]] TaskHandle spawn_task(
      TaskPriority priority,
      TaskCompletionCallback callback) const;
  [[nodiscard]] Result<TaskHandle> try_spawn_task(
      TaskCompletionCallback callback) const;
  [[nodiscard]] Result<TaskHandle> try_spawn_task(
      TaskPriority priority,
      TaskCompletionCallback callback) const;
  [[nodiscard]] TaskHandle spawn_background_task(
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  [[nodiscard]] TaskHandle spawn_background_task(
      TaskPriority priority,
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  [[nodiscard]] Result<TaskHandle> try_spawn_background_task(
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;
  [[nodiscard]] Result<TaskHandle> try_spawn_background_task(
      TaskPriority priority,
      BackgroundTaskCallback work,
      TaskCompletionCallback completion) const;

 private:
  friend class WindowRuntime;
  friend struct WindowRuntimeContext;
  TaskGroup(WindowRuntime& runtime, TaskGroupId id);
  [[nodiscard]] WindowRuntime* attached_runtime() const;

  WindowRuntime* runtime_ = nullptr;
  std::weak_ptr<detail::TaskGroupLifetime> runtime_lifetime_;
  TaskGroupId id_{};
  WindowRuntimeId window_runtime_id_{};
  TaskCompletionCallback bind_callback(TaskCompletionCallback callback) const;
};

} // namespace cgpui
