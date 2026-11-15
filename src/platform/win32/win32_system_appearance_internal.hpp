#pragma once

#include "cgpui/platform/platform_appearance.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <optional>

namespace cgpui {

inline constexpr const wchar_t* win32_personalize_key =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";

[[nodiscard]] SystemAppearance win32_read_system_appearance();

// Maps a top-level window message to the appearance source it reports, or
// nullopt when the message is unrelated to system appearance.
[[nodiscard]] std::optional<SystemAppearanceChangeSource>
win32_appearance_change_source(UINT message, WPARAM wparam, LPARAM lparam);

// Sets DWMWA_USE_IMMERSIVE_DARK_MODE on the window's title bar.
bool win32_apply_immersive_dark_mode(HWND hwnd, bool dark);

// Applies the current system mode to every CGPUI window on this thread.
void win32_apply_title_bar_appearance_to_thread_windows(bool dark);

} // namespace cgpui
