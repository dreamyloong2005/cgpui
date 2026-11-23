#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace cgpui {

// Called after Windows has drawn the real menu. Only the menu-bar alpha is
// changed; native RGB, layout, interaction and command dispatch stay native.
[[nodiscard]] HRESULT win32_repair_native_menu_alpha(HWND hwnd);
[[nodiscard]] bool win32_native_menu_alpha_event(UINT message);

} // namespace cgpui
