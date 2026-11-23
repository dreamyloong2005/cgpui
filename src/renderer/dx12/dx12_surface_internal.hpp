#pragma once

#include "cgpui/renderer/renderer_surface_composition.hpp"
#include "cgpui/renderer/renderer_types.hpp"
#include "cgpui/core/error.hpp"

namespace cgpui {
struct Dx12State;
Result<void> dx12_configure_surface(Dx12State&, const RenderSurfaceDescriptor&);
RendererSurfaceComposition dx12_surface_composition(const Dx12State&);
Result<RendererSurfaceComposition> dx12_set_transparent_surface(Dx12State&, bool);
Color dx12_surface_clear(const Dx12State&, Color);
} // namespace cgpui
