#include "win32_dpi_awareness_internal.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {
namespace {

PlatformDpiAwareness awareness_from_context(DPI_AWARENESS_CONTEXT context) {
  if (context == nullptr) return PlatformDpiAwareness::unsupported;
  if (AreDpiAwarenessContextsEqual(
          context, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
    return PlatformDpiAwareness::per_monitor_aware_v2;
  }
  switch (GetAwarenessFromDpiAwarenessContext(context)) {
    case DPI_AWARENESS_UNAWARE:
      return PlatformDpiAwareness::unaware;
    case DPI_AWARENESS_SYSTEM_AWARE:
      return PlatformDpiAwareness::system_aware;
    case DPI_AWARENESS_PER_MONITOR_AWARE:
      return PlatformDpiAwareness::per_monitor_aware;
    default:
      return PlatformDpiAwareness::unsupported;
  }
}

PlatformDpiAwarenessState declare_once() {
  const BOOL declared = SetProcessDpiAwarenessContext(
      DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  const DWORD error = declared ? ERROR_SUCCESS : GetLastError();
  PlatformDpiAwarenessState state = win32_current_dpi_awareness();
  state.declared_by_framework = declared != FALSE;
  state.already_set_by_process = error == ERROR_ACCESS_DENIED;
  return state;
}

} // namespace

PlatformDpiAwarenessState win32_declare_dpi_awareness() {
  static const PlatformDpiAwarenessState state = declare_once();
  return state;
}

PlatformDpiAwarenessState win32_current_dpi_awareness() {
  return PlatformDpiAwarenessState{
      .awareness = awareness_from_context(GetThreadDpiAwarenessContext()),
      .backend = "win32",
  };
}

} // namespace cgpui
