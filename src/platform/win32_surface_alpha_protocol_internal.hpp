#pragma once

// Private token-only Win32 protocol shared by the platform and renderers.
// It carries no borrowed addresses or renderer/platform link dependency.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace cgpui {
inline constexpr UINT win32_surface_alpha_instance_message = WM_APP + 3U;
inline constexpr UINT win32_surface_alpha_query_message = WM_APP + 4U;
inline constexpr UINT win32_surface_alpha_acquire_message = WM_APP + 5U;
inline constexpr UINT win32_surface_alpha_release_message = WM_APP + 6U;
inline constexpr UINT win32_surface_alpha_status_message = WM_APP + 7U;
inline constexpr UINT win32_surface_alpha_mark_opaque_message = WM_APP + 8U;
// +9 belongs to the separate native-window-work mailbox.
inline constexpr UINT win32_surface_alpha_reserve_message = WM_APP + 16U;
inline constexpr UINT win32_surface_alpha_suspend_message = WM_APP + 17U;
inline constexpr const wchar_t* win32_surface_alpha_property = L"CGPUI.SurfaceAlpha.V1";

// The property identifies protocol V1. Renderer callers require the same process
// and UI thread for alpha operations. Opaque factory identity/mark may use bounded
// cross-thread SendMessageTimeoutW. No message interprets a token as an address,
// so posted malformed/foreign messages cannot dereference caller data.
// instance: (0, 0) -> UINT_PTR window_instance, even on unsupported OS/topology.
// query: (0, window_instance) -> HRESULT capability result.
// acquire/release/status/mark_opaque: (owner_cookie, window_instance) -> HRESULT.
// After discovery all operations validate instance; owner operations require a nonzero cookie.
// status: last repair_result for the active matching owner, else E_INVALIDARG.
// Supported: build >= 26100 and actual DWMWA_REDIRECTIONBITMAP_ALPHA succeeds.
// E_NOTIMPL: unsupported OS/topology/history; ERROR_BUSY: another lease/operation;
// E_INVALIDARG: stale instance/cookie. mark_opaque permanently records HWND-flip
// history, rejects an active lease, and makes later query/acquire E_NOTIMPL.
// Foreign-thread destruction may PostMessageW(release, owner_cookie, instance).
// Both tokens are checked on the window thread; no borrowed data is queued.
// reserve: fresh eligible system/client-caption ordinary HWND, holds the owner token
// with bitmap alpha FALSE; no BufferedPaint/menu repair. acquire may promote the
// same reservation. suspend rolls back that promotion without dropping ownership.
// Client caption requires no HMENU; acquire rechecks current menu/chrome capability.
// A reservation blocks mark_opaque, but active() remains actual bitmap alpha.
} // namespace cgpui
