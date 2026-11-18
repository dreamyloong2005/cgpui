#pragma once

#include "win32_ime_composition_detail_internal.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imm.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cgpui {

// Raw bytes of one ImmGetCompositionStringW data kind (win32_window_ime.cpp).
[[nodiscard]] std::optional<std::vector<std::uint8_t>>
win32_ime_composition_bytes(HIMC context, DWORD data_kind);

// GCS_CURSORPOS, GCS_COMPATTR and GCS_COMPCLAUSE for an already read
// GCS_COMPSTR (win32_window_ime_detail.cpp).
[[nodiscard]] Win32ImeCompositionDetail read_win32_ime_composition_detail(
    HIMC context,
    std::wstring text);

} // namespace cgpui
