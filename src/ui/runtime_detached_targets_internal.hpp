#pragma once

// Inert window and renderer a WindowRuntimeContext refers to while its
// runtime has no window (before run() opened one, or after it closed it):
// a task completion or an immediate animation callback may run then, and
// the context's reference members must name an object. Every request is a
// no-op; state() is a default WindowState.

#include "cgpui/platform/platform_window.hpp"
#include "cgpui/renderer/renderer.hpp"

namespace cgpui {

[[nodiscard]] PlatformWindow& detached_runtime_window();
[[nodiscard]] Renderer& detached_runtime_renderer();

} // namespace cgpui
