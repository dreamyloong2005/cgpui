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
};

struct Error {
  ErrorCode code;
  std::string message;
};

template <typename T>
using Result = std::expected<T, Error>;

} // namespace cgpui
