#include "cgpui/ui/task_group.hpp"
#include "window_runtime_test_support.hpp"

#include <atomic>
#include <chrono>
#include <concepts>
#include <thread>

static_assert(requires(const cgpui::TaskGroup& group) {
  { group.create_child_group() } -> std::same_as<cgpui::TaskGroup>;
});

namespace {

class PropagationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  std::atomic_size_t started_count = 0;
  std::atomic_size_t cancellation_count = 0;
  std::size_t completion_count = 0;
  bool subtree_matches = false;
  bool ancestor_destructor_matches = false;
};

cgpui::WindowRuntime* propagation_runtime = nullptr;
PropagationView* propagation_view = nullptr;

bool wait_until(const auto& predicate) {
  for (int attempt = 0; attempt < 3000; ++attempt) {
    if (predicate()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return false;
}

cgpui::BackgroundTaskCallback cancellable_work(PropagationView& view) {
  return [&view](cgpui::TaskCancellationToken token) {
    view.started_count.fetch_add(1);
    while (!token.cancellation_requested()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    view.cancellation_count.fetch_add(1);
  };
}

cgpui::TaskCompletionCallback completion(PropagationView& view) {
  return [&view](const cgpui::WindowRuntimeContext&) {
    view.completion_count += 1;
  };
}

void dispatch_propagation_sequence() {
  auto& runtime = *propagation_runtime;
  auto& view = *propagation_view;

  cgpui::TaskGroup parent = runtime.create_task_group();
  cgpui::TaskGroup child = parent.create_child_group();
  cgpui::TaskGroup grandchild = child.create_child_group();
  cgpui::TaskGroup sibling = parent.create_child_group();
  const cgpui::TaskHandle parent_task = parent.spawn_task(completion(view));
  const cgpui::TaskHandle child_task = child.spawn_task(completion(view));
  const cgpui::TaskHandle deep_task = grandchild.spawn_background_task(
      cancellable_work(view), completion(view));
  const cgpui::TaskHandle sibling_task = sibling.spawn_task(completion(view));
  if (!wait_until([&] { return view.started_count.load() == 1; })) return;

  const bool tree_before =
      parent.task_count() == 4 && parent.active_task_count() == 4 &&
      child.task_count() == 2 && child.active_task_count() == 2;
  const bool child_cancelled = child.cancel();
  if (!wait_until([&] { return view.cancellation_count.load() == 1; })) return;
  runtime.drain_task_completions();
  const bool child_isolated =
      child_cancelled && child.cancelled() && child.complete() &&
      child.active_task_count() == 0 && child_task.cancelled() &&
      deep_task.cancelled() && !parent.cancelled() && !sibling.cancelled() &&
      parent_task.active() && sibling_task.active() &&
      parent.task_count() == 4 && parent.active_task_count() == 2;
  const bool parent_cancelled = parent.cancel();
  const cgpui::TaskGroup rejected_child = parent.create_child_group();
  const cgpui::TaskHandle rejected_task = grandchild.spawn_task(completion(view));
  runtime.drain_task_completions();
  view.subtree_matches =
      tree_before && child_isolated && parent_cancelled && parent.cancelled() &&
      parent.complete() && parent.active_task_count() == 0 &&
      parent_task.cancelled() && sibling.cancelled() &&
      sibling_task.cancelled() && rejected_child.id().value == 0 &&
      rejected_task.id().value == 0 && view.completion_count == 0;

  const std::size_t started_before = view.started_count.load();
  const std::size_t cancelled_before = view.cancellation_count.load();
  cgpui::TaskGroup surviving_child;
  cgpui::TaskHandle descendant_task;
  {
    cgpui::TaskGroup owner = runtime.create_task_group();
    surviving_child = owner.create_child_group();
    cgpui::TaskGroup deepest = surviving_child.create_child_group();
    descendant_task = deepest.spawn_background_task(
        cancellable_work(view), completion(view));
    if (!wait_until([&] {
          return view.started_count.load() == started_before + 1;
        })) {
      return;
    }
  }
  if (!wait_until([&] {
        return view.cancellation_count.load() == cancelled_before + 1;
      })) {
    return;
  }
  runtime.drain_task_completions();
  view.ancestor_destructor_matches =
      surviving_child.cancelled() && surviving_child.complete() &&
      descendant_task.cancelled() && view.completion_count == 0;
}

} // namespace

int main() {
  RuntimeFixture fixture;
  PropagationView view;
  fixture.app.on_run = &dispatch_propagation_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  propagation_runtime = &runtime;
  propagation_view = &view;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  propagation_runtime = nullptr;
  propagation_view = nullptr;
  if (result != 0) return 1;
  if (!view.subtree_matches) return 2;
  return view.ancestor_destructor_matches ? 0 : 3;
}
