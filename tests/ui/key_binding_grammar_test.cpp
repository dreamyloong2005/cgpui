#include "window_runtime_test_support.hpp"

#include "cgpui/ui/key_binding.hpp"

#include <optional>
#include <variant>

namespace {

RuntimeFixture* key_binding_grammar_fixture = nullptr;

void dispatch_key_binding_grammar_sequence() {
  auto& callback = key_binding_grammar_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 'I',
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 'S',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true, .control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 'P',
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.super = true}});
}

class KeyBindingGrammarView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
    if (key == nullptr) {
      return cgpui::EventResult::unhandled();
    }

    if (key->key_code == 'I') {
      context.register_action(
          "grammar.save",
          [this](const cgpui::WindowRuntimeContext&) {
            save_count += 1;
            return cgpui::EventResult::consumed_event();
          });
      context.register_action(
          "grammar.palette",
          [this](const cgpui::WindowRuntimeContext&) {
            palette_count += 1;
            return cgpui::EventResult::consumed_event();
          });
      save_bound = context.bind_key("ctrl-shift-s", "grammar.save");
      palette_bound = context.bind_key("cmd-p", "grammar.palette");
      invalid_bound = context.bind_key("ctrl-unknown", "grammar.invalid");
    }

    return cgpui::EventResult::unhandled();
  }

  bool save_bound = false;
  bool palette_bound = false;
  bool invalid_bound = true;
  int save_count = 0;
  int palette_count = 0;
};

int test_key_binding_parser_accepts_gpui_style_chords() {
  const std::optional<cgpui::KeyBinding> save =
      cgpui::parse_key_binding("ctrl-shift-s", "grammar.save");
  if (!save.has_value() ||
      save->key_code != 'S' ||
      save->action != cgpui::KeyAction::pressed ||
      !save->modifiers.control ||
      !save->modifiers.shift ||
      save->modifiers.alt ||
      save->modifiers.super ||
      save->action_name != "grammar.save") {
    return 1;
  }

  const std::optional<cgpui::KeyBinding> palette =
      cgpui::parse_key_binding("cmd-p", "grammar.palette");
  if (!palette.has_value() ||
      palette->key_code != 'P' ||
      palette->modifiers.control ||
      palette->modifiers.shift ||
      palette->modifiers.alt ||
      !palette->modifiers.super) {
    return 2;
  }

  const std::optional<cgpui::KeyBinding> enter =
      cgpui::parse_key_binding("alt-enter", "grammar.enter");
  if (!enter.has_value() ||
      enter->key_code != 13 ||
      !enter->modifiers.alt ||
      enter->modifiers.control ||
      enter->modifiers.shift ||
      enter->modifiers.super) {
    return 3;
  }

  if (cgpui::parse_key_binding("ctrl-unknown", "grammar.invalid").has_value() ||
      cgpui::parse_key_binding("ctrl-s", "").has_value() ||
      cgpui::parse_key_binding("", "grammar.empty").has_value()) {
    return 4;
  }

  return 0;
}

int test_context_bind_key_accepts_key_binding_grammar() {
  RuntimeFixture fixture;
  KeyBindingGrammarView view;
  key_binding_grammar_fixture = &fixture;
  fixture.app.on_run = &dispatch_key_binding_grammar_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  key_binding_grammar_fixture = nullptr;

  if (result != 0) {
    return 5;
  }
  if (!view.save_bound || !view.palette_bound || view.invalid_bound) {
    return 6;
  }
  if (view.save_count != 1 || view.palette_count != 1) {
    return 7;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_key_binding_parser_accepts_gpui_style_chords();
      result != 0) {
    return result;
  }
  return test_context_bind_key_accepts_key_binding_grammar();
}
