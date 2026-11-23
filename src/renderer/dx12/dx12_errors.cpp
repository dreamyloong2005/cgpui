#include "dx12_native_internal.hpp"
#include <cmath>
#include <cstdio>

namespace cgpui {
Error dx12_error(HRESULT result, const char* operation) {
  char code[16]{};
  std::snprintf(code, sizeof(code), "0x%08lX", static_cast<unsigned long>(result));
  return {.code = ErrorCode::renderer_initialization_failed,
          .message = std::string("DX12 ") + operation + " failed (" + code + ")"};
}
Error dx12_unsupported(const char* feature) {
  return {.code = ErrorCode::unsupported_platform,
          .message = std::string("DX12 bootstrap does not support ") + feature};
}
Result<void> dx12_validate_extent(Size extent, DpiScale scale) {
  constexpr float limit = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
  if (!std::isfinite(extent.width) || !std::isfinite(extent.height) ||
      extent.width < 1 || extent.height < 1 || extent.width > limit ||
      extent.height > limit || !std::isfinite(scale.value) || scale.value <= 0) {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 needs finite positive framebuffer extent and scale"});
  }
  return {};
}
Dx12Event::~Dx12Event() { if (value) CloseHandle(value); }
} // namespace cgpui
