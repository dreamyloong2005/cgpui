#include "metal_renderer_internal.hpp"

namespace cgpui {

bool MetalFramePacingState::try_acquire() {
  const std::scoped_lock lock(mutex_);
  if (in_flight_count_ >= max_frames_in_flight_) return false;
  ++in_flight_count_;
  return true;
}

void MetalFramePacingState::release() {
  const std::scoped_lock lock(mutex_);
  if (in_flight_count_ > 0) --in_flight_count_;
}

std::size_t MetalFramePacingState::in_flight_count() const {
  const std::scoped_lock lock(mutex_);
  return in_flight_count_;
}

}  // namespace cgpui
