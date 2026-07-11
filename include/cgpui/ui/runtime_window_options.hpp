#pragma once

#include "cgpui/ui/runtime_callbacks.hpp"
#include "cgpui/ui/runtime_ids.hpp"
#include "cgpui/ui/runtime_input_state.hpp"

#include <memory>
#include <optional>
#include <string>

namespace cgpui {

struct WindowRuntimeOptions {
  bool request_initial_redraw = true;
};

struct WindowOptions {
  WindowDescriptor descriptor;

  WindowOptions& title(std::string title);
  WindowOptions& size(Size size);
  WindowOptions& size(float width, float height);
  WindowOptions& titlebar_visible(bool visible);
  WindowOptions& decorations(bool enabled);
  WindowOptions& resizable(bool enabled);
  WindowOptions& transparent(bool enabled);
  [[nodiscard]] WindowDescriptor to_descriptor() const;
};

struct AppOpenedWindow {
  WindowRuntimeId runtime_id;
  WindowRuntimeId parent_runtime_id;
  WindowDescriptor descriptor;
  ViewId root_view_id;
};

struct WindowRuntimeRecord {
  WindowRuntimeId runtime_id;
  WindowRuntimeId parent_runtime_id;
  WindowDescriptor descriptor;
  ViewId root_view_id;
  Size framebuffer_size{};
  Size viewport_size{};
  DpiScale scale{};
  bool redraw_scheduled = false;
  ViewInputState input{};
  PlatformWindow* window = nullptr;
  Renderer* renderer = nullptr;
  bool owns_window = false;
  bool owns_renderer = false;
  bool owns_root_view = false;
  bool active = false;
  std::optional<Error> native_window_error;
};

} // namespace cgpui
