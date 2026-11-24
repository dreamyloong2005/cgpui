#include "cgpui/platform/platform.hpp"
#include "win32_native_window_work_internal.hpp"
#include <memory>
#include <thread>
#include <variant>

using namespace cgpui;
namespace {
struct Work final : NativeWindowWork {
  DWORD owner = GetCurrentThreadId();
  unsigned calls = 0;
  bool wrong_thread = false, busy = false;
  NativeWindowWorkResult run() noexcept override {
    ++calls;
    wrong_thread |= GetCurrentThreadId() != owner;
    return busy ? NativeWindowWorkResult::deferred : NativeWindowWorkResult::complete;
  }
};
unsigned dispatch_work(HWND hwnd) {
  MSG message{};
  unsigned count = 0;
  while (count != 8 && PeekMessageW(&message, hwnd, win32_native_window_work_message,
                                  win32_native_window_work_message, PM_REMOVE)) {
    ++count;
    DispatchMessageW(&message);
  }
  return count;
}
} // namespace

int main() {
  auto app = create_platform_application();
  if (!app) return 1;
  auto window = (*app)->create_window(
      {.title = "CGPUI managed native work", .size = {120, 80}}, [](const PlatformEvent&) {});
  if (!window) return 2;
  const auto surface = (*window)->native_surface();
  const auto* native = std::get_if<Win32SurfaceHandle>(&surface);
  if (!native || !native->hwnd) return 3;
  const auto hwnd = static_cast<HWND>(native->hwnd);
  auto work = std::make_shared<Work>();
  NativeWindowWorkToken token;
  bool posted = false;
  std::thread sender([&] {
    token = native_window_work_register(hwnd, work);
    posted = native_window_work_post(token) && native_window_work_post(token);
  });
  sender.join();
  if (!token) return 4;
  if (!posted || work->calls || dispatch_work(hwnd) != 1 ||
      work->calls != 1 || work->wrong_thread) return 5;
  // Numeric stale/forged tokens do not become callback pointers.
  if (!PostMessageW(hwnd, win32_native_window_work_message, token.instance, 0) ||
      dispatch_work(hwnd) != 1 || work->calls != 1) return 6;
  work->busy = true;
  if (!native_window_work_post(token) || dispatch_work(hwnd) != 1 || work->calls != 2 ||
      dispatch_work(hwnd) != 0) return 7;
  work->busy = false;
  if (!native_window_work_post(token) || dispatch_work(hwnd) != 1 || work->calls != 3) return 8;
  if (!native_window_work_post(token)) return 9;
  window->reset(); // Endpoint closes before DestroyWindow, cancelling queued work.
  if (native_window_work_post(token) ||
      native_window_work_dispatch(hwnd, token) != NativeWindowWorkDispatch::ignored ||
      work->calls != 3) return 10;

  const HWND unmanaged = CreateWindowExW(0, L"STATIC", L"unmanaged work target", 0,
      0, 0, 10, 10, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (!unmanaged) return 11;
  const bool wrongly_registered = bool(native_window_work_register(unmanaged, work));
  DestroyWindow(unmanaged);
  if (wrongly_registered) return 12;
  MSG message{};
  while (PeekMessageW(&message, nullptr, WM_QUIT, WM_QUIT, PM_REMOVE)) {}
  return 0;
}
