#pragma once

#include "cgpui/core/event_window.hpp"

#include <cstdint>

namespace cgpui {

enum class PlatformWindowCloseResolution {
  accept,
  cancel,
};

struct PlatformWindowCloseState {
  bool pending = false;
  bool accepted = false;
  WindowCloseRequestSource source =
      WindowCloseRequestSource::window_manager;
  std::uint64_t sequence = 0;
  std::uint32_t accepted_count = 0;
  std::uint32_t cancelled_count = 0;
  std::uint32_t coalesced_count = 0;
};

} // namespace cgpui
