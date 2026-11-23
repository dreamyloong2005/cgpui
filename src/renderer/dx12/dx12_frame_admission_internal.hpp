#pragma once

#include "cgpui/core/error.hpp"
#include <cstdint>

namespace cgpui {
struct Dx12State;
struct Dx12FrameAdmissionTiming {
  std::uint64_t admission_call_ns = 0;
};
// Caller holds the operation guard and has completed dx12_prepare_frame.
// Only this successful admission call is measured; no pending/holding time.
Result<Dx12FrameAdmissionTiming> dx12_admit_frame(Dx12State& state);
} // namespace cgpui
