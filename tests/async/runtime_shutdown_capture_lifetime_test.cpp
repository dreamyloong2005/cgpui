#include "popup_test_support.hpp"

#include <iostream>
#include <memory>

namespace {
struct GroupCapture {
  cgpui::TaskGroup group;
  int* released = nullptr;
  bool* valid = nullptr;
  ~GroupCapture() {
    // This user destructor runs while the runtime is shutting down. Group
    // metadata must still exist until all user-owned resources are retired.
    *valid = *valid && group.task_count() == 1 && group.active_task_count() == 0;
    ++*released;
  }
};

struct ReentrantCapture {
  cgpui::WindowRuntime* runtime = nullptr;
  cgpui::TaskGroup group;
  bool* valid = nullptr;
  ~ReentrantCapture() {
    const auto completion = [](const cgpui::WindowRuntimeContext&) {};
    *valid = *valid && runtime->spawn_task(completion).id().value == 0 &&
        group.spawn_task(completion).id().value == 0 &&
        !runtime->create_async_io_hook([](const auto&, const auto&) {}).active();
  }
};
}

int main() {
  popup_test::Application app;
  int views = 0;
  popup_test::CountingView root(views);
  popup_test::Renderer renderer(true);
  int released = 0;
  bool valid = true;
  {
    cgpui::WindowRuntime runtime(app, root,
        [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
          return &renderer;
        });
    for (int storage = 0; storage < 10; ++storage) {
      auto capture = std::make_shared<GroupCapture>();
      capture->group = runtime.create_task_group();
      capture->released = &released;
      capture->valid = &valid;
      (void)capture->group.spawn_task([](const cgpui::WindowRuntimeContext&) {});
      if (storage == 0) {
        (void)runtime.schedule_repeating_timer(10,
            [capture](const cgpui::WindowRuntimeContext&) {});
      } else if (storage == 1) {
        runtime.defer([capture](const cgpui::WindowRuntimeContext&) {});
      } else if (storage == 2) {
        runtime.set_global(capture);
      } else if (storage == 3) {
        (void)runtime.insert_entity(capture);
      } else if (storage == 4) {
        (void)runtime.observe_window([capture](const cgpui::WindowRuntimeContext&) {});
      } else if (storage == 5) {
        (void)runtime.observe_view(cgpui::ViewId{1},
            [capture](const cgpui::WindowRuntimeContext&, cgpui::ViewId) {});
      } else if (storage == 6) {
        const auto model = runtime.insert_entity(1);
        (void)runtime.observe_model(model,
            [capture](const cgpui::WindowRuntimeContext&, cgpui::Model<int>) {});
      } else if (storage == 7) {
        runtime.register_action("shutdown.capture", [capture](const cgpui::WindowRuntimeContext&) {
          return cgpui::EventResult::unhandled();
        });
      } else if (storage == 8) {
        runtime.set_after_frame_callback([capture](const cgpui::WindowRuntimeContext&) {});
      } else {
        (void)runtime.start_animation({.duration_ms = 1000},
            [capture](const cgpui::WindowRuntimeContext&, const cgpui::AnimationSnapshot&) {});
      }
    }
    auto reentrant = std::make_shared<ReentrantCapture>();
    reentrant->runtime = &runtime;
    reentrant->group = runtime.create_task_group();
    reentrant->valid = &valid;
    runtime.set_global(reentrant);
  }
  if (released != 10 || !valid) {
    std::cerr << "user captures outlived task metadata during runtime shutdown\n";
    return 1;
  }
  return 0;
}
