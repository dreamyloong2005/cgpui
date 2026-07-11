#pragma once

#include <functional>
#include <string>

namespace cgpui {

using PlatformReopenCallback = std::function<void()>;

struct PlatformReopenResult {
  bool supported = false;
  bool requested = false;
  std::string backend;
  std::string error_message;
};

} // namespace cgpui
