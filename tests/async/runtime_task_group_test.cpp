#include "cgpui/ui/task_group.hpp"
#include "window_runtime_test_support.hpp"

#include <atomic>
#include <chrono>
#include <concepts>
#include <thread>
#include <type_traits>

static_assert(!std::copy_constructible<cgpui::TaskGroup>);
static_assert(!std::is_copy_assignable_v<cgpui::TaskGroup>);
static_assert(std::movable<cgpui::TaskGroup>);

static_assert(requires(
    cgpui::WindowRuntime& runtime,
    const cgpui::WindowRuntimeContext& context) {
  { runtime.create_task_group() } -> std::same_as<cgpui::TaskGroup>;
  { context.create_task_group() } -> std::same_as<cgpui::TaskGroup>;
  { context.async_context().create_task_group() } ->
      std::same_as<cgpui::TaskGroup>;
});

namespace {

class TaskGroupView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  std::atomic_size_t started_count = 0;
  std::atomic_size_t cancellation_observed_count = 0;
  std::size_t completion_count = 0;
  bool explicit_cancel_matches = false;
  bool destructor_cancel_matches = false;
  bool normal_completion_matches = false;
  bool move_matches = false;
};

cgpui::WindowRuntime* task_group_runtime = nullptr;
TaskGroupView* task_group_view = nullptr;

bool wait_until(const auto& predicate) {
  for (int attempt = 0; attempt < 3000; ++attempt) {
    if (predicate()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return false;
}

cgpui::BackgroundTaskCallback cancellable_work(TaskGroupView& view) {
  return [&view](cgpui::TaskCancellationToken token) {
    view.started_count.fetch_add(1);
    while (!token.cancellation_requested()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    view.cancellation_observed_count.fetch_add(1);
  };
}

void dispatch_task_group_sequence() {
  auto& runtime = *task_group_runtime;
  auto& view = *task_group_view;

  cgpui::TaskGroup group = runtime.create_task_group();
  const cgpui::TaskGroupId group_id = group.id();
  const cgpui::TaskHandle foreground = group.spawn_task(
      [&view](const cgpui::WindowRuntimeContext&) {
        view.completion_count += 1;
      });
  const cgpui::TaskHandle background = group.spawn_background_task(
      cgpui::TaskPriority::high,
      cancellable_work(view),
      [&view](const cgpui::WindowRuntimeContext&) {
        view.completion_count += 1;
      });
  if (!wait_until([&] { return view.started_count.load() == 1; })) return;
  const bool active_matches =
      group.id() == group_id && group.task_count() == 2 &&
      group.active_task_count() == 2 && !group.complete() &&
      !group.cancelled();
  const bool cancelled = group.cancel();
  if (!wait_until([&] {
        return view.cancellation_observed_count.load() == 1;
      })) {
    return;
  }
  runtime.drain_task_completions();
  view.explicit_cancel_matches =
      active_matches && cancelled && group.cancelled() && group.complete() &&
      group.active_task_count() == 0 && foreground.cancelled() &&
      background.cancelled() && view.completion_count == 0;

  const std::size_t started_before_scope = view.started_count.load();
  const std::size_t cancelled_before_scope =
      view.cancellation_observed_count.load();
  cgpui::TaskHandle scoped_task;
  {
    cgpui::TaskGroup scoped = runtime.create_task_group();
    scoped_task = scoped.spawn_background_task(
        cancellable_work(view),
        [&view](const cgpui::WindowRuntimeContext&) {
          view.completion_count += 1;
        });
    if (!wait_until([&] {
          return view.started_count.load() == started_before_scope + 1;
        })) {
      return;
    }
  }
  if (!wait_until([&] {
        return view.cancellation_observed_count.load() ==
            cancelled_before_scope + 1;
      })) {
    return;
  }
  runtime.drain_task_completions();
  view.destructor_cancel_matches =
      scoped_task.cancelled() && view.completion_count == 0;

  cgpui::TaskGroup completed = runtime.create_task_group();
  std::atomic_size_t normal_started = 0;
  const cgpui::TaskHandle normal_foreground = completed.spawn_task(
      [&view](const cgpui::WindowRuntimeContext&) {
        view.completion_count += 1;
      });
  const cgpui::TaskHandle normal_background = completed.spawn_background_task(
      [&normal_started](cgpui::TaskCancellationToken) {
        normal_started.fetch_add(1);
      },
      [&view](const cgpui::WindowRuntimeContext&) {
        view.completion_count += 1;
      });
  if (!wait_until([&] { return normal_started.load() == 1; })) return;
  if (!runtime.complete_task(normal_foreground.id())) return;
  runtime.drain_task_completions();
  view.normal_completion_matches =
      completed.task_count() == 2 && completed.active_task_count() == 0 &&
      completed.complete() && !completed.cancelled() &&
      normal_foreground.complete() && normal_background.complete() &&
      view.completion_count == 2;

  cgpui::TaskGroup source = runtime.create_task_group();
  const cgpui::TaskGroupId moved_id = source.id();
  cgpui::TaskGroup destination = std::move(source);
  view.move_matches =
      source.id().value == 0 && destination.id() == moved_id;
}

} // namespace

int main() {
  RuntimeFixture fixture;
  TaskGroupView view;
  fixture.app.on_run = &dispatch_task_group_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  task_group_runtime = &runtime;
  task_group_view = &view;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  task_group_runtime = nullptr;
  task_group_view = nullptr;
  if (result != 0) return 1;
  if (!view.explicit_cancel_matches) return 2;
  if (!view.destructor_cancel_matches) return 3;
  if (!view.normal_completion_matches) return 4;
  return view.move_matches ? 0 : 5;
}
