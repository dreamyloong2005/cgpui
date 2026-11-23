#pragma once

namespace cgpui {
class Win32WindowRegistry;

// Reserve this UI thread's menu mutation before any native callback. Failed
// preflight changes neither the app menu model nor HWNDs/accelerator tables.
class Win32NativeMenuInstallationGuard {
 public:
  Win32NativeMenuInstallationGuard(Win32WindowRegistry& windows, bool has_items);
  ~Win32NativeMenuInstallationGuard();
  Win32NativeMenuInstallationGuard(const Win32NativeMenuInstallationGuard&) = delete;
  Win32NativeMenuInstallationGuard& operator=(const Win32NativeMenuInstallationGuard&) = delete;
  [[nodiscard]] bool allowed() const;
 private:
  bool owns_reservation_ = false;
  bool allowed_ = false;
};

[[nodiscard]] bool win32_native_menu_installation_active();
} // namespace cgpui
