#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::bind_key(KeyBinding binding) {
  if (!binding.action_name.empty()) {
    key_bindings_.push_back(std::move(binding));
  }
}

void WindowRuntime::bind_text_edit_action(TextEditBinding binding) {
  text_edit_bindings_.push_back(binding);
}

} // namespace cgpui
