#pragma once

namespace cgpui {

struct Error;
class WindowRuntime;
struct WindowRuntimeId;

// Handles only the explicit recoverable-frame result. No backend names or
// diagnostic strings participate in scheduling or fatal-error decisions.
[[nodiscard]] bool runtime_retry_frame(
    WindowRuntime& runtime, WindowRuntimeId window, const Error& error);

} // namespace cgpui
