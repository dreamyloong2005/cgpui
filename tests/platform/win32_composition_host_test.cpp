#include "win32_composition_host_internal.hpp"
#include "cgpui/platform/platform_application.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dcomp.h>
#include <wrl/client.h>

#include <iostream>

int main() {
  // Host capability must not construct a D3D11 device. A null rendering
  // device supports targets/visuals/commit; renderer bridges probe separately.
  if (!cgpui::win32_composition_host_available()) return 1;
  auto application = cgpui::create_platform_application();
  if (!application || !(*application)->popup_windows_supported()) return 2;
  const HWND hwnd = CreateWindowExW(
      WS_EX_NOREDIRECTIONBITMAP, L"STATIC", L"CGPUI composition host test",
      WS_POPUP, 0, 0, 16, 16, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (hwnd == nullptr) return 3;
  HRESULT result;
  {
    Microsoft::WRL::ComPtr<IDCompositionDevice> device;
    Microsoft::WRL::ComPtr<IDCompositionTarget> target;
    Microsoft::WRL::ComPtr<IDCompositionVisual> visual;
    result = DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&device));
    if (SUCCEEDED(result)) result = device->CreateTargetForHwnd(hwnd, TRUE, &target);
    if (SUCCEEDED(result)) result = device->CreateVisual(&visual);
    if (SUCCEEDED(result)) result = target->SetRoot(visual.Get());
    if (SUCCEEDED(result)) result = device->Commit();
  }
  DestroyWindow(hwnd);
  if (FAILED(result)) {
    std::cerr << "composition host failed: " << std::hex << result << '\n';
    return 4;
  }
  return 0;
}
