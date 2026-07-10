#include "win32_pointer_button_internal.hpp"

namespace cgpui {
namespace {

MouseButton xbutton_from_wparam(WPARAM wparam) {
  return GET_XBUTTON_WPARAM(wparam) == XBUTTON1
      ? MouseButton::back
      : MouseButton::forward;
}

} // namespace

std::optional<Win32PointerButtonMessage> decode_win32_pointer_button(
    UINT message,
    WPARAM wparam) {
  switch (message) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
      return Win32PointerButtonMessage{
          .button = MouseButton::left,
          .pressed = message != WM_LBUTTONUP,
          .click_count = message == WM_LBUTTONDBLCLK ? 2U : 1U};
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
      return Win32PointerButtonMessage{
          .button = MouseButton::right,
          .pressed = message != WM_RBUTTONUP,
          .click_count = message == WM_RBUTTONDBLCLK ? 2U : 1U};
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
      return Win32PointerButtonMessage{
          .button = MouseButton::middle,
          .pressed = message != WM_MBUTTONUP,
          .click_count = message == WM_MBUTTONDBLCLK ? 2U : 1U};
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
      return Win32PointerButtonMessage{
          .button = xbutton_from_wparam(wparam),
          .pressed = message != WM_XBUTTONUP,
          .click_count = message == WM_XBUTTONDBLCLK ? 2U : 1U,
          .result = TRUE};
    default:
      return std::nullopt;
  }
}

} // namespace cgpui
