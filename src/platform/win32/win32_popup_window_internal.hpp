#pragma once

#include "win32_window_internal.hpp"

#include <memory>

namespace cgpui {

// Popup windows (WindowKind::popup): WS_POPUP, never activated, not in
// the taskbar or Alt+Tab, owned by the parent window, and without a
// redirection surface (renderers present through DirectComposition,
// Win32SurfaceHandle::composition). Their own window class keeps the popup
// state in the window's extra bytes, so the window procedure and the
// placement need no Win32Window members.
inline constexpr const wchar_t* win32_popup_window_class = L"CGPUIPopupWindow";
inline constexpr DWORD win32_popup_style = WS_POPUP;
inline constexpr DWORD win32_popup_extended_style =
    WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP;
// Pointer-transparent popups (tooltips) are also layered and transparent:
// the system passes their pointer input to the window below on any thread
// (an HTTRANSPARENT hit test reaches same-thread windows only), and their
// DirectComposition content still shows.
inline constexpr DWORD win32_popup_pointer_transparent_style =
    WS_EX_LAYERED | WS_EX_TRANSPARENT;

// Extra bytes: flags, then the popup's offset (physical pixels) from its
// owner's client origin, kept when the owner moves.
inline constexpr int win32_popup_flags_index = 0;
inline constexpr int win32_popup_offset_x_index = static_cast<int>(sizeof(LONG));
inline constexpr int win32_popup_offset_y_index = static_cast<int>(2 * sizeof(LONG));
inline constexpr int win32_popup_extra_bytes = static_cast<int>(3 * sizeof(LONG));
inline constexpr LONG win32_popup_placed = 0x1;

// The popup owned by `owner`, at descriptor.size (logical, at the owner's
// monitor scale), placed at descriptor.popup when set and shown without
// activation. Fails without an owner.
Result<std::unique_ptr<Win32Window>> create_win32_popup_window(
    HINSTANCE instance,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    HWND owner);
[[nodiscard]] bool win32_is_popup_window(HWND hwnd);
// PlatformWindow::place_popup for the popup window `popup`.
PlatformPopupPlacementResult win32_place_popup(
    HWND popup, WindowPopupPlacement placement);
// Owned windows do not follow their owner on Windows: moves the placed
// popups of `owner` with it, keeping their offset to its client area.
void win32_move_owned_popups(HWND owner);

} // namespace cgpui
