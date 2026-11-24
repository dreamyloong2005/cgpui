#include "probe_window.hpp"
#include <algorithm>

namespace renderer_probe {
LRESULT CALLBACK Window::procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  auto* self = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (message == WM_NCCREATE) {
    self = static_cast<Window*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
  }
  if (self && (message == WM_MOVE || message == WM_SIZE || message == WM_DPICHANGED ||
      message == WM_DISPLAYCHANGE)) ++self->display_event_epoch_;
  return DefWindowProcW(hwnd, message, wparam, lparam);
}
Window::Window() {
  serial_ = CreateMutexW(nullptr, FALSE, L"Local\\CGPUI_Renderer_Steady_State_Probe");
  if (!serial_) { setup_error_ = "serial mutex creation failed"; return; }
  const auto acquired = WaitForSingleObject(serial_, 0);
  serial_owned_ = acquired == WAIT_OBJECT_0 || acquired == WAIT_ABANDONED;
  if (!serial_owned_) { setup_error_ = "another renderer probe is running"; return; }
  if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
    setup_error_ = "per-monitor-v2 DPI setup failed; physical-pixel premise unavailable"; return;
  }
  instance_ = GetModuleHandleW(nullptr);
  class_name_ = L"CGPUIRendererSteadyState_" + std::to_wstring(GetCurrentProcessId());
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc); wc.lpfnWndProc = procedure;
  wc.hInstance = instance_; wc.lpszClassName = class_name_.c_str();
  if (!RegisterClassExW(&wc)) { setup_error_ = "window class registration failed"; return; }
  constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  RECT rect{0, 0, 800, 600}, work{};
  if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0) ||
      !AdjustWindowRectExForDpi(&rect, style, FALSE, 0, GetDpiForSystem())) {
    setup_error_ = "window geometry setup failed"; return;
  }
  if (work.right - work.left < rect.right - rect.left || work.bottom - work.top < rect.bottom - rect.top) {
    setup_error_ = "desktop too small for the fixed 800x600 scene"; return;
  }
  hwnd_ = CreateWindowExW(0, class_name_.c_str(), L"CGPUI renderer baseline (do not cover)",
      style, work.left, work.top, rect.right - rect.left, rect.bottom - rect.top,
      nullptr, nullptr, instance_, this);
  if (!hwnd_) { setup_error_ = "native window creation failed"; return; }
  // This is an explicit interactive native measurement; never steals focus or sets topmost.
  ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
  UpdateWindow(hwnd_);
}
Window::~Window() {
  if (hwnd_ && IsWindow(hwnd_)) DestroyWindow(hwnd_);
  if (!class_name_.empty()) UnregisterClassW(class_name_.c_str(), instance_);
  if (serial_owned_) ReleaseMutex(serial_);
  if (serial_) CloseHandle(serial_);
}
cgpui::Result<void> Window::check() const {
  if (!setup_error_.empty()) return std::unexpected(failure(setup_error_));
  RECT client{};
  if (!IsWindow(hwnd_) || !IsWindowVisible(hwnd_) || IsIconic(hwnd_) ||
      !GetClientRect(hwnd_, &client) || client.right != 800 || client.bottom != 600)
    return std::unexpected(failure("window closed/hidden/minimized or client size changed"));
  // Sampled occlusion guard, not a proof of every pixel's desktop visibility.
  for (const POINT sample : {POINT{2, 2}, POINT{797, 2}, POINT{400, 300}, POINT{2, 597}, POINT{797, 597}}) {
    POINT screen = sample;
    if (!ClientToScreen(hwnd_, &screen) || WindowFromPoint(screen) != hwnd_)
      return std::unexpected(failure("a sampled client point is obscured; no hidden-window baseline"));
  }
  return {};
}
cgpui::Result<void> Window::pump(bool verify_display) const {
  MSG message{};
  for (unsigned count = 0; count != 64 && PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE); ++count) {
    if (message.message == WM_QUIT) return std::unexpected(failure("WM_QUIT interrupted the probe"));
    TranslateMessage(&message); DispatchMessageW(&message);
  }
  if (auto valid = check(); !valid) return valid;
  return verify_display ? check_display() : cgpui::Result<void>{};
}
cgpui::Result<void> Window::wait(Clock::time_point deadline) const {
  const auto now = Clock::now();
  if (now >= deadline) return std::unexpected(failure("frame acquisition deadline expired"));
  const auto remaining = std::chrono::ceil<std::chrono::milliseconds>(deadline - now).count();
  const DWORD milliseconds = static_cast<DWORD>(std::min<std::int64_t>(2, remaining));
  if (MsgWaitForMultipleObjectsEx(0, nullptr, milliseconds, QS_ALLINPUT,
      MWMO_INPUTAVAILABLE) == WAIT_FAILED) return std::unexpected(failure("native readiness scheduling wait failed"));
  return pump(false); // Dispatch/epoch updates count as scheduled acquisition wait; no facts query here.
}
cgpui::RenderSurfaceDescriptor Window::descriptor() const {
  return {.native_surface = cgpui::Win32SurfaceHandle{instance_, hwnd_},
      .framebuffer_size = {800, 600}, .scale = {1.0F}};
}
} // namespace renderer_probe
