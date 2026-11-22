#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/platform_window.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/runtime_window_options.hpp"

namespace cgpui {

inline constexpr Color runtime_default_clear_color{
    .r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F};

// Applies `backdrop` with the renderer's real surface composition, switching
// the renderer to a transparent surface first when the backend can show the
// material and the surface is still opaque.
PlatformWindowBackdropState runtime_apply_window_backdrop(
    PlatformWindow& window,
    Renderer* renderer,
    WindowBackdrop backdrop);

// A window created with a backdrop asks for an alpha-composited surface up
// front so device selection can prefer a GPU whose surface supports it.
[[nodiscard]] bool runtime_wants_transparent_surface(
    const WindowDescriptor& descriptor);

// Transparent clear only while a material is visible behind the client area.
[[nodiscard]] Color runtime_window_clear_color(const PlatformWindow* window);
[[nodiscard]] Color runtime_window_clear_color(const WindowRuntimeRecord& record);

} // namespace cgpui
