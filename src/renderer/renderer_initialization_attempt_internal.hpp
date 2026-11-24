#pragma once
#include "cgpui/renderer/renderer_frame.hpp"

namespace cgpui::detail {
// Private construction result. No runtime operation can manufacture this proof.
// Native owners may grant fallback only after checked retirement of all HWND
// claims; a Result<Renderer> error/destructor alone never grants permission.
struct RendererInitializationFailure {
  Error error;
  bool same_surface_reusable = false;
};
using RendererInitializationAttempt =
    std::expected<std::unique_ptr<Renderer>, RendererInitializationFailure>;
using RendererAttemptFactory = RendererInitializationAttempt (*)(const RenderSurfaceDescriptor&);
} // namespace cgpui::detail
