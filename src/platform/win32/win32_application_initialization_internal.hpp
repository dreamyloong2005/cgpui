#pragma once

#include "win32_timer_wakeup_internal.hpp"

namespace cgpui {

// Private per-instance creation seam; the public entry always uses the native function.
Result<std::unique_ptr<PlatformApplication>> create_win32_application(
    Win32TimerWakeup::CreateTimer create_timer);

} // namespace cgpui
