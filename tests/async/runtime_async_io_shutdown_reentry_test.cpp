#include "popup_test_support.hpp"
#include "cgpui/ui/async_io_hook.hpp"

#include <array>
#include <iostream>
#include <memory>

namespace {
struct RegisterOnDestruction {
  cgpui::WindowRuntime* runtime = nullptr;
  cgpui::AsyncIoHook* escaped = nullptr;
  ~RegisterOnDestruction() {
    *escaped = runtime->create_async_io_hook(
        [](const cgpui::WindowRuntimeContext&, const cgpui::AsyncIoResult&) {});
  }
};
}

int main() {
  popup_test::Application app;
  int views = 0;
  popup_test::CountingView root(views);
  popup_test::Renderer renderer(true);
  std::array<cgpui::AsyncIoHook, 2> escaped;
  {
    cgpui::WindowRuntime runtime(app, root,
        [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
          return &renderer;
        });
    auto from_io = std::make_shared<RegisterOnDestruction>();
    from_io->runtime = &runtime;
    from_io->escaped = &escaped[0];
    (void)runtime.create_async_io_hook(
        [from_io](const cgpui::WindowRuntimeContext&, const cgpui::AsyncIoResult&) {});
    auto from_task = std::make_shared<RegisterOnDestruction>();
    from_task->runtime = &runtime;
    from_task->escaped = &escaped[1];
    (void)runtime.spawn_task([from_task](const cgpui::WindowRuntimeContext&) {});
  }
  for (const auto& hook : escaped) {
    // Inspect only local state before invoking any method that could touch
    // the freed runtime on the broken implementation.
    if (hook.id().value != 0 || hook.active()) {
      std::cerr << "shutdown accepted an escaping async I/O registration\n";
      return 1;
    }
    if (hook.notify({}) || hook.cancel()) return 2;
  }
  return 0;
}
