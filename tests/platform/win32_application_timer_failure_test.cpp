#include "win32_application_initialization_internal.hpp"
#include "win32_timer_test_support.hpp"

#include <iostream>

int main() {
  auto application = cgpui::create_win32_application(
      &cgpui_test::reject_timer<ERROR_NOT_ENOUGH_MEMORY>);
  if (application) {
    std::cerr << "platform factory exposed an application without a native delayed timer\n";
    return 1;
  }
  if (application.error().code != cgpui::ErrorCode::platform_initialization_failed ||
      application.error().message != "CreateThreadpoolTimer failed (Win32 error 8)") return 2;
  // No fault state may leak to the actual public/native entry in the same process.
  auto native = cgpui::create_platform_application();
  if (!native || !*native) return 3;
  return 0;
}
