#include "win32_window_internal.hpp"

#include "win32_window_ime_read_internal.hpp"

#include <cstring>
#include <optional>
#include <string>
#include <utility>

namespace cgpui {

std::optional<std::vector<std::uint8_t>> win32_ime_composition_bytes(
    HIMC context,
    DWORD data_kind) {
  const LONG size = ImmGetCompositionStringW(context, data_kind, nullptr, 0);
  if (size < 0) {
    return std::nullopt;
  }
  std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
  if (size > 0) {
    const LONG written = ImmGetCompositionStringW(
        context, data_kind, bytes.data(), static_cast<DWORD>(size));
    if (written < 0) {
      return std::nullopt;
    }
    bytes.resize(static_cast<std::size_t>(written));
  }
  return bytes;
}

namespace {

std::optional<std::wstring> ime_composition_text(HIMC context, DWORD kind) {
  const auto bytes = win32_ime_composition_bytes(context, kind);
  if (!bytes.has_value() || bytes->size() % sizeof(wchar_t) != 0) {
    return std::nullopt;
  }
  std::wstring text(bytes->size() / sizeof(wchar_t), L'\0');
  if (!text.empty()) {
    std::memcpy(text.data(), bytes->data(), bytes->size());
  }
  return text;
}

} // namespace

void Win32Window::ime_start_composition() {
  apply_ime_text_input_placement();
}

void Win32Window::ime_composition(LPARAM lparam) {
  HIMC context = hwnd_ == nullptr ? nullptr : ImmGetContext(hwnd_);
  if (context == nullptr) {
    return;
  }
  std::optional<ImeComposition> commit;
  std::optional<ImeComposition> update;
  if ((lparam & GCS_RESULTSTR) != 0) {
    if (auto text = ime_composition_text(context, GCS_RESULTSTR)) {
      commit = ImeComposition{
          .phase = ImeCompositionPhase::commit,
          .text = utf8_from_utf16(*text),
          .modifiers = current_modifiers()};
    }
  }
  if ((lparam & GCS_COMPSTR) != 0) {
    if (auto text = ime_composition_text(context, GCS_COMPSTR)) {
      update = win32_ime_composition_update(
          read_win32_ime_composition_detail(context, std::move(*text)),
          current_modifiers());
    }
  }
  // Release before dispatch: callbacks may re-enter IMM placement.
  ImmReleaseContext(hwnd_, context);
  for (auto* event : {&commit, &update}) {
    if (event->has_value()) {
      callback_(std::move(**event));
    }
  }
}

void Win32Window::ime_end_composition() {
  callback_(ImeComposition{
      .phase = ImeCompositionPhase::cancel,
      .modifiers = current_modifiers()});
}

} // namespace cgpui
