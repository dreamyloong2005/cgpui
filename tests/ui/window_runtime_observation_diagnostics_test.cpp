#include "window_runtime_test_support.hpp"

namespace {

struct ObservationDiagnostics {
  cgpui::RuntimeDiagnosticsSnapshot after_register{};
  cgpui::RuntimeDiagnosticsSnapshot after_release{};
  int window_observer_count = 0;
  int view_observer_count = 0;
  bool window_subscription_release = false;
  bool view_subscription_release = false;
};

RuntimeFixture* observation_diagnostics_fixture = nullptr;

void dispatch_observation_diagnostics_sequence() {
  auto& callback = observation_diagnostics_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 79,
      .action = cgpui::KeyAction::pressed});
}

class ObservationDiagnosticsView final : public cgpui::View {
 public:
  explicit ObservationDiagnosticsView(ObservationDiagnostics& diagnostics)
      : diagnostics_(diagnostics) {}

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    (void)context.observe_window(
        [this](
            const cgpui::WindowRuntimeContext&,
            cgpui::WindowContextCapability) {
          diagnostics_.window_observer_count += 1;
        });
    window_subscription_ = context.observe_window_subscription(
        [this](
            const cgpui::WindowRuntimeContext&,
            cgpui::WindowContextCapability) {
          diagnostics_.window_observer_count += 1;
        });

    const cgpui::ViewHandle<ObservationDiagnosticsView> view =
        context.view<ObservationDiagnosticsView>();
    (void)context.observe_view(
        view,
        [this](
            const cgpui::WindowRuntimeContext&,
            cgpui::ViewHandle<ObservationDiagnosticsView>) {
          diagnostics_.view_observer_count += 1;
        });
    view_subscription_ = context.observe_view_subscription(
        view,
        [this](
            const cgpui::WindowRuntimeContext&,
            cgpui::ViewHandle<ObservationDiagnosticsView>) {
          diagnostics_.view_observer_count += 1;
        });

    diagnostics_.after_register = context.diagnostics_snapshot();

    context.request_paint();

    diagnostics_.window_subscription_release = window_subscription_.release();
    diagnostics_.view_subscription_release = view_subscription_.release();
    diagnostics_.after_release = context.diagnostics_snapshot();

    return cgpui::EventResult::consumed_event();
  }

 private:
  ObservationDiagnostics& diagnostics_;
  cgpui::Subscription window_subscription_;
  cgpui::Subscription view_subscription_;
};

int test_window_and_view_observer_counts_are_diagnostic() {
  RuntimeFixture fixture;
  ObservationDiagnostics diagnostics;
  ObservationDiagnosticsView view(diagnostics);
  observation_diagnostics_fixture = &fixture;
  fixture.app.on_run = &dispatch_observation_diagnostics_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  observation_diagnostics_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (diagnostics.after_register.entity_observer_count != 0 ||
      diagnostics.after_register.window_observer_count != 2 ||
      diagnostics.after_register.view_observer_count != 2 ||
      diagnostics.after_register.connected_subscription_count != 2) {
    return 2;
  }
  if (diagnostics.window_observer_count != 2 ||
      diagnostics.view_observer_count != 2) {
    return 3;
  }
  if (!diagnostics.window_subscription_release ||
      !diagnostics.view_subscription_release) {
    return 4;
  }
  if (diagnostics.after_release.entity_observer_count != 0 ||
      diagnostics.after_release.window_observer_count != 1 ||
      diagnostics.after_release.view_observer_count != 1 ||
      diagnostics.after_release.connected_subscription_count != 0) {
    return 5;
  }

  return 0;
}

} // namespace

int main() {
  return test_window_and_view_observer_counts_are_diagnostic();
}
