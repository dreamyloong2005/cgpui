#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* ime_diagnostics_fixture = nullptr;
cgpui::WindowRuntime* ime_diagnostics_runtime = nullptr;
cgpui::RuntimeDiagnosticsSnapshot ime_diagnostics_snapshot;

void dispatch_ime_operation_diagnostics() {
  auto& callback = ime_diagnostics_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed,
  });
  callback(cgpui::ImeComposition{
      .phase = cgpui::ImeCompositionPhase::update,
      .text = "draft",
  });
  callback(cgpui::ImeComposition{
      .phase = cgpui::ImeCompositionPhase::commit,
      .text = "x",
  });
  callback(cgpui::ImeDeleteSurroundingText{.before_length = 1});
  ime_diagnostics_snapshot = ime_diagnostics_runtime->diagnostics_snapshot();
}

void dispatch_ime_diagnostics_stress() {
  auto& callback = ime_diagnostics_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed,
  });
  for (int index = 0; index < 64; ++index) {
    callback(cgpui::ImeComposition{
        .phase = cgpui::ImeCompositionPhase::update,
        .text = "draft " + std::to_string(index),
        .serial = static_cast<std::uint32_t>(index + 1),
    });
    callback(cgpui::ImeComposition{
        .phase = cgpui::ImeCompositionPhase::cancel,
        .serial = static_cast<std::uint32_t>(index + 1),
    });
  }
  ime_diagnostics_snapshot = ime_diagnostics_runtime->diagnostics_snapshot();
}

} // namespace

int main() {
  {
    RuntimeFixture fixture;
    ime_diagnostics_fixture = &fixture;
    fixture.app.on_run = &dispatch_ime_operation_diagnostics;
    fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
    fixture.view.request_keyboard_focus_element_on_first_key = true;
    cgpui::TextModel model("ab");
    model.set_selection(2, 2);
    cgpui::WindowRuntime runtime(
        fixture.app,
        fixture.view,
        [&](const cgpui::RenderSurfaceDescriptor&) {
          return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
        });
    runtime.bind_text_model(cgpui::ElementId{21}, &model);
    ime_diagnostics_runtime = &runtime;
    const int result = runtime.run(
        cgpui::WindowDescriptor{},
        cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
    ime_diagnostics_fixture = nullptr;
    ime_diagnostics_runtime = nullptr;
    if (result != 0 || model.text() != "ab" ||
        ime_diagnostics_snapshot.platform_diagnostics.size() != 3)
      return 1;
    const auto& update = ime_diagnostics_snapshot.platform_diagnostics[0];
    const auto& commit = ime_diagnostics_snapshot.platform_diagnostics[1];
    const auto& deletion = ime_diagnostics_snapshot.platform_diagnostics[2];
    if (update.operation != "composition-update" || update.value_count != 5 ||
        commit.operation != "composition-commit" || commit.value_count != 1 ||
        deletion.operation != "delete-surrounding" ||
        deletion.event_kind !=
            cgpui::EventKind::ime_delete_surrounding_text ||
        deletion.value_count != 1 || !deletion.succeeded)
      return 2;
  }

  RuntimeFixture fixture;
  ime_diagnostics_fixture = &fixture;
  fixture.app.on_run = &dispatch_ime_diagnostics_stress;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;
  cgpui::TextModel model;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  ime_diagnostics_runtime = &runtime;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  ime_diagnostics_fixture = nullptr;
  ime_diagnostics_runtime = nullptr;
  if (result != 0 || model.has_composition() ||
      ime_diagnostics_snapshot.platform_diagnostics.size() != 32)
    return 3;
  for (std::size_t index = 0; index < 32; ++index) {
    const auto& event = ime_diagnostics_snapshot.platform_diagnostics[index];
    const bool cancellation = (index % 2U) != 0;
    if (event.kind != cgpui::PlatformDiagnosticKind::ime ||
        event.event_kind != cgpui::EventKind::ime_composition ||
        event.sequence != static_cast<int>(97 + index) ||
        event.cancelled != cancellation || event.succeeded == cancellation ||
        event.operation !=
            (cancellation ? "composition-cancel" : "composition-update") ||
        event.value_count !=
            (cancellation ? 0U : std::string{"draft 00"}.size()))
      return 4;
  }
  return 0;
}
