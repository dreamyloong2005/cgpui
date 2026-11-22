// The composition module with D3D11 alone: a second D3D11 device
// (standing in for a GPU backend such as DX12) renders into the shared
// texture under the keyed mutex, and the presenter composites it with
// DirectComposition over a green window. Runs on every hardware adapter:
// premultiplied alpha blends with the window below, pixels outside the
// frame stay transparent (no black frame), a resize replaces the texture,
// and a present without a rendered frame times out instead of hanging.
#include "win32_composition/win32_composition_presenter_internal.hpp"
#include "win32_composition_screen_support.hpp"

#include <d3d11_1.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <chrono>
#include <iostream>
#include <vector>

namespace {

using Microsoft::WRL::ComPtr;

// What the stand-in backend does each frame: clear the whole texture.
bool render(LUID adapter_luid, HANDLE handle, const float (&premultiplied)[4]) {
  ComPtr<IDXGIFactory4> factory;
  ComPtr<IDXGIAdapter1> adapter;
  ComPtr<ID3D11Device> device;
  ComPtr<ID3D11DeviceContext> context;
  ComPtr<ID3D11Device1> device1;
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<IDXGIKeyedMutex> mutex;
  ComPtr<ID3D11RenderTargetView> target;
  if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory))) ||
      FAILED(factory->EnumAdapterByLuid(adapter_luid, IID_PPV_ARGS(&adapter))) ||
      FAILED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0,
                               D3D11_SDK_VERSION, &device, nullptr, &context)) ||
      FAILED(device.As(&device1)) ||
      FAILED(device1->OpenSharedResource1(handle, IID_PPV_ARGS(&texture))) ||
      FAILED(texture.As(&mutex)) ||
      mutex->AcquireSync(cgpui::win32_composition_render_key, 1000) != S_OK) {
    return false;
  }
  const bool cleared = SUCCEEDED(device->CreateRenderTargetView(texture.Get(), nullptr, &target));
  if (cleared) context->ClearRenderTargetView(target.Get(), premultiplied);
  context->Flush();
  return mutex->ReleaseSync(cgpui::win32_composition_present_key) == S_OK && cleared;
}

int run_on_adapter(const DXGI_ADAPTER_DESC1& adapter, bool check_timeout) {
  std::wcout << L"adapter: " << adapter.Description << L'\n';
  cgpui_test::CompositionTestWindows windows(400, 200);
  if (!windows.valid()) return 1;
  const auto started = std::chrono::steady_clock::now();
  auto presenter = cgpui::Win32CompositionPresenter::create(
      adapter.AdapterLuid, windows.popup(), {.width = 64, .height = 64});
  if (!presenter) {
    std::cerr << "create failed: " << presenter.error().message << '\n';
    return 2;
  }
  std::cout << "presenter created in "
            << std::chrono::duration<double, std::milli>(
                   std::chrono::steady_clock::now() - started).count()
            << " ms\n";
  if ((*presenter)->shared_texture_handle() == nullptr) return 3;
  // Premultiplied half-transparent red over green: 128, 127, 0.
  const float red[4]{128.0F / 255.0F, 0.0F, 0.0F, 128.0F / 255.0F};
  if (!render(adapter.AdapterLuid, (*presenter)->shared_texture_handle(), red)) return 4;
  if (auto presented = (*presenter)->present(); !presented) {
    std::cerr << "present failed: " << presented.error().message << '\n';
    return 5;
  }
  if (!windows.expect({0, 255, 0}, windows.backdrop_point()) ||
      !windows.expect({128, 127, 0}, windows.popup_point(32, 32))) {
    return 6;
  }
  // A smaller frame covers the top left; the rest of the popup shows the
  // green window through transparent pixels.
  if (auto resized = (*presenter)->resize({.width = 32, .height = 32}); !resized ||
      (*presenter)->size().width != 32 || (*presenter)->shared_texture_handle() == nullptr) {
    return 7;
  }
  const float blue[4]{0.0F, 0.0F, 128.0F / 255.0F, 128.0F / 255.0F};
  if (!render(adapter.AdapterLuid, (*presenter)->shared_texture_handle(), blue) ||
      !(*presenter)->present()) {
    return 8;
  }
  if (!windows.expect({0, 127, 128}, windows.popup_point(16, 16)) ||
      !windows.expect({0, 255, 0}, windows.popup_point(48, 48))) {
    return 9;
  }
  // No frame was rendered since: the present key never comes (checked on
  // the first adapter only; the wait takes a second).
  if (!check_timeout) return 0;
  auto stale = (*presenter)->present();
  return !stale && stale.error().code == cgpui::ErrorCode::frame_acquisition_failed ? 0 : 10;
}

} // namespace

int main() {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  if (!cgpui::win32_composition_available()) {
    std::cerr << "no hardware D3D11 or DirectComposition device\n";
    return 20;
  }
  ComPtr<IDXGIFactory1> factory;
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return 21;
  int adapters = 0;
  ComPtr<IDXGIAdapter1> adapter;
  for (UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index) {
    DXGI_ADAPTER_DESC1 description{};
    if (FAILED(adapter->GetDesc1(&description)) ||
        (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) {
      continue;
    }
    if (const int result = run_on_adapter(description, adapters++ == 0); result != 0) {
      std::cerr << "adapter " << index << " failed with " << result << '\n';
      return result;
    }
  }
  return adapters > 0 ? 0 : 22;
}
