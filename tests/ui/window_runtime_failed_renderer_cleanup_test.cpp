#include "runtime_window_lifecycle_test_support.hpp"

#include <iostream>

namespace {
using namespace runtime_window_test;

int failed_root_has_detached_context(bool empty_renderer) {
  Application app;
  View root;
  cgpui::WindowRuntime runtime(app, root,
      [empty_renderer](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        if (empty_renderer) return static_cast<cgpui::Renderer*>(nullptr);
        return std::unexpected(cgpui::Error{
            .code = cgpui::ErrorCode::renderer_initialization_failed,
            .message = "injected root renderer failure"});
      });
  if (runtime.run({}, {.request_initial_redraw = false}) != 1 ||
      app.root == nullptr || !app.root->destroyed) return 1;
  const auto* record = runtime.window_runtime_record(runtime.root_window_runtime_id());
  if (record == nullptr || record->active || record->window != nullptr ||
      record->renderer != nullptr) return 2;

  bool detached = false;
  runtime.batch_updates([&](const cgpui::WindowRuntimeContext& context) {
    detached = &context.platform_window != app.root->native_address;
  });
  // Compare addresses before calling any virtual member: RED can report the
  // dangling runtime target deterministically instead of relying on a crash.
  if (!detached) return 3;
  const int redraws = app.root->redraws;
  runtime.request_render();
  runtime.request_layout();
  runtime.request_paint();
  return app.root->redraws == redraws ? 0 : 4;
}
} // namespace

int main() {
  for (const bool empty_renderer : {false, true}) {
    if (const int result = failed_root_has_detached_context(empty_renderer); result != 0) {
      std::cerr << "failed root renderer cleanup " << empty_renderer << ": " << result << '\n';
      return result;
    }
  }
  return 0;
}
