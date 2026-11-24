#include "dx12_surface_internal.hpp"
#include "dx12_frame_command_reuse_internal.hpp"
#include "dx12_command_reuse_surface_epoch_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_window_alpha_internal.hpp"
#include "dx12_surface_transition_internal.hpp"
#include <algorithm>

namespace cgpui {
Result<void> dx12_configure_surface(Dx12State& state, const RenderSurfaceDescriptor& descriptor) {
  if (auto valid = dx12_validate_surface_descriptor(descriptor); !valid) return valid;
  const auto* native = std::get_if<Win32SurfaceHandle>(&descriptor.native_surface);
  const auto hwnd = static_cast<HWND>(native->hwnd);
  state.hwnd = hwnd;
  state.composition_surface = native->composition;
  state.surface_policy.route = native->composition ? Dx12SurfaceRoute::composition : Dx12SurfaceRoute::hwnd_flip;
  state.transparent_background = descriptor.transparent_background;
  state.width = static_cast<UINT>(descriptor.framebuffer_size.width);
  state.height = static_cast<UINT>(descriptor.framebuffer_size.height);
  state.scale = descriptor.scale;
  if (!native->composition && descriptor.transparent_background) {
    if (auto lease = dx12_acquire_window_alpha(state); !lease) return lease;
    state.composition_surface = true;
    state.surface_policy.route = Dx12SurfaceRoute::composition;
  } else if (!native->composition) {
    auto reserved = dx12_reserve_ordinary_composition(state);
    if (!reserved) return std::unexpected(reserved.error());
    if (*reserved) state.surface_policy = {Dx12SurfaceRoute::composition, true};
  }
  return {};
}

RendererSurfaceComposition dx12_surface_composition(const Dx12State& state) {
  return {.supported = true, .transparent_requested = state.transparent_background,
          .composite_alpha = state.composition_surface ? RendererCompositeAlpha::pre_multiplied
                                                      : RendererCompositeAlpha::opaque};
}

Result<RendererSurfaceComposition> dx12_set_transparent_surface(Dx12State& state, bool transparent) {
  if (auto valid = dx12_validate_window_alpha(state); !valid) return std::unexpected(valid.error());
  const auto change = dx12_surface_alpha_change(state.surface_policy, state.composition_surface, transparent);
  if (change == Dx12SurfaceAlphaChange::unsupported)
    return std::unexpected(dx12_unsupported("transparent ordinary HWND presentation"));
  if (change == Dx12SurfaceAlphaChange::replace_chain) {
    if (auto replaced = dx12_replace_surface_alpha(state, transparent); !replaced)
      return std::unexpected(replaced.error());
  }
  // Initial-alpha ordinary and native Popup retain their sticky PREMULT route.
  state.transparent_background = transparent;
  state.command_reuse->accept_surface({state.width, state.height, state.scale.value, state.composition_surface, transparent});
  return dx12_surface_composition(state);
}

Color dx12_surface_clear(const Dx12State& state, Color color) {
  if (!state.composition_surface) return color;
  // ClearRTV bypasses blending. Clamp before multiplying so alpha-zero colors
  // cannot leave nonzero RGB in a premultiplied composition surface.
  const auto alpha = std::clamp(color.a, 0.0F, 1.0F);
  return {std::clamp(color.r, 0.0F, 1.0F) * alpha,
          std::clamp(color.g, 0.0F, 1.0F) * alpha,
          std::clamp(color.b, 0.0F, 1.0F) * alpha, alpha};
}
} // namespace cgpui
