#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* timer_diagnostics_fixture = nullptr;
cgpui::WindowRuntime* timer_diagnostics_runtime = nullptr;
cgpui::RuntimeDiagnosticsSnapshot timer_diagnostics_snapshot;
int fired_timer_count = 0;

void dispatch_timer_diagnostics_stress() {
  for (int index = 0; index < 64; ++index) {
    const cgpui::TimerId timer = timer_diagnostics_runtime->schedule_timer(
        0,
        [](const cgpui::WindowRuntimeContext&) { ++fired_timer_count; });
    if (timer.value != static_cast<std::uint64_t>(index + 1)) return;
  }
  if (timer_diagnostics_fixture->app.request_wakeup_count != 64 ||
      !timer_diagnostics_fixture->app.wakeup_pending)
    return;
  timer_diagnostics_fixture->app.dispatch_wakeup();
  timer_diagnostics_snapshot = timer_diagnostics_runtime->diagnostics_snapshot();
}

} // namespace

int main() {
  RuntimeFixture fixture;
  timer_diagnostics_fixture = &fixture;
  timer_diagnostics_runtime = nullptr;
  fired_timer_count = 0;
  fixture.app.on_run = &dispatch_timer_diagnostics_stress;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  timer_diagnostics_runtime = &runtime;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  timer_diagnostics_fixture = nullptr;
  timer_diagnostics_runtime = nullptr;
  if (result != 0 || fired_timer_count != 64 || fixture.app.wakeup_pending ||
      timer_diagnostics_snapshot.platform_diagnostics.size() != 32)
    return 1;
  for (std::size_t index = 0; index < 32; ++index) {
    const auto& event = timer_diagnostics_snapshot.platform_diagnostics[index];
    if (event.kind != cgpui::PlatformDiagnosticKind::scheduling ||
        event.operation != "timer-fired" || !event.succeeded ||
        event.sequence != static_cast<int>(index + 33) ||
        event.value_count != index + 33)
      return 2;
  }
  return 0;
}
