#include "popup_test_support.hpp"
#include "../../src/ui/runtime_cross_thread_entity_internal.hpp"

#include <memory>

namespace {
struct ReenterQueue {
  std::weak_ptr<cgpui::detail::CrossThreadEntityQueueState> queue;
  bool* rejected = nullptr;
  ~ReenterQueue() {
    const auto owner = queue.lock();
    *rejected = owner && !owner->enqueue([](const cgpui::WindowRuntimeContext&) {});
  }
};
}

int main() {
  popup_test::Application app;
  int views = 0;
  popup_test::CountingView view(views);
  popup_test::Renderer renderer(true);
  cgpui::WindowRuntime runtime(app, view,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        return &renderer;
      });
  auto queue = std::make_shared<cgpui::detail::CrossThreadEntityQueueState>(runtime);
  bool rejected = false;
  auto capture = std::make_shared<ReenterQueue>();
  capture->queue = queue;
  capture->rejected = &rejected;
  const std::weak_ptr<ReenterQueue> weak = capture;
  if (!queue->enqueue([capture](const cgpui::WindowRuntimeContext&) {})) return 1;
  capture.reset();
  queue->shutdown();
  return rejected && weak.expired() ? 0 : 2;
}
