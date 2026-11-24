#pragma once
#include "dx12_native_internal.hpp"
#include <memory>

namespace cgpui {
struct Dx12State;
Result<void> dx12_initialize_renderer_state(const std::shared_ptr<Dx12State>&,
    const RenderSurfaceDescriptor&, bool* rollback_candidate = nullptr);
std::unique_ptr<Renderer> dx12_wrap_initialized_renderer(std::shared_ptr<Dx12State>);
} // namespace cgpui
