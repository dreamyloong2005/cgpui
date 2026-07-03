#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::set_app_theme(Theme theme) {
  app_theme_ = std::move(theme);
  request_render();
}

const Theme& WindowRuntime::app_theme() const {
  return app_theme_;
}

void WindowRuntime::set_window_theme(
    WindowRuntimeId runtime_id,
    Theme theme) {
  if (runtime_id.value == 0) {
    return;
  }
  window_themes_[runtime_id.value] = std::move(theme);
  request_render();
}

bool WindowRuntime::clear_window_theme(WindowRuntimeId runtime_id) {
  if (runtime_id.value == 0) {
    return false;
  }
  const bool erased = window_themes_.erase(runtime_id.value) != 0;
  if (erased) {
    request_render();
  }
  return erased;
}

const Theme* WindowRuntime::window_theme(WindowRuntimeId runtime_id) const {
  if (runtime_id.value == 0) {
    return nullptr;
  }
  const auto theme = window_themes_.find(runtime_id.value);
  if (theme == window_themes_.end()) {
    return nullptr;
  }
  return &theme->second;
}

std::optional<Color> WindowRuntime::theme_color(
    WindowRuntimeId runtime_id,
    const ThemeTokenId& id) const {
  if (const Theme* theme = window_theme(runtime_id); theme != nullptr) {
    if (std::optional<Color> color = theme->color(id); color.has_value()) {
      return color;
    }
  }
  return app_theme_.color(id);
}

std::optional<float> WindowRuntime::theme_spacing(
    WindowRuntimeId runtime_id,
    const ThemeTokenId& id) const {
  if (const Theme* theme = window_theme(runtime_id); theme != nullptr) {
    if (std::optional<float> spacing = theme->spacing(id);
        spacing.has_value()) {
      return spacing;
    }
  }
  return app_theme_.spacing(id);
}

} // namespace cgpui
