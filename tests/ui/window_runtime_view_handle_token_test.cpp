#include "window_runtime_test_support.hpp"

namespace {

class ViewHandleTokenView;

struct ViewHandleTokenState {
  cgpui::ViewHandle<ViewHandleTokenView> source_handle;
  cgpui::WeakViewHandle<ViewHandleTokenView> source_weak;
  bool captured_source_handle = false;
  bool cross_runtime_read_blocked = false;
  bool cross_runtime_observe_blocked = false;
  bool cross_runtime_subscription_blocked = false;
  bool cross_runtime_weak_upgrade_blocked = false;
};

class ViewHandleTokenView final : public cgpui::View {
 public:
  ViewHandleTokenView(ViewHandleTokenState& state, bool source)
      : state_(state), source_(source) {}

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    if (source_) {
      state_.source_handle = context.view<ViewHandleTokenView>();
      state_.source_weak = context.weak_view<ViewHandleTokenView>();
      state_.captured_source_handle = true;
      return cgpui::EventResult::consumed_event();
    }

    const ViewHandleTokenView* cross_runtime_read =
        state_.source_handle.read(context);
    state_.cross_runtime_read_blocked = cross_runtime_read == nullptr;
    state_.cross_runtime_observe_blocked =
        !state_.source_handle.observe(
            context,
            [](const cgpui::WindowRuntimeContext&,
               cgpui::ViewHandle<ViewHandleTokenView>) {});
    cgpui::Subscription cross_runtime_subscription =
        state_.source_handle.observe_subscription(
            context,
            [](const cgpui::WindowRuntimeContext&,
               cgpui::ViewHandle<ViewHandleTokenView>) {});
    state_.cross_runtime_subscription_blocked =
        !cross_runtime_subscription.connected();
    state_.cross_runtime_weak_upgrade_blocked =
        !context.upgrade_view(state_.source_weak).has_value();

    return cgpui::EventResult::consumed_event();
  }

 private:
  ViewHandleTokenState& state_;
  bool source_ = false;
};

RuntimeFixture* source_fixture = nullptr;
RuntimeFixture* target_fixture = nullptr;

void dispatch_source_sequence() {
  auto& callback = source_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 82,
      .action = cgpui::KeyAction::pressed});
}

void dispatch_target_sequence() {
  auto& callback = target_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed});
}

int test_view_handles_reject_cross_runtime_contexts() {
  ViewHandleTokenState state;
  RuntimeFixture first_fixture;
  RuntimeFixture second_fixture;
  ViewHandleTokenView first_view(state, true);
  ViewHandleTokenView second_view(state, false);

  source_fixture = &first_fixture;
  first_fixture.app.on_run = &dispatch_source_sequence;
  cgpui::WindowRuntime first_runtime(
      first_fixture.app,
      first_view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&first_fixture.renderer};
      });
  const int first_result =
      first_runtime.run(cgpui::WindowDescriptor{},
                        cgpui::WindowRuntimeOptions{
                            .request_initial_redraw = false});
  source_fixture = nullptr;

  if (first_result != 0 || !state.captured_source_handle) {
    return 1;
  }

  target_fixture = &second_fixture;
  second_fixture.app.on_run = &dispatch_target_sequence;
  cgpui::WindowRuntime second_runtime(
      second_fixture.app,
      second_view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&second_fixture.renderer};
      });
  const int second_result =
      second_runtime.run(cgpui::WindowDescriptor{},
                         cgpui::WindowRuntimeOptions{
                             .request_initial_redraw = false});
  target_fixture = nullptr;

  if (second_result != 0) {
    return 2;
  }
  if (!state.cross_runtime_read_blocked) {
    return 3;
  }
  if (!state.cross_runtime_observe_blocked) {
    return 4;
  }
  if (!state.cross_runtime_subscription_blocked) {
    return 5;
  }
  if (!state.cross_runtime_weak_upgrade_blocked) {
    return 6;
  }

  return 0;
}

} // namespace

int main() {
  return test_view_handles_reject_cross_runtime_contexts();
}
