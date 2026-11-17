#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imm.h>

#include <cmath>
#include <iostream>
#include <variant>

// IME placement rects are logical pixels; IMM32 receives physical client
// pixels (logical x WindowState::scale).
int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return 1;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI IME placement scale test",
                              .size = cgpui::Size{320.0F, 240.0F}},
      [](const cgpui::PlatformEvent&) {});
  if (!window) return 2;
  const cgpui::NativeSurfaceHandle native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  if (surface == nullptr || surface->hwnd == nullptr) return 3;
  auto* hwnd = static_cast<HWND>(surface->hwnd);
  // Force a non-unit scale so the check holds on a 96 DPI machine too.
  RECT suggested{};
  GetWindowRect(hwnd, &suggested);
  SendMessageW(hwnd, WM_DPICHANGED, MAKEWPARAM(144, 144),
               reinterpret_cast<LPARAM>(&suggested));
  const float scale = (*window)->state().scale.value;
  if (std::fabs(scale - 1.5F) > 0.001F) return 4;
  (*window)->set_ime_text_input_placement(cgpui::ImeTextInputPlacement{
      .rect = {{24.0F, 36.0F}, {2.0F, 20.0F}}});
  HIMC context = ImmGetContext(hwnd);
  if (context == nullptr) {
    std::cerr << "no input context; IMM32 placement not observable\n";
    return 0;
  }
  COMPOSITIONFORM composition{};
  const BOOL read = ImmGetCompositionWindow(context, &composition);
  ImmReleaseContext(hwnd, context);
  if (read == FALSE) return 5;
  if (composition.ptCurrentPos.x != 36 || composition.ptCurrentPos.y != 54) {
    std::cerr << "composition point " << composition.ptCurrentPos.x << ','
              << composition.ptCurrentPos.y << " expected 36,54\n";
    return 6;
  }
  return 0;
}
