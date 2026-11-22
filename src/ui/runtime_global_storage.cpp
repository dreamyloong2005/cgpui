#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::set_global_value(std::type_index type, std::any value) {
  state_->entities.globals_[type] = std::move(value);
  request_render();
}

std::any* WindowRuntime::global_value(std::type_index type) {
  const auto entry = state_->entities.globals_.find(type);
  return entry == state_->entities.globals_.end() ? nullptr : &entry->second;
}

const std::any* WindowRuntime::global_value(std::type_index type) const {
  const auto entry = state_->entities.globals_.find(type);
  return entry == state_->entities.globals_.end() ? nullptr : &entry->second;
}

} // namespace cgpui
