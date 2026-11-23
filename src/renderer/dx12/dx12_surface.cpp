#include "dx12_surface_internal.hpp"
#include "dx12_state_internal.hpp"
#include <algorithm>

namespace cgpui {
Result<void> dx12_configure_surface(Dx12State& state, const RenderSurfaceDescriptor& descriptor) {
  const auto* native = std::get_if<Win32SurfaceHandle>(&descriptor.native_surface);
  if (!native || !native->hwnd || !IsWindow(static_cast<HWND>(native->hwnd))) {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 requires a live Win32 HWND"});
  }
  const auto hwnd = static_cast<HWND>(native->hwnd);
  const bool no_redirection = (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_NOREDIRECTIONBITMAP) != 0;
  if (native->composition != no_redirection) {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 native surface must match its window's redirection style"});
  }
  if (!native->composition && descriptor.transparent_background) {
    return std::unexpected(dx12_unsupported("transparent ordinary HWND presentation"));
  }
  if (auto valid = dx12_validate_extent(descriptor.framebuffer_size, descriptor.scale); !valid) return valid;
  state.hwnd = hwnd;
  state.composition_surface = native->composition;
  state.transparent_background = descriptor.transparent_background;
  state.width = static_cast<UINT>(descriptor.framebuffer_size.width);
  state.height = static_cast<UINT>(descriptor.framebuffer_size.height);
  state.scale = descriptor.scale;
  return {};
}

RendererSurfaceComposition dx12_surface_composition(const Dx12State& state) {
  return {.supported = true, .transparent_requested = state.transparent_background,
          .composite_alpha = state.composition_surface ? RendererCompositeAlpha::pre_multiplied
                                                      : RendererCompositeAlpha::opaque};
}

Result<RendererSurfaceComposition> dx12_set_transparent_surface(Dx12State& state, bool transparent) {
  if (transparent && !state.composition_surface)
    return std::unexpected(dx12_unsupported("transparent ordinary HWND presentation"));
  // A native Popup's actual presentation stays premultiplied, as on Vulkan.
  // Changing intent does not change the HWND topology or recreate a swapchain.
  state.transparent_background = transparent;
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
