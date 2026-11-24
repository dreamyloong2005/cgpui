#include "dx12_surface_internal.hpp"
#include "dx12_native_internal.hpp"

namespace cgpui {
Result<void> dx12_validate_surface_descriptor(const RenderSurfaceDescriptor& descriptor) {
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
  if (auto valid = dx12_validate_extent(descriptor.framebuffer_size, descriptor.scale); !valid) return valid;
  return {};
}
} // namespace cgpui
