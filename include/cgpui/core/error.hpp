#pragma once

#include <expected>
#include <string>

namespace cgpui {

enum class ErrorCode {
  platform_initialization_failed,
  window_creation_failed,
  renderer_initialization_failed,
  frame_acquisition_failed,
  unsupported_platform,
  invalid_argument,
  asset_io_failed,
  asset_too_large,
  asset_revision_exhausted,
  // The renderer is usable, but this frame was not presented. Schedule a
  // later frame; other acquisition/initialization errors remain fatal.
  frame_retry_required,
  // begin_frame acquired no frame and submitted no work: schedule a delayed
  // attempt rather than treating temporary presentation backpressure as fatal.
  frame_not_ready,
  // Device recovery is waiting for other owners to retire their native state.
  // No successful frame/resize is implied; retry later without a busy loop.
  renderer_recovery_pending,
};

struct Error {
  ErrorCode code;
  std::string message;
};

template <typename T>
using Result = std::expected<T, Error>;

} // namespace cgpui
