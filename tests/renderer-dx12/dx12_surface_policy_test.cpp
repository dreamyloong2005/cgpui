#include "../../src/renderer/dx12/dx12_surface_policy_internal.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <iostream>

int main() {
  using namespace cgpui;
  const Dx12SurfacePolicy flip{};
  const Dx12SurfacePolicy sticky{Dx12SurfaceRoute::composition, false};
  const Dx12SurfacePolicy dynamic{Dx12SurfaceRoute::composition, true};
  if (dx12_composition_route(flip) || !dx12_composition_route(dynamic) ||
      dx12_surface_alpha_change(flip, false, true) != Dx12SurfaceAlphaChange::unsupported ||
      dx12_surface_alpha_change(flip, false, false) != Dx12SurfaceAlphaChange::intent_only ||
      dx12_surface_alpha_change(sticky, true, false) != Dx12SurfaceAlphaChange::intent_only ||
      dx12_surface_alpha_change(dynamic, false, true) != Dx12SurfaceAlphaChange::replace_chain ||
      dx12_surface_alpha_change(dynamic, true, false) != Dx12SurfaceAlphaChange::replace_chain ||
      dx12_surface_alpha_change(dynamic, true, true) != Dx12SurfaceAlphaChange::intent_only) return 1;
  Dx12State state;
  state.surface_policy = dynamic;
  const auto opaque = dx12_surface_clear(state, {1, .5F, .25F, 0});
  if (opaque.r != 1 || opaque.g != .5F || opaque.b != .25F || opaque.a != 0 ||
      dx12_surface_composition(state).composite_alpha != RendererCompositeAlpha::opaque) return 2;
  state.composition_surface = true;
  const auto alpha = dx12_surface_clear(state, {1, .5F, .25F, .5F});
  if (alpha.r != .5F || alpha.g != .25F || alpha.b != .125F || alpha.a != .5F ||
      dx12_surface_composition(state).composite_alpha != RendererCompositeAlpha::pre_multiplied) return 3;
  std::cout << "surface route and actual alpha are independent; no native device created\n";
  return 0;
}
