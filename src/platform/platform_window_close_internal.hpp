#pragma once

#include "cgpui/platform/platform_window_close.hpp"

namespace cgpui {

class PlatformWindowCloseController {
 public:
  [[nodiscard]] bool begin(WindowCloseRequestSource source);
  [[nodiscard]] bool resolve(PlatformWindowCloseResolution resolution);
  [[nodiscard]] PlatformWindowCloseState state() const;
  [[nodiscard]] WindowCloseRequested event() const;

 private:
  PlatformWindowCloseState state_;
};

} // namespace cgpui
