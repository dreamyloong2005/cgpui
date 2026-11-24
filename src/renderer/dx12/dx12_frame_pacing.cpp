#include "dx12_frame_pacing_internal.hpp"
#include <utility>

namespace cgpui {
Dx12FramePacing::Dx12FramePacing(Dx12FramePacingApi& api) noexcept : api_(api) {}
Dx12FramePacing::~Dx12FramePacing() { reset(); }
Dx12FramePacingApi& Dx12FramePacing::api() const noexcept { return api_; }

Result<void> Dx12FramePacing::configure(IDXGISwapChain2* chain, bool enabled, Dx12InitializationFailureClass* failure) {
  if (failure) *failure = Dx12InitializationFailureClass::terminal;
  if (configured_) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 frame pacing must be reset before reconfiguration"});
  if (!enabled) {
    configured_ = true;
    return {};
  }
  const HRESULT configured = api_.set_maximum_latency(chain, 1);
  if (FAILED(configured)) return std::unexpected(dx12_initialization_error(configured, "maximum frame latency", failure));
  const HANDLE handle = api_.latency_object(chain);
  if (!handle) return std::unexpected(Error{.code = ErrorCode::renderer_initialization_failed,
      .message = "DX12 waitable swapchain returned a null frame-latency handle"});
  handle_ = handle;
  enabled_ = configured_ = true;
  return {};
}

Result<Dx12FrameAdmission> Dx12FramePacing::try_admit() {
  if (!configured_) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 frame pacing is not configured"});
  if (!enabled_) return Dx12FrameAdmission::disabled;
  if (granted_) return Dx12FrameAdmission::ready;
  const auto waited = api_.poll(handle_);
  if (waited.status == WAIT_OBJECT_0) {
    granted_ = true;
    return Dx12FrameAdmission::ready;
  }
  if (waited.status == WAIT_TIMEOUT) return Dx12FrameAdmission::pending;
  if (waited.status == WAIT_FAILED) {
    const DWORD error = waited.error ? waited.error : ERROR_GEN_FAILURE;
    return std::unexpected(dx12_error(HRESULT_FROM_WIN32(error), "frame admission wait"));
  }
  return std::unexpected(Error{.code = ErrorCode::renderer_initialization_failed,
      .message = "DX12 frame admission returned an unexpected non-alertable wait status"});
}

void Dx12FramePacing::presentation_attempted() noexcept { granted_ = false; }
void Dx12FramePacing::reset() noexcept {
  const HANDLE handle = std::exchange(handle_, nullptr);
  configured_ = enabled_ = granted_ = false;
  if (handle) (void)api_.close(handle);
}
} // namespace cgpui
