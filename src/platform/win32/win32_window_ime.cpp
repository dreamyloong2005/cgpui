#include "win32_window_internal.hpp"

#include <optional>
#include <string>
#include <utility>

namespace cgpui {

namespace {

std::optional<std::string> win32_ime_composition_string(
    HWND hwnd,
    DWORD data_kind) {
  HIMC context = ImmGetContext(hwnd);
  if (context == nullptr) {
    return std::nullopt;
  }

  std::optional<std::string> result;
  const LONG byte_count =
      ImmGetCompositionStringW(context, data_kind, nullptr, 0);
  if (byte_count >= 0 &&
      byte_count % static_cast<LONG>(sizeof(wchar_t)) == 0) {
    if (byte_count == 0) {
      result = std::string{};
    } else {
      std::wstring text(
          static_cast<std::size_t>(byte_count) / sizeof(wchar_t), L'\0');
      const LONG written = ImmGetCompositionStringW(
          context,
          data_kind,
          text.data(),
          static_cast<DWORD>(byte_count));
      if (written >= 0 &&
          written % static_cast<LONG>(sizeof(wchar_t)) == 0) {
        text.resize(static_cast<std::size_t>(written) / sizeof(wchar_t));
        result = utf8_from_utf16(text);
      }
    }
  }

  ImmReleaseContext(hwnd, context);
  return result;
}

} // namespace

void Win32Window::ime_start_composition() {
  apply_ime_text_input_placement();
}

void Win32Window::ime_composition(LPARAM lparam) {
  if (hwnd_ == nullptr) {
    return;
  }

  if ((lparam & GCS_RESULTSTR) != 0) {
    std::optional<std::string> text =
        win32_ime_composition_string(hwnd_, GCS_RESULTSTR);
    if (text.has_value()) {
      callback_(ImeComposition{
          .phase = ImeCompositionPhase::commit,
          .text = std::move(*text),
          .modifiers = current_modifiers()});
    }
  }

  if ((lparam & GCS_COMPSTR) != 0) {
    std::optional<std::string> text =
        win32_ime_composition_string(hwnd_, GCS_COMPSTR);
    if (text.has_value()) {
      callback_(ImeComposition{
          .phase = ImeCompositionPhase::update,
          .text = std::move(*text),
          .modifiers = current_modifiers()});
    }
  }
}

void Win32Window::ime_end_composition() {
  callback_(ImeComposition{
      .phase = ImeCompositionPhase::cancel,
      .modifiers = current_modifiers()});
}

} // namespace cgpui
