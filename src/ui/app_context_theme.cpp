#include "ui_internal.hpp"

namespace cgpui {

void AppContext::set_app_theme(Theme theme) const {
  runtime.set_app_theme(std::move(theme));
}

void AppContext::set_window_theme(
    WindowRuntimeId runtime_id,
    Theme theme) const {
  runtime.set_window_theme(runtime_id, std::move(theme));
}

bool AppContext::clear_window_theme(WindowRuntimeId runtime_id) const {
  return runtime.clear_window_theme(runtime_id);
}

std::optional<Color> AppContext::theme_color(
    WindowRuntimeId runtime_id,
    const ThemeTokenId& id) const {
  return runtime.theme_color(runtime_id, id);
}

std::optional<float> AppContext::theme_spacing(
    WindowRuntimeId runtime_id,
    const ThemeTokenId& id) const {
  return runtime.theme_spacing(runtime_id, id);
}

} // namespace cgpui
