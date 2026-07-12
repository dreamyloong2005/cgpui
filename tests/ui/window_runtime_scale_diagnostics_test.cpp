#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* scale_diagnostics_fixture = nullptr;
cgpui::WindowRuntime* scale_diagnostics_runtime = nullptr;
cgpui::RuntimeDiagnosticsSnapshot scale_diagnostics_snapshot;

void dispatch_scale_diagnostics_stress() {
  for (int index = 0; index < 64; ++index) {
    scale_diagnostics_fixture->window.dispatch_resize(
        cgpui::Size{.width = 640.0F + static_cast<float>(index),
                    .height = 480.0F + static_cast<float>(index)},
        cgpui::DpiScale{.value = 1.0F + 0.25F * static_cast<float>(index % 4)});
  }
  scale_diagnostics_snapshot = scale_diagnostics_runtime->diagnostics_snapshot();
}

} // namespace

int main() {
  RuntimeFixture fixture;
  scale_diagnostics_fixture = &fixture;
  fixture.app.on_run = &dispatch_scale_diagnostics_stress;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  scale_diagnostics_runtime = &runtime;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  scale_diagnostics_fixture = nullptr;
  scale_diagnostics_runtime = nullptr;
  if (result != 0 || fixture.renderer.resize_count != 64 ||
      scale_diagnostics_snapshot.platform_diagnostics.size() != 32)
    return 1;
  for (std::size_t index = 0; index < 32; ++index) {
    const int source_index = static_cast<int>(index) + 32;
    const auto& event = scale_diagnostics_snapshot.platform_diagnostics[index];
    if (event.kind != cgpui::PlatformDiagnosticKind::window_scale ||
        event.event_kind != cgpui::EventKind::window_resized ||
        event.operation != "resize-scale" || !event.succeeded ||
        event.sequence != source_index + 1 ||
        event.framebuffer_size.width != 640.0F + source_index ||
        event.framebuffer_size.height != 480.0F + source_index ||
        event.scale.value != 1.0F + 0.25F * static_cast<float>(source_index % 4))
      return 2;
  }
  return 0;
}
