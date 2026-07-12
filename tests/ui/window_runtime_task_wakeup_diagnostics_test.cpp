#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* task_diagnostics_fixture = nullptr;
cgpui::WindowRuntime* task_diagnostics_runtime = nullptr;
cgpui::RuntimeDiagnosticsSnapshot task_diagnostics_snapshot;
int completed_task_count = 0;

void dispatch_task_diagnostics_stress() {
  for (int index = 0; index < 64; ++index) {
    const cgpui::TaskHandle task = task_diagnostics_runtime->spawn_task(
        [](const cgpui::WindowRuntimeContext&) { ++completed_task_count; });
    if (task.id().value != static_cast<std::uint64_t>(index + 1) ||
        !task_diagnostics_runtime->complete_task(task.id()))
      return;
  }
  if (task_diagnostics_fixture->app.request_wakeup_count != 64 ||
      !task_diagnostics_fixture->app.wakeup_pending)
    return;
  task_diagnostics_fixture->app.dispatch_wakeup();
  task_diagnostics_snapshot = task_diagnostics_runtime->diagnostics_snapshot();
}

} // namespace

int main() {
  RuntimeFixture fixture;
  task_diagnostics_fixture = &fixture;
  task_diagnostics_runtime = nullptr;
  completed_task_count = 0;
  fixture.app.on_run = &dispatch_task_diagnostics_stress;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  task_diagnostics_runtime = &runtime;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  task_diagnostics_fixture = nullptr;
  task_diagnostics_runtime = nullptr;
  if (result != 0 || completed_task_count != 64 || fixture.app.wakeup_pending ||
      task_diagnostics_snapshot.completed_task_count != 64 ||
      task_diagnostics_snapshot.queued_task_count != 0 ||
      task_diagnostics_snapshot.platform_diagnostics.size() != 32)
    return 1;
  for (std::size_t index = 0; index < 32; ++index) {
    const auto& event = task_diagnostics_snapshot.platform_diagnostics[index];
    if (event.kind != cgpui::PlatformDiagnosticKind::scheduling ||
        event.operation != "task-completed" || !event.succeeded ||
        event.sequence != static_cast<int>(index + 33) ||
        event.value_count != index + 33)
      return 2;
  }
  return 0;
}
