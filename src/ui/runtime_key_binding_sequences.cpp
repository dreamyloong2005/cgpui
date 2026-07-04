#include "ui_internal.hpp"

namespace cgpui {
namespace {

int key_binding_context_rank(KeyBindingContextKind kind) {
  switch (kind) {
    case KeyBindingContextKind::focused_element:
      return 3;
    case KeyBindingContextKind::view:
      return 2;
    case KeyBindingContextKind::window:
      return 1;
    case KeyBindingContextKind::app:
      return 0;
  }
  return 0;
}

KeyBindingChord chord_from_key(const KeyboardKey& key) {
  return KeyBindingChord{
      .key_code = key.key_code,
      .action = key.action,
      .modifiers = key.modifiers};
}

KeyBindingChord binding_chord_at(
    const KeyBinding& binding,
    std::size_t index) {
  if (!binding.sequence.empty()) {
    return binding.sequence[index];
  }
  return KeyBindingChord{
      .key_code = binding.key_code,
      .action = binding.action,
      .modifiers = binding.modifiers};
}

std::size_t key_binding_sequence_size(const KeyBinding& binding) {
  return binding.sequence.empty() ? 1 : binding.sequence.size();
}

bool key_binding_chords_equal(
    const KeyBindingChord& lhs,
    const KeyBindingChord& rhs) {
  return lhs.key_code == rhs.key_code && lhs.action == rhs.action &&
         modifiers_equal(lhs.modifiers, rhs.modifiers);
}

bool key_binding_sequence_has_prefix(
    const KeyBinding& binding,
    const std::vector<KeyBindingChord>& prefix) {
  const std::size_t binding_size = key_binding_sequence_size(binding);
  if (prefix.empty() || prefix.size() > binding_size) {
    return false;
  }

  for (std::size_t index = 0; index < prefix.size(); ++index) {
    if (!key_binding_chords_equal(binding_chord_at(binding, index),
                                  prefix[index])) {
      return false;
    }
  }
  return true;
}

} // namespace

bool WindowRuntime::dispatch_key_binding_for_event(const KeyboardKey& key) {
  std::vector<KeyBindingChord> sequence = pending_key_binding_sequence_;
  sequence.push_back(chord_from_key(key));

  const KeyBinding* selected = nullptr;
  int selected_rank = -1;
  bool has_partial_match = false;

  for (const KeyBinding& binding : key_bindings_) {
    if (!key_binding_context_active(binding.context) ||
        !key_binding_sequence_has_prefix(binding, sequence)) {
      continue;
    }

    const std::size_t binding_size = key_binding_sequence_size(binding);
    if (binding_size > sequence.size()) {
      has_partial_match = true;
      continue;
    }

    const int rank = key_binding_context_rank(binding.context.kind);
    if (selected == nullptr || rank > selected_rank) {
      selected = &binding;
      selected_rank = rank;
    }
  }

  if (has_partial_match) {
    pending_key_binding_sequence_ = std::move(sequence);
    return true;
  }

  pending_key_binding_sequence_.clear();
  if (selected == nullptr) {
    return false;
  }

  (void)dispatch_action(selected->action_name);
  return true;
}

} // namespace cgpui
