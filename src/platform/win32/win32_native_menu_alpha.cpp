#include "win32_native_menu_alpha_internal.hpp"

#include <uxtheme.h>

namespace cgpui {
namespace {

HRESULT last_error() {
  const DWORD error = GetLastError();
  return error == ERROR_SUCCESS ? E_FAIL : HRESULT_FROM_WIN32(error);
}

} // namespace

HRESULT win32_repair_native_menu_alpha(HWND hwnd) {
  if (GetMenu(hwnd) == nullptr || IsIconic(hwnd)) return S_FALSE;
  MENUBARINFO bar{sizeof(bar)};
  RECT window{};
  if (!GetMenuBarInfo(hwnd, OBJID_MENU, 0, &bar) || !GetWindowRect(hwnd, &window)) {
    return last_error();
  }
  RECT rectangle = bar.rcBar;
  OffsetRect(&rectangle, -window.left, -window.top);
  if (IsRectEmpty(&rectangle)) return S_FALSE;
  HDC target = GetWindowDC(hwnd);
  if (target == nullptr) return last_error();
  HDC buffer = nullptr;
  BP_PAINTPARAMS parameters{sizeof(parameters), BPPF_NONCLIENT};
  HPAINTBUFFER paint = BeginBufferedPaint(
      target, &rectangle, BPBF_TOPDOWNDIB, &parameters, &buffer);
  HRESULT result = E_FAIL;
  if (paint != nullptr) {
    if (BitBlt(buffer, rectangle.left, rectangle.top,
               rectangle.right - rectangle.left, rectangle.bottom - rectangle.top,
               target, rectangle.left, rectangle.top, SRCCOPY)) {
      // Finish GDI's batched copy before BufferedPaint writes the DIB's alpha.
      result = GdiFlush() ? BufferedPaintSetAlpha(paint, &rectangle, 255) : E_FAIL;
    } else {
      result = last_error();
    }
    const HRESULT ended = EndBufferedPaint(paint, SUCCEEDED(result));
    if (SUCCEEDED(result)) result = ended;
  }
  ReleaseDC(hwnd, target);
  return result;
}

bool win32_native_menu_alpha_event(UINT message) {
  switch (message) {
    case WM_NCPAINT:
    case WM_NCACTIVATE:
    case WM_NCMOUSEMOVE:
    case WM_NCMOUSELEAVE:
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:
    case WM_INITMENU:
    case WM_INITMENUPOPUP:
    case WM_UNINITMENUPOPUP:
    case WM_MENUSELECT:
    case WM_ENTERMENULOOP:
    case WM_EXITMENULOOP:
    case WM_SYSCOMMAND:
    case WM_THEMECHANGED:
    case WM_SYSCOLORCHANGE:
    case WM_DWMCOMPOSITIONCHANGED:
    case WM_DPICHANGED:
    case WM_WINDOWPOSCHANGED:
      return true;
    default:
      return false;
  }
}

} // namespace cgpui
