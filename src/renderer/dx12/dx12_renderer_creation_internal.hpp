#pragma once

#include "cgpui/renderer/renderer.hpp"

namespace cgpui {
class Dx12FramePacingApi;
class Dx12SynchronizationApi;

// Private creation seam; still returns the real Dx12Renderer. The borrowed API
// must outlive the renderer AND every outstanding frame sharing its state,
// including partial initialization cleanup and any later recovery/cohort reset.
// A non-null synchronization API has the same borrowed lifetime requirement.
// The public factory always uses native pacing and synchronization.
Result<std::unique_ptr<Renderer>> dx12_create_renderer(
    const RenderSurfaceDescriptor& descriptor, Dx12FramePacingApi& pacing_api,
    Dx12SynchronizationApi* synchronization_api = nullptr);
} // namespace cgpui
