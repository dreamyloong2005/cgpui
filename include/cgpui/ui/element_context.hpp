#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/runtime_input_state.hpp"

namespace cgpui {

struct WindowRuntimeContext;

class ElementContextCapability {
 public:
  constexpr ElementContextCapability() = default;
  constexpr ElementContextCapability(
      const WindowRuntimeContext& context,
      ElementId element_id)
      : context_(&context), element_id_(element_id) {}

  [[nodiscard]] ElementId element_id() const;

  void request_keyboard_focus() const;
  void release_keyboard_focus() const;
  void focus() const;
  void blur() const;
  [[nodiscard]] FocusHandle focus_handle() const;
  void capture_pointer() const;
  void release_pointer() const;
  void set_cursor(CursorShape cursor_shape) const;

  template <typename T>
  [[nodiscard]] T* state() const;
  template <typename T, typename... Args>
  [[nodiscard]] T* emplace_state(Args&&... args) const;
  template <typename T, typename... Args>
  [[nodiscard]] T* state_or_init(Args&&... args) const;

 private:
  const WindowRuntimeContext* context_ = nullptr;
  ElementId element_id_{};
};

} // namespace cgpui
