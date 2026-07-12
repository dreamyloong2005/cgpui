#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>

#include "win32_accessibility_internal.hpp"
#include "win32_window_proc_accessibility_internal.hpp"

namespace {

class TestAccessibilityTarget final
    : public cgpui::Win32AccessibilityMessageTarget {
 public:
  bool accessibility_object(
      WPARAM wparam,
      LPARAM lparam,
      LRESULT& result) override {
    called = true;
    observed_wparam = wparam;
    observed_lparam = lparam;
    result = 0x1234;
    return handled;
  }

  bool called = false;
  bool handled = true;
  WPARAM observed_wparam = 0;
  LPARAM observed_lparam = 0;
};

} // namespace

int main() {
  TestAccessibilityTarget target;
  LRESULT result = 0;
  if (cgpui::win32_window_proc_handle_accessibility(
          WM_PAINT, 7, 9, &target, result) || target.called) return 1;
  if (cgpui::win32_window_proc_handle_accessibility(
          WM_GETOBJECT, 7, UiaRootObjectId, nullptr, result)) return 2;
  if (!cgpui::win32_window_proc_handle_accessibility(
          WM_GETOBJECT, 7, UiaRootObjectId, &target, result) ||
      !target.called || target.observed_wparam != 7 ||
      target.observed_lparam != UiaRootObjectId || result != 0x1234) return 3;
  target.handled = false;
  if (cgpui::win32_window_proc_handle_accessibility(
          WM_GETOBJECT, 0, UiaRootObjectId, &target, result)) return 4;

  HWND hwnd = CreateWindowExW(
      0, L"STATIC", L"CGPUI UIA get object test", WS_OVERLAPPED,
      0, 0, 200, 100, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (hwnd == nullptr) return 5;
  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.attach(hwnd);
  if (adapter.handle_get_object(0, UiaRootObjectId, result)) return 6;
  adapter.update(cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 42,
      .node_count = 1,
      .nodes = {{.element_id = 42,
                 .role = cgpui::PlatformAccessibilityRole::generic,
                 .name = "Root",
                 .bounds = cgpui::Rect{{0.0F, 0.0F}, {200.0F, 100.0F}}}}});
  if (adapter.handle_get_object(0, OBJID_CLIENT, result) ||
      !adapter.handle_get_object(0, UiaRootObjectId, result)) return 7;
  adapter.detach();
  DestroyWindow(hwnd);
  return 0;
}
