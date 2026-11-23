#include "frame_wait_win32_test_support.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>

namespace cgpui_test::detail {

void pump_frame_wait_win32_messages(void*, unsigned maximum_messages) {
  MSG message{};
  const unsigned limit = std::min(maximum_messages, 32U);
  for (unsigned count = 0; count != limit &&
      PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0; ++count) {
    if (message.message == WM_QUIT) {
      PostQuitMessage(static_cast<int>(message.wParam));
      break; // Do not consume the surrounding test/event loop's exit request.
    }
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
}

} // namespace cgpui_test::detail
