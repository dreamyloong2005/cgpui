#include "win32_native_menu_policy_internal.hpp"
#include "win32_window_registry_internal.hpp"
#include "win32_window_factory_internal.hpp"
#include "win32_window_surface_alpha_internal.hpp"

namespace cgpui {
namespace {
thread_local bool installing_native_menu = false;
}

Win32NativeMenuInstallationGuard::Win32NativeMenuInstallationGuard(
    Win32WindowRegistry& windows, bool has_items) {
  if (installing_native_menu) return;
  allowed_ = true;
  if (has_items) windows.visit([&](Win32Window* window) {
    const auto* alpha = win32_window_surface_alpha(win32_window_hwnd(*window));
    if (alpha && !alpha->accepts_native_menu()) allowed_ = false;
  });
  // Preflight is read-only and invokes no application/native callbacks.
  // Reserve after its snapshot allocation so a thrown constructor cannot latch
  // this thread's gate; keep it reserved across all subsequent menu mutations.
  if (allowed_) { installing_native_menu = true; owns_reservation_ = true; }
}

Win32NativeMenuInstallationGuard::~Win32NativeMenuInstallationGuard() {
  if (owns_reservation_) installing_native_menu = false;
}

bool Win32NativeMenuInstallationGuard::allowed() const { return allowed_; }
bool win32_native_menu_installation_active() { return installing_native_menu; }
} // namespace cgpui
