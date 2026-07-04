#pragma once

#include "cgpui/app/window.hpp"

namespace cgpui {

struct WindowRuntimeContext;

class WindowContextCapability {
 public:
  constexpr WindowContextCapability() = default;
  explicit WindowContextCapability(const WindowRuntimeContext& context);

  [[nodiscard]] Window window() const;
  [[nodiscard]] Window current_window() const;
  [[nodiscard]] WindowRuntimeId runtime_id() const;
  [[nodiscard]] WindowDescriptor descriptor() const;
  [[nodiscard]] ViewId root_view_id() const;
  [[nodiscard]] bool active() const;
  [[nodiscard]] Size viewport_size() const;
  [[nodiscard]] DpiScale scale() const;
  [[nodiscard]] ViewInputState input_state() const;
  [[nodiscard]] bool focused() const;

  void request_render() const;
  void request_layout() const;
  void request_paint() const;

 private:
  const WindowRuntimeContext* context_ = nullptr;
};

} // namespace cgpui
