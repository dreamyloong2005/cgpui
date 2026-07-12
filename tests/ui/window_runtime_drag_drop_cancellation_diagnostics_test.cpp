#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* drag_diagnostics_fixture = nullptr;
cgpui::WindowRuntime* drag_diagnostics_runtime = nullptr;
cgpui::RuntimeDiagnosticsSnapshot drag_diagnostics_snapshot;

void dispatch_drag_cancellation_stress() {
  for (int index = 0; index < 64; ++index) {
    const cgpui::DragDropPayload payload{
        .kind = cgpui::DragDropPayloadKind::text,
        .text = "drag " + std::to_string(index),
    };
    drag_diagnostics_fixture->window.callback(cgpui::DragEntered{
        .position = {static_cast<float>(index), 1.0F},
        .payload = payload,
        .action = cgpui::DragDropAction::copy,
    });
    drag_diagnostics_fixture->window.callback(cgpui::DragExited{
        .position = {static_cast<float>(index), 1.0F},
        .payload = payload,
        .action = cgpui::DragDropAction::copy,
    });
  }
  drag_diagnostics_snapshot = drag_diagnostics_runtime->diagnostics_snapshot();
}

} // namespace

int main() {
  RuntimeFixture fixture;
  drag_diagnostics_fixture = &fixture;
  fixture.app.on_run = &dispatch_drag_cancellation_stress;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  drag_diagnostics_runtime = &runtime;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  drag_diagnostics_fixture = nullptr;
  drag_diagnostics_runtime = nullptr;
  if (result != 0 || drag_diagnostics_snapshot.platform_diagnostics.size() != 32)
    return 1;
  for (std::size_t index = 0; index < 32; ++index) {
    const auto& event = drag_diagnostics_snapshot.platform_diagnostics[index];
    const bool cancellation = (index % 2U) != 0;
    if (event.kind != cgpui::PlatformDiagnosticKind::drag_drop ||
        event.sequence != static_cast<int>(97 + index) ||
        event.value_count != 1 || event.cancelled != cancellation ||
        event.succeeded == cancellation ||
        event.operation != (cancellation ? "drag-exited" : "drag-entered"))
      return 2;
  }
  return 0;
}
