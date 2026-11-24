#include "probe_window.hpp"

namespace renderer_probe {
DisplayFacts Window::display_facts() const {
  DisplayFacts result;
  result.event_epoch = display_event_epoch_;
  result.dpi = GetDpiForWindow(hwnd_);
  MONITORINFOEXW monitor{};
  monitor.cbSize = sizeof(monitor);
  RECT window{};
  if (!GetWindowRect(hwnd_, &window) || !GetMonitorInfoW(
      MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST), reinterpret_cast<MONITORINFO*>(&monitor))) return result;
  for (const wchar_t c : monitor.szDevice) { if (!c) break; result.device += c < 128 ? char(c) : '?'; }
  DEVMODEW mode{};
  mode.dmSize = sizeof(mode);
  if (!EnumDisplaySettingsW(monitor.szDevice, ENUM_CURRENT_SETTINGS, &mode)) return result;
  result.monitor_left = monitor.rcMonitor.left; result.monitor_top = monitor.rcMonitor.top;
  result.monitor_right = monitor.rcMonitor.right; result.monitor_bottom = monitor.rcMonitor.bottom;
  result.window_left = window.left; result.window_top = window.top;
  result.width = mode.dmPelsWidth; result.height = mode.dmPelsHeight;
  result.hz = mode.dmDisplayFrequency; result.bits_per_pixel = mode.dmBitsPerPel;
  result.orientation = mode.dmDisplayOrientation;
  result.valid = result.dpi != 0 && result.width != 0 && result.height != 0 && !result.device.empty();
  return result;
}
cgpui::Result<void> Window::lock_display(Report& report) {
  report_ = &report;
  report.display_before = display_facts();
  report.display_after = report.display_before;
  report.remote_session = GetSystemMetrics(SM_REMOTESESSION) != 0;
  SYSTEM_POWER_STATUS power{};
  if (GetSystemPowerStatus(&power)) report.ac_line_status = power.ACLineStatus;
  if (!report.display_before->valid) return std::unexpected(failure("initial display facts unavailable"));
  return {};
}
cgpui::Result<void> Window::check_display() const {
  if (!report_) return {}; // Initial constructor messages precede the locked premise.
  report_->display_after = display_facts();
  if (!report_->display_after->valid || report_->display_before != report_->display_after)
    return std::unexpected(failure("display/window facts or notification epoch changed; series invalid"));
  return {};
}
} // namespace renderer_probe
