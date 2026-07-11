#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace {

std::atomic_uint observed_command_id{0};
cgpui::PlatformApplication* application = nullptr;
WNDPROC original_window_proc = nullptr;

LRESULT CALLBACK capture_command(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam) {
  if (message == WM_COMMAND && HIWORD(wparam) == 1U) {
    observed_command_id.store(LOWORD(wparam));
    application->quit();
    return 0;
  }
  return CallWindowProcW(original_window_proc, hwnd, message, wparam, lparam);
}

void set_control_down(bool down) {
  BYTE keyboard_state[256]{};
  GetKeyboardState(keyboard_state);
  keyboard_state[VK_CONTROL] = down ? 0x80 : 0x00;
  SetKeyboardState(keyboard_state);
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return 1;
  application = app->get();
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "Menu Accelerator Registration",
          .size = {420, 280}},
      [](const cgpui::PlatformEvent&) {});
  if (!window) return 2;

  const auto installed = (*app)->install_native_menu(cgpui::NativeMenuModel{
      .items = {cgpui::NativeMenuItem{
          .kind = cgpui::NativeMenuItemKind::submenu,
          .title = "File",
          .children = {cgpui::NativeMenuItem{
              .title = "Open",
              .action_name = "file.open",
              .accelerator = cgpui::NativeMenuAccelerator{
                  .key_code = 'O',
                  .modifiers = {.control = true},
              },
          }, cgpui::NativeMenuItem{
              .title = "Workspace 1",
              .action_name = "workspace.one",
              .accelerator = cgpui::NativeMenuAccelerator{
                  .key_code = '1',
                  .modifiers = {.super = true},
              },
          }, cgpui::NativeMenuItem{
              .title = "Release",
              .action_name = "file.release",
              .accelerator = cgpui::NativeMenuAccelerator{
                  .key_code = 'R',
                  .action = cgpui::KeyAction::released,
              },
          }},
      }},
  });
  if (!installed.supported || installed.accelerator_count != 3 ||
      installed.registered_accelerator_count != 1 ||
      installed.skipped_accelerator_count != 2) return 3;

  const auto native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  const HWND hwnd = surface == nullptr
      ? nullptr
      : static_cast<HWND>(surface->hwnd);
  original_window_proc = hwnd == nullptr
      ? nullptr
      : reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
            hwnd,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(capture_command)));
  if (original_window_proc == nullptr) return 4;

  set_control_down(true);
  PostMessageW(hwnd, WM_KEYDOWN, 'O', 0);
  const DWORD thread_id = GetCurrentThreadId();
  std::thread watchdog([thread_id] {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (observed_command_id.load() == 0) {
      PostThreadMessageW(thread_id, WM_QUIT, 0, 0);
    }
  });
  const int run_result = (*app)->run();
  watchdog.join();
  set_control_down(false);
  SetWindowLongPtrW(
      hwnd,
      GWLP_WNDPROC,
      reinterpret_cast<LONG_PTR>(original_window_proc));

  if (run_result != 0 || observed_command_id.load() != 0x1000U) return 5;
  return 0;
}
