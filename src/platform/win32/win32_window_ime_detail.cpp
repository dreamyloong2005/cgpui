#include "win32_window_ime_read_internal.hpp"

#include <cstring>
#include <utility>

namespace cgpui {

Win32ImeCompositionDetail read_win32_ime_composition_detail(
    HIMC context,
    std::wstring text) {
  Win32ImeCompositionDetail detail{.text = std::move(text)};
  // GCS_CURSORPOS returns the caret in UTF-16 units as the result value.
  const LONG cursor =
      ImmGetCompositionStringW(context, GCS_CURSORPOS, nullptr, 0);
  if (cursor >= 0) {
    detail.cursor = static_cast<std::int32_t>(cursor);
  }
  if (auto attributes = win32_ime_composition_bytes(context, GCS_COMPATTR);
      attributes.has_value()) {
    detail.attributes = std::move(*attributes);
  }
  const auto clauses = win32_ime_composition_bytes(context, GCS_COMPCLAUSE);
  if (clauses.has_value() && clauses->size() % sizeof(std::uint32_t) == 0) {
    detail.clauses.resize(clauses->size() / sizeof(std::uint32_t));
    if (!detail.clauses.empty()) {
      std::memcpy(detail.clauses.data(), clauses->data(), clauses->size());
    }
  }
  return detail;
}

} // namespace cgpui
