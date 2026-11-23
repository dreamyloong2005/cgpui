#pragma once

#include "cgpui/renderer/renderer_frame.hpp"

namespace cgpui {

// Named factories bypass CGPUI_RENDERER and never select another backend.
#if defined(_WIN32) || defined(__linux__)
Result<std::unique_ptr<Renderer>> create_vulkan_renderer(
    const RenderSurfaceDescriptor& descriptor);
#endif

#if defined(_WIN32)
// Experimental backend for opaque HWNDs and native alpha-composited Popups;
// unsupported features (including transparent ordinary HWNDs) fail explicitly.
Result<std::unique_ptr<Renderer>> create_dx12_renderer(
    const RenderSurfaceDescriptor& descriptor);
#endif

} // namespace cgpui
