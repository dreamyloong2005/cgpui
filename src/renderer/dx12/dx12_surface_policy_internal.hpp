#pragma once

namespace cgpui {
enum class Dx12SurfaceRoute { hwnd_flip, composition };
struct Dx12SurfacePolicy {
  Dx12SurfaceRoute route = Dx12SurfaceRoute::hwnd_flip;
  bool dynamic_ordinary = false;
};
enum class Dx12SurfaceAlphaChange { intent_only, replace_chain, unsupported };
constexpr bool dx12_composition_route(const Dx12SurfacePolicy& policy) noexcept {
  return policy.route == Dx12SurfaceRoute::composition;
}
Dx12SurfaceAlphaChange dx12_surface_alpha_change(
    const Dx12SurfacePolicy&, bool premultiplied, bool requested) noexcept;
} // namespace cgpui
