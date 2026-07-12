#pragma once

#include <cstddef>
#include <cstdint>

namespace cgpui {

struct AnimationFramePacingSnapshot {
  std::size_t pending_animation_count = 0;
  bool element_frame_pending = false;
  bool wakeup_scheduled = false;
  std::uint64_t next_frame_deadline_ms = 0;
  std::uint64_t last_scheduled_delay_ms = 0;
  std::size_t scheduled_wakeup_count = 0;
  std::size_t delivered_frame_count = 0;
  std::size_t coalesced_request_count = 0;
  std::size_t late_frame_count = 0;
};

} // namespace cgpui
