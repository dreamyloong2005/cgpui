#pragma once

#include "cgpui/renderer/renderer.hpp"

namespace cgpui {
class Dx12FramePacingApi;

// Private creation seam; still returns the real Dx12Renderer. The borrowed API
// must outlive the renderer AND every outstanding frame sharing its state,
// including partial initialization cleanup and any later recovery/cohort reset.
// The public factory always supplies the process-lifetime native API.
Result<std::unique_ptr<Renderer>> dx12_create_renderer(
    const RenderSurfaceDescriptor& descriptor, Dx12FramePacingApi& pacing_api);
} // namespace cgpui
