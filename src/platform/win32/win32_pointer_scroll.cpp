#include "win32_pointer_scroll_internal.hpp"

namespace cgpui {

std::optional<Win32PointerScrollMessage> decode_win32_pointer_scroll(
    UINT message,
    WPARAM wparam) {
  if (message != WM_MOUSEWHEEL && message != WM_MOUSEHWHEEL) {
    return std::nullopt;
  }
  const int raw_delta = GET_WHEEL_DELTA_WPARAM(wparam);
  const float delta = static_cast<float>(raw_delta) /
      static_cast<float>(WHEEL_DELTA);
  return Win32PointerScrollMessage{
      .delta = message == WM_MOUSEWHEEL
          ? Point{0.0F, delta}
          : Point{delta, 0.0F},
      .precise = raw_delta % WHEEL_DELTA != 0,
  };
}

} // namespace cgpui
