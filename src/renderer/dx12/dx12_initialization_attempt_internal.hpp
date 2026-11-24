#pragma once
#include "../renderer_initialization_attempt_internal.hpp"

namespace cgpui {
class Dx12FramePacingApi;
class Dx12SynchronizationApi;
detail::RendererInitializationAttempt dx12_attempt_renderer_initialization(
    const RenderSurfaceDescriptor&);
detail::RendererInitializationAttempt dx12_attempt_renderer_initialization_with_api(
    const RenderSurfaceDescriptor&, Dx12FramePacingApi&, Dx12SynchronizationApi* = nullptr);
} // namespace cgpui
