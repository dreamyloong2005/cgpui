#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* fixture = nullptr;

void emit_native_menu_commands() {
  fixture->window.callback(cgpui::NativeMenuCommand{
      .command_id = 0x1000U,
      .action_name = "file.open",
      .source = cgpui::NativeMenuCommandSource::accelerator,
  });
  fixture->window.callback(cgpui::NativeMenuCommand{
      .command_id = 0x1001U,
      .action_name = "file.missing",
      .source = cgpui::NativeMenuCommandSource::menu,
  });
}

} // namespace

int main() {
  RuntimeFixture runtime_fixture;
  fixture = &runtime_fixture;
  runtime_fixture.app.on_run = &emit_native_menu_commands;
  cgpui::WindowRuntime runtime(
      runtime_fixture.app,
      runtime_fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&runtime_fixture.renderer};
      });
  runtime.register_window_action(
      "file.open",
      [](const cgpui::WindowRuntimeContext&) {
        return cgpui::EventResult::consumed_event();
      });
  const int run_result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  fixture = nullptr;
  if (run_result != 0) return 1;

  const auto diagnostics = runtime.platform_diagnostics();
  if (diagnostics.size() != 2 ||
      diagnostics[0].kind != cgpui::PlatformDiagnosticKind::menu ||
      diagnostics[0].event_kind != cgpui::EventKind::native_menu_command ||
      diagnostics[0].operation != "dispatch-native-menu-accelerator" ||
      !diagnostics[0].succeeded || diagnostics[0].value_count != 0x1000U) return 2;
  if (diagnostics[1].operation != "dispatch-native-menu-command" ||
      diagnostics[1].succeeded || diagnostics[1].value_count != 0x1001U) return 3;
  return 0;
}
