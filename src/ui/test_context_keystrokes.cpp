#include "cgpui/ui/test_context.hpp"

#include "cgpui/ui/key_binding.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {
namespace {

constexpr char test_context_action_name[] = "__cgpui_test_context_keystroke__";

KeyboardKey key_from_chord(const KeyBindingChord& chord) {
  return KeyboardKey{
      .key_code = chord.key_code,
      .action = chord.action,
      .modifiers = chord.modifiers};
}

} // namespace

void TestContextCapability::dispatch_keystroke(KeyboardKey key) const {
  context_->runtime.handle_event(PlatformEvent{key});
}

bool TestContextCapability::simulate_keystrokes(
    std::string_view keystrokes) const {
  const std::optional<KeyBinding> parsed =
      parse_key_binding(keystrokes, test_context_action_name);
  if (!parsed.has_value()) {
    return false;
  }

  if (parsed->sequence.empty()) {
    dispatch_keystroke(KeyboardKey{
        .key_code = parsed->key_code,
        .action = parsed->action,
        .modifiers = parsed->modifiers});
    return true;
  }

  for (const KeyBindingChord& chord : parsed->sequence) {
    dispatch_keystroke(key_from_chord(chord));
  }
  return true;
}

} // namespace cgpui
