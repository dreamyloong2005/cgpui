#pragma once

#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/text.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <imm.h>

#include "win32_accessibility_internal.hpp"
#include "win32_drag_drop_internal.hpp"
#include "win32_native_internal.hpp"
#include "win32_window_message_internal.hpp"

#include <string>
#include <string_view>

namespace cgpui {

inline constexpr UINT cgpui_wakeup_message = WM_APP + 1U;

std::wstring widen(std::string_view value);
std::string utf8_from_utf16(std::wstring_view value);
UINT test_drag_enter_message();
UINT test_drag_update_message();
UINT test_drag_drop_message();
UINT test_drag_exit_message();
DragDropPayload drag_payload_from_test_hook(
    const Win32TestDragDropPayload* payload);
Point drag_position_from_test_hook(const Win32TestDragDropPayload* payload);
DragDropAction drag_action_from_drop_effect(DWORD drop_effect);
DragDropAction drag_action_from_test_hook(
    const Win32TestDragDropPayload* payload);
DWORD choose_ole_drop_effect(DWORD allowed_effect, DWORD key_state);
DragDropPayload drag_payload_from_ole_data_object(IDataObject* data_object);
KeyboardModifiers current_modifiers();
const wchar_t* cursor_id_for(CursorShape cursor_shape);
DWORD win32_window_style_for(const WindowChromeOptions& chrome);
DWORD win32_window_extended_style_for(const WindowChromeOptions& chrome);
PlatformFontDiscoveryResult win32_discover_fonts();
LRESULT CALLBACK win32_window_proc(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam);
bool win32_window_proc_handle_test_drag(
    Win32WindowMessageTarget* window,
    UINT message,
    LPARAM lparam,
    LRESULT& result);
bool win32_window_proc_handle_lifecycle(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result);
bool win32_window_proc_handle_pointer(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result);
bool win32_window_proc_handle_keyboard(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result);
bool win32_window_proc_handle_text(
    UINT message,
    WPARAM wparam,
    Win32WindowMessageTarget* window,
    LRESULT& result);

} // namespace cgpui
