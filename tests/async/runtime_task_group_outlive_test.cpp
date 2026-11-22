#include "popup_test_support.hpp"

#include <atomic>
#include <iostream>
#include <memory>
#include <utility>

namespace {
using Calls = std::shared_ptr<std::atomic_uint>;

bool detached_contract(cgpui::TaskGroup& group, const Calls& calls) {
  if (group.task_count() != 0 || group.active_task_count() != 0 ||
      group.complete() || group.cancelled() || group.cancel()) return false;
  const auto completion = [calls](const cgpui::WindowRuntimeContext&) { ++*calls; };
  const auto work = [calls](cgpui::TaskCancellationToken) { ++*calls; };
  if (group.spawn_task(completion).id().value != 0 ||
      group.spawn_task(cgpui::TaskPriority::high, completion).id().value != 0 ||
      group.spawn_background_task(work, completion).id().value != 0 ||
      group.spawn_background_task(cgpui::TaskPriority::high, work, completion).id().value != 0) {
    return false;
  }
  const auto ordinary = group.try_spawn_task(completion);
  const auto priority = group.try_spawn_task(cgpui::TaskPriority::high, completion);
  const auto background = group.try_spawn_background_task(work, completion);
  const auto priority_background =
      group.try_spawn_background_task(cgpui::TaskPriority::high, work, completion);
  for (const auto* result : {&ordinary, &priority, &background, &priority_background}) {
    if (*result || result->error().code != cgpui::ErrorCode::invalid_argument) return false;
  }
  auto child = group.create_child_group();
  return child.id().value == 0 && child.task_count() == 0 &&
      child.active_task_count() == 0 && !child.complete() && !child.cancelled() &&
      !child.cancel() && calls->load() == 0;
}

class RootView final : public cgpui::View {
 public:
  explicit RootView(bool& destroyed) : destroyed_(destroyed) {}
  ~RootView() override { destroyed_ = true; }
  void paint(cgpui::PaintList&, cgpui::Size) override {}
  cgpui::TaskGroup group;
 private:
  bool& destroyed_;
};

int root_view_group_outlives_run_app() {
  popup_test::Application app;
  const auto calls = std::make_shared<std::atomic_uint>(0);
  bool destroyed = false;
  auto root = std::make_unique<RootView>(destroyed);
  bool attached = false;
  app.on_run = [] {};
  cgpui::AppRunnerOptions options;
  options.runtime.request_initial_redraw = false;
  options.setup = [&](cgpui::WindowRuntime& runtime) {
    root->group = runtime.create_task_group();
    const auto task = root->group.spawn_task(
        [calls](const cgpui::WindowRuntimeContext&) { ++*calls; });
    attached = root->group.id().value != 0 && task.id().value != 0 &&
        root->group.task_count() == 1 && root->group.active_task_count() == 1;
  };
  const int result = cgpui::run_app(app, *root, [](const auto&)
      -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
    return std::make_unique<popup_test::Renderer>(true);
  }, std::move(options));
  if (result != 0 || !attached || destroyed) return 1;
  // run_app's runtime has been destroyed, but the caller-owned view is alive.
  if (!detached_contract(root->group, calls)) return 2;
  root.reset();
  return destroyed && calls->load() == 0 ? 0 : 3;
}

int moved_groups_outlive_a_local_runtime() {
  popup_test::Application app;
  int live_views = 0;
  popup_test::CountingView root(live_views);
  popup_test::Renderer renderer(true);
  const auto calls = std::make_shared<std::atomic_uint>(0);
  cgpui::TaskGroup source;
  cgpui::TaskGroup destination;
  cgpui::TaskGroup destruction_only;
  cgpui::TaskGroup spawn_first;
  {
    cgpui::WindowRuntime runtime(app, root, [&](const auto&)
        -> cgpui::Result<cgpui::Renderer*> { return &renderer; });
    source = runtime.create_task_group();
    destination = runtime.create_task_group();
    destruction_only = runtime.create_task_group();
    spawn_first = runtime.create_task_group();
    for (auto* group : {&source, &destination, &destruction_only, &spawn_first}) {
      const auto task = group->spawn_task(
          [calls](const cgpui::WindowRuntimeContext&) { ++*calls; });
      if (group->id().value == 0 || task.id().value == 0 ||
          group->task_count() != 1 || group->active_task_count() != 1) return 10;
    }
  }
  const auto rejected = spawn_first.try_spawn_background_task(
      cgpui::TaskPriority::high,
      [calls](cgpui::TaskCancellationToken) { ++*calls; },
      [calls](const cgpui::WindowRuntimeContext&) { ++*calls; });
  if (rejected || rejected.error().code != cgpui::ErrorCode::invalid_argument ||
      !detached_contract(spawn_first, calls)) return 11;
  // Move and later destroy these groups without first normalizing expired state.
  cgpui::TaskGroup moved(std::move(source));
  if (source.id().value != 0 || !detached_contract(source, calls) ||
      !detached_contract(moved, calls)) return 12;
  // Assignment must safely cancel an old destination whose runtime is also gone.
  destination = std::move(moved);
  if (moved.id().value != 0 || !detached_contract(moved, calls) ||
      !detached_contract(destination, calls)) return 13;
  return 0;
}
} // namespace

int main() {
  for (const auto test : {root_view_group_outlives_run_app,
                          moved_groups_outlive_a_local_runtime}) {
    if (const int result = test(); result != 0) {
      std::cerr << "task group outlive failure: " << result << '\n';
      return result;
    }
  }
  return 0;
}
