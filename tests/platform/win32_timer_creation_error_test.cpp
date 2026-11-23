#include "win32_timer_test_support.hpp"

#include <array>
#include <iostream>

int main() {
  struct Case { cgpui::Win32TimerWakeup::CreateTimer create; const char* message; };
  const std::array cases{
      Case{&cgpui_test::reject_timer<ERROR_NOT_ENOUGH_MEMORY>, "CreateThreadpoolTimer failed (Win32 error 8)"},
      Case{&cgpui_test::reject_timer<ERROR_ACCESS_DENIED>, "CreateThreadpoolTimer failed (Win32 error 5)"},
      Case{&cgpui_test::reject_timer<ERROR_SUCCESS>, "CreateThreadpoolTimer failed (Win32 error 0)"}};
  for (const auto& item : cases) {
    cgpui::Win32TimerWakeup timer(item.create);
    SetLastError(ERROR_BUSY); // Another native call must not replace the captured creation code.
    for (int check = 0; check != 2; ++check) {
      const auto result = timer.initialization_result();
      if (result || result.error().code != cgpui::ErrorCode::platform_initialization_failed ||
          result.error().message != item.message) {
        std::cerr << "timer creation did not preserve its native failure: " << item.message << '\n';
        return 1;
      }
    }
    timer.cancel(); // Failed owners must also destruct without touching a fake handle.
  }
  return 0;
}
