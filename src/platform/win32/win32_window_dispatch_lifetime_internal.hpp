#pragma once

#include "cgpui/platform/platform_window.hpp"
#include "win32_window_registry_internal.hpp"
#include <cstdint>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace cgpui {

// Native dispatch identity, event callback and registry lifetime travel
// together. Win32Window invalidates this identity before native teardown.
class Win32WindowDispatchLifetime {
 protected:
  explicit Win32WindowDispatchLifetime(PlatformEventCallback callback);
  void invalidate_window_registration();
  HWND hwnd_ = nullptr;
  std::uintptr_t native_work_instance_ = 0;
  PlatformEventCallback callback_;

 private:
  friend class Win32WindowRegistry;
  std::shared_ptr<Win32WindowRegistration> registration_;
};

} // namespace cgpui
