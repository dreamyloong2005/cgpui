#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::bind_key(KeyBinding binding) {
  if (!binding.action_name.empty()) {
    state_->actions.key_bindings_.push_back(std::move(binding));
  }
}

void WindowRuntime::bind_text_edit_action(TextEditBinding binding) {
  state_->text.text_edit_bindings_.push_back(binding);
}

} // namespace cgpui
