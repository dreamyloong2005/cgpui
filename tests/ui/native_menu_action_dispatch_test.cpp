#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* fixture = nullptr;

void emit_native_menu_command() {
  fixture->window.callback(cgpui::NativeMenuCommand{
      .command_id = 0x1000U,
      .action_name = "file.open",
      .source = cgpui::NativeMenuCommandSource::accelerator,
  });
}

} // namespace

int main() {
  RuntimeFixture runtime_fixture;
  fixture = &runtime_fixture;
  runtime_fixture.app.on_run = &emit_native_menu_command;
  cgpui::WindowRuntime runtime(
      runtime_fixture.app,
      runtime_fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&runtime_fixture.renderer};
      });

  int dispatch_count = 0;
  runtime.register_window_action(
      "file.open",
      [&](const cgpui::WindowRuntimeContext&) {
        dispatch_count += 1;
        return cgpui::EventResult::consumed_event();
      });
  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  fixture = nullptr;
  const auto dispatch = runtime.last_action_dispatch();
  if (run_result != 0 || dispatch_count != 1 || !dispatch.has_value() ||
      !dispatch->handled || dispatch->name != "file.open" ||
      dispatch->scope != cgpui::ActionScope::window) return 1;
  return 0;
}
