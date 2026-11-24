#include "win32_native_window_work_internal.hpp"
#include <utility>

namespace cgpui {
namespace {
bool post_work(void* window, NativeWindowWorkToken token) noexcept {
  return PostMessageW(static_cast<HWND>(window), win32_native_window_work_message,
      static_cast<WPARAM>(token.instance), static_cast<LPARAM>(token.work)) != FALSE;
}
bool owned_here(HWND hwnd) noexcept {
  DWORD process = 0;
  const DWORD thread = GetWindowThreadProcessId(hwnd, &process);
  return thread && thread == GetCurrentThreadId() && process == GetCurrentProcessId();
}
} // namespace

std::uintptr_t win32_native_window_work_open(HWND hwnd) noexcept {
  return owned_here(hwnd) ? native_window_work_open(hwnd, post_work) : 0;
}
void win32_native_window_work_close(HWND hwnd, std::uintptr_t& instance) noexcept {
  native_window_work_close(hwnd, std::exchange(instance, 0));
}
bool win32_window_proc_handle_native_work(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) noexcept {
  if (message != win32_native_window_work_message) return false;
  result = 0;
  if (owned_here(hwnd))
    (void)native_window_work_dispatch(hwnd,
        {static_cast<std::uintptr_t>(wparam), static_cast<std::uintptr_t>(lparam)});
  return true;
}
} // namespace cgpui
