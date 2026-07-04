#include "window_runtime_test_support.hpp"

#include "cgpui/ui/key_binding.hpp"

#include <variant>

namespace {

RuntimeFixture* partial_key_fixture = nullptr;

void dispatch_partial_key_sequence() {
  auto& callback = partial_key_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 'I',
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 'K',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'S',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'K',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'X',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'K',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'S',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

class PartialKeyMatchView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
    if (key == nullptr || key->key_code != 'I') {
      return cgpui::EventResult::unhandled();
    }

    context.register_action(
        "partial.prefix",
        [this](const cgpui::WindowRuntimeContext&) {
          prefix_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_action(
        "partial.sequence",
        [this](const cgpui::WindowRuntimeContext&) {
          sequence_count += 1;
          return cgpui::EventResult::consumed_event();
        });

    prefix_bound = context.bind_key("ctrl-k", "partial.prefix");
    sequence_bound = context.bind_key("ctrl-k ctrl-s", "partial.sequence");
    return cgpui::EventResult::consumed_event();
  }

  bool prefix_bound = false;
  bool sequence_bound = false;
  int prefix_count = 0;
  int sequence_count = 0;
};

int test_parse_key_binding_sequence_records_chords() {
  const auto binding =
      cgpui::parse_key_binding("ctrl-k ctrl-s", "partial.sequence");
  if (!binding.has_value()) {
    return 1;
  }
  if (binding->sequence.size() != 2) {
    return 2;
  }
  if (binding->sequence[0].key_code != 'K' ||
      !binding->sequence[0].modifiers.control ||
      binding->sequence[1].key_code != 'S' ||
      !binding->sequence[1].modifiers.control) {
    return 3;
  }
  return 0;
}

int test_partial_key_match_waits_for_sequence_completion() {
  RuntimeFixture fixture;
  PartialKeyMatchView view;
  partial_key_fixture = &fixture;
  fixture.app.on_run = &dispatch_partial_key_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  partial_key_fixture = nullptr;

  if (result != 0) {
    return 10;
  }
  if (!view.prefix_bound || !view.sequence_bound) {
    return 11;
  }
  if (view.prefix_count != 0) {
    return 12;
  }
  if (view.sequence_count != 2) {
    return 13;
  }

  return 0;
}

} // namespace

int main() {
  int result = test_parse_key_binding_sequence_records_chords();
  if (result != 0) {
    return result;
  }
  return test_partial_key_match_waits_for_sequence_completion();
}
