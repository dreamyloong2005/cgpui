#include "window_runtime_test_support.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

namespace {

class TaskPoolView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  std::atomic_size_t started_count = 0;
  std::atomic_size_t active_count = 0;
  std::atomic_size_t peak_active_count = 0;
  std::atomic_bool release = false;
  std::size_t expected_completion_count = 0;
  std::size_t completion_count = 0;
  bool bounded_snapshot_matches = false;
  bool completed_snapshot_matches = false;
};

cgpui::WindowRuntime* task_pool_runtime = nullptr;
TaskPoolView* task_pool_view = nullptr;

bool wait_until(const auto& predicate) {
  for (int attempt = 0; attempt < 2000; ++attempt) {
    if (predicate()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return false;
}

void dispatch_task_pool_sequence() {
  auto& runtime = *task_pool_runtime;
  auto& view = *task_pool_view;
  const auto initial = runtime.diagnostics_snapshot();
  const std::size_t worker_count = initial.task_pool_worker_count;
  const std::size_t task_count = worker_count * 2 + 1;
  if (worker_count == 0) return;
  view.expected_completion_count = task_count;

  std::vector<cgpui::TaskHandle> tasks;
  tasks.reserve(task_count);
  for (std::size_t index = 0; index < task_count; ++index) {
    tasks.push_back(runtime.spawn_background_task(
        [&view](cgpui::TaskCancellationToken) {
          const std::size_t active = view.active_count.fetch_add(1) + 1;
          std::size_t peak = view.peak_active_count.load();
          while (peak < active &&
                 !view.peak_active_count.compare_exchange_weak(peak, active)) {
          }
          view.started_count.fetch_add(1);
          while (!view.release.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
          }
          view.active_count.fetch_sub(1);
        },
        [&view](const cgpui::WindowRuntimeContext&) {
          view.completion_count += 1;
        }));
  }

  if (!wait_until([&] { return view.started_count.load() == worker_count; })) {
    view.release.store(true);
    return;
  }
  const auto bounded = runtime.diagnostics_snapshot();
  view.bounded_snapshot_matches =
      bounded.task_pool_worker_count == worker_count &&
      bounded.task_pool_active_work_count == worker_count &&
      bounded.task_pool_queued_work_count == task_count - worker_count &&
      bounded.task_pool_peak_active_work_count == worker_count &&
      view.peak_active_count.load() == worker_count;

  view.release.store(true);
  if (!wait_until([&] {
        return runtime.diagnostics_snapshot().task_pool_completed_work_count ==
            task_count;
      })) {
    return;
  }
  runtime.drain_task_completions();
  const auto completed = runtime.diagnostics_snapshot();
  view.completed_snapshot_matches =
      completed.task_pool_worker_count == worker_count &&
      completed.task_pool_active_work_count == 0 &&
      completed.task_pool_queued_work_count == 0 &&
      completed.task_pool_completed_work_count == task_count &&
      completed.completed_task_count == task_count &&
      std::ranges::all_of(tasks, [](const cgpui::TaskHandle& task) {
        return task.complete();
      });
}

} // namespace

int main() {
  RuntimeFixture fixture;
  TaskPoolView view;
  fixture.app.on_run = &dispatch_task_pool_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  task_pool_runtime = &runtime;
  task_pool_view = &view;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  task_pool_runtime = nullptr;
  task_pool_view = nullptr;
  if (result != 0) return 1;
  if (!view.bounded_snapshot_matches || !view.completed_snapshot_matches) {
    return 2;
  }
  return view.completion_count == view.expected_completion_count ? 0 : 3;
}
