#include "cgpui/ui/task_priority.hpp"
#include "window_runtime_test_support.hpp"

#include <atomic>
#include <chrono>
#include <concepts>
#include <mutex>
#include <thread>
#include <vector>

static_assert(requires(
    cgpui::WindowRuntime& runtime,
    cgpui::TaskCompletionCallback completion,
    cgpui::BackgroundTaskCallback work) {
  { runtime.spawn_task(cgpui::TaskPriority::high, completion) } ->
      std::same_as<cgpui::TaskHandle>;
  { runtime.try_spawn_task(cgpui::TaskPriority::low, completion) } ->
      std::same_as<cgpui::Result<cgpui::TaskHandle>>;
  { runtime.spawn_background_task(
        cgpui::TaskPriority::normal, work, completion) } ->
      std::same_as<cgpui::TaskHandle>;
  { runtime.try_spawn_background_task(
        cgpui::TaskPriority::high, work, completion) } ->
      std::same_as<cgpui::Result<cgpui::TaskHandle>>;
});

static_assert(requires(
    const cgpui::WindowRuntimeContext& context,
    cgpui::TaskCompletionCallback completion,
    cgpui::BackgroundTaskCallback work) {
  { context.spawn_task(cgpui::TaskPriority::high, completion) } ->
      std::same_as<cgpui::TaskHandle>;
  { context.try_spawn_background_task(
        cgpui::TaskPriority::low, work, completion) } ->
      std::same_as<cgpui::Result<cgpui::TaskHandle>>;
  { context.async_context().spawn_background_task(
        cgpui::TaskPriority::normal, work, completion) } ->
      std::same_as<cgpui::TaskHandle>;
  { context.async_context().try_spawn_task(
        cgpui::TaskPriority::high, completion) } ->
      std::same_as<cgpui::Result<cgpui::TaskHandle>>;
});

namespace {

class PriorityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  std::atomic_size_t blockers_started = 0;
  std::atomic_size_t blocker_permits = 0;
  std::mutex order_mutex;
  std::vector<int> work_order;
  std::vector<int> completion_order;
  bool foreground_order_matches = false;
  bool background_order_matches = false;
};

cgpui::WindowRuntime* priority_runtime = nullptr;
PriorityView* priority_view = nullptr;

bool wait_until(const auto& predicate) {
  for (int attempt = 0; attempt < 3000; ++attempt) {
    if (predicate()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return false;
}

void acquire_blocker_permit(std::atomic_size_t& permits) {
  while (true) {
    std::size_t available = permits.load();
    if (available != 0 &&
        permits.compare_exchange_weak(available, available - 1)) {
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void dispatch_priority_sequence() {
  auto& runtime = *priority_runtime;
  auto& view = *priority_view;

  const cgpui::TaskHandle low = runtime.spawn_task(
      cgpui::TaskPriority::low,
      [&view](const cgpui::WindowRuntimeContext&) {
        view.completion_order.push_back(1);
      });
  const cgpui::TaskHandle high = runtime.spawn_task(
      cgpui::TaskPriority::high,
      [&view](const cgpui::WindowRuntimeContext&) {
        view.completion_order.push_back(3);
      });
  const cgpui::TaskHandle normal = runtime.spawn_task(
      cgpui::TaskPriority::normal,
      [&view](const cgpui::WindowRuntimeContext&) {
        view.completion_order.push_back(2);
      });
  (void)runtime.complete_task(low.id());
  (void)runtime.complete_task(high.id());
  (void)runtime.complete_task(normal.id());
  runtime.drain_task_completions();
  view.foreground_order_matches =
      view.completion_order == std::vector<int>{3, 2, 1};
  view.completion_order.clear();

  const std::size_t worker_count =
      runtime.diagnostics_snapshot().task_pool_worker_count;
  if (worker_count == 0) return;
  std::vector<cgpui::TaskHandle> blockers;
  for (std::size_t index = 0; index < worker_count; ++index) {
    blockers.push_back(runtime.spawn_background_task(
        cgpui::TaskPriority::normal,
        [&view](cgpui::TaskCancellationToken) {
          view.blockers_started.fetch_add(1);
          acquire_blocker_permit(view.blocker_permits);
        },
        [](const cgpui::WindowRuntimeContext&) {}));
  }
  if (!wait_until([&] { return view.blockers_started.load() == worker_count; })) {
    view.blocker_permits.store(worker_count);
    return;
  }

  const auto submit = [&runtime, &view](cgpui::TaskPriority priority, int value) {
    return runtime.spawn_background_task(
        priority,
        [&view, value](cgpui::TaskCancellationToken) {
          std::lock_guard lock(view.order_mutex);
          view.work_order.push_back(value);
        },
        [&view, value](const cgpui::WindowRuntimeContext&) {
          view.completion_order.push_back(value);
        });
  };
  std::vector<cgpui::TaskHandle> priority_tasks;
  priority_tasks.push_back(submit(cgpui::TaskPriority::low, 1));
  priority_tasks.push_back(submit(cgpui::TaskPriority::low, 2));
  priority_tasks.push_back(submit(cgpui::TaskPriority::normal, 3));
  priority_tasks.push_back(submit(cgpui::TaskPriority::high, 4));
  priority_tasks.push_back(submit(cgpui::TaskPriority::high, 5));

  view.blocker_permits.store(1);
  if (!wait_until([&] {
        std::lock_guard lock(view.order_mutex);
        return view.work_order.size() == priority_tasks.size();
      })) {
    view.blocker_permits.store(worker_count);
    return;
  }
  view.blocker_permits.store(worker_count);
  const std::size_t expected_background_count = worker_count + priority_tasks.size();
  if (!wait_until([&] {
        return runtime.diagnostics_snapshot().task_pool_completed_work_count ==
            expected_background_count;
      })) {
    return;
  }
  runtime.drain_task_completions();
  std::lock_guard lock(view.order_mutex);
  view.background_order_matches =
      view.work_order == std::vector<int>{4, 5, 3, 1, 2} &&
      view.completion_order == std::vector<int>{4, 5, 3, 1, 2};
}

} // namespace

int main() {
  RuntimeFixture fixture;
  PriorityView view;
  fixture.app.on_run = &dispatch_priority_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  priority_runtime = &runtime;
  priority_view = &view;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  priority_runtime = nullptr;
  priority_view = nullptr;
  if (result != 0) return 1;
  if (!view.foreground_order_matches) return 2;
  return view.background_order_matches ? 0 : 3;
}
