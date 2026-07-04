#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::apply_keyboard_bindings_for_event(
    const PlatformEvent& event) {
  const auto* key = std::get_if<KeyboardKey>(&event);
  if (key == nullptr) {
    return;
  }

  if (is_focus_traversal_key(*key) &&
      focus_next_element(key->modifiers.shift) &&
      current_event_route_.has_value()) {
    current_event_route_->target_element_id = keyboard_focus_element_owner_;
    refresh_route_ancestry(*current_event_route_);
  }

  (void)dispatch_key_binding_for_event(*key);

  if (!keyboard_focus_element_owner_.has_value()) {
    return;
  }

  TextModel* model = focused_text_model();
  if (model == nullptr) {
    return;
  }

  for (const TextEditBinding& binding : text_edit_bindings_) {
    if (binding.key_code == key->key_code && binding.action == key->action &&
        modifiers_equal(binding.modifiers, key->modifiers)) {
      (void)model->apply_edit_action(binding.edit_action);
      break;
    }
  }
}

} // namespace cgpui
