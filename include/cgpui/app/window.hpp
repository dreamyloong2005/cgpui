#pragma once

#include "cgpui/ui/runtime_input_state.hpp"
#include "cgpui/ui/runtime_window_options.hpp"

namespace cgpui {

class WindowRuntime;

class Window {
 public:
  Window(WindowRuntime& runtime, WindowRuntimeId runtime_id);

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
  [[nodiscard]] const WindowRuntimeRecord* record() const;

  WindowRuntime* runtime_ = nullptr;
  WindowRuntimeId runtime_id_{};
};

} // namespace cgpui
