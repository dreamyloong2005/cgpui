#pragma once

#include <string>

namespace cgpui {

struct PlatformOpenUrlResult {
  bool supported = false;
  bool opened = false;
  std::string backend;
  std::string error_message;
};

} // namespace cgpui
