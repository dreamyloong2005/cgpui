#include "dx12_frame_admission_internal.hpp"
#include "dx12_frame_pacing_internal.hpp"
#include "dx12_frame_recovery_internal.hpp"
#include "dx12_state_internal.hpp"
#include <chrono>
#include <utility>

namespace cgpui {
Result<Dx12FrameAdmissionTiming> dx12_admit_frame(Dx12State& state) {
  const auto started = std::chrono::steady_clock::now();
  auto admission = state.frame_pacing->try_admit();
  const auto stopped = std::chrono::steady_clock::now();
  if (!admission) {
    auto failure = dx12_handle_frame_failure(state, std::move(admission.error()));
    return std::unexpected(std::move(failure.error()));
  }
  if (*admission == Dx12FrameAdmission::pending)
    return std::unexpected(Error{.code = ErrorCode::frame_not_ready,
        .message = "DX12 swapchain frame admission is not ready"});
  if (*admission == Dx12FrameAdmission::disabled) return Dx12FrameAdmissionTiming{};
  const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(stopped - started).count();
  return Dx12FrameAdmissionTiming{elapsed > 0 ? static_cast<std::uint64_t>(elapsed) : 0};
}
} // namespace cgpui
