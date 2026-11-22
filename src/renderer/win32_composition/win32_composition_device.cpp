#include "win32_composition_objects_internal.hpp"

#include <cstdio>
#include <string>

namespace cgpui {

Error win32_composition_error(const char* call, HRESULT result) {
  char code[16]{};
  std::snprintf(code, sizeof(code), "0x%08lX", static_cast<unsigned long>(result));
  return Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = std::string{call} + " failed (HRESULT " + code + ")"};
}

Result<void> win32_composition_create_device(
    LUID adapter_luid, Win32CompositionObjects& objects) {
  Win32ComPtr<IDXGIFactory4> factory;
  if (const HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
      FAILED(result)) {
    return std::unexpected(win32_composition_error("CreateDXGIFactory2", result));
  }
  Win32ComPtr<IDXGIAdapter1> adapter;
  if (const HRESULT result =
          factory->EnumAdapterByLuid(adapter_luid, IID_PPV_ARGS(&adapter));
      FAILED(result)) {
    return std::unexpected(
        win32_composition_error("IDXGIFactory4::EnumAdapterByLuid", result));
  }
  if (const HRESULT result = D3D11CreateDevice(
          adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr,
          D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION,
          &objects.device, nullptr, &objects.context);
      FAILED(result)) {
    return std::unexpected(win32_composition_error("D3D11CreateDevice", result));
  }
  return {};
}

Result<void> win32_composition_create_swapchain(Win32CompositionObjects& objects) {
  // The factory that owns the device's adapter creates its swapchains.
  Win32ComPtr<IDXGIDevice> dxgi_device;
  Win32ComPtr<IDXGIAdapter> adapter;
  Win32ComPtr<IDXGIFactory2> factory;
  if (const HRESULT result = objects.device.As(&dxgi_device); FAILED(result)) {
    return std::unexpected(win32_composition_error("ID3D11Device as IDXGIDevice", result));
  }
  if (const HRESULT result = dxgi_device->GetAdapter(&adapter); FAILED(result)) {
    return std::unexpected(win32_composition_error("IDXGIDevice::GetAdapter", result));
  }
  if (const HRESULT result = adapter->GetParent(IID_PPV_ARGS(&factory)); FAILED(result)) {
    return std::unexpected(win32_composition_error("IDXGIAdapter::GetParent", result));
  }
  const DXGI_SWAP_CHAIN_DESC1 description{
      .Width = objects.size.width,
      .Height = objects.size.height,
      .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .SampleDesc = {.Count = 1, .Quality = 0},
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      .BufferCount = 2,
      .Scaling = DXGI_SCALING_STRETCH,
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
      .AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED,
  };
  if (const HRESULT result = factory->CreateSwapChainForComposition(
          objects.device.Get(), &description, nullptr, &objects.swapchain);
      FAILED(result)) {
    return std::unexpected(win32_composition_error(
        "IDXGIFactory2::CreateSwapChainForComposition", result));
  }
  if (const HRESULT result = DCompositionCreateDevice(
          dxgi_device.Get(), IID_PPV_ARGS(&objects.composition));
      FAILED(result)) {
    return std::unexpected(win32_composition_error("DCompositionCreateDevice", result));
  }
  if (const HRESULT result = objects.composition->CreateTargetForHwnd(
          objects.window, TRUE, &objects.target);
      FAILED(result)) {
    return std::unexpected(win32_composition_error(
        "IDCompositionDevice::CreateTargetForHwnd", result));
  }
  if (const HRESULT result = objects.composition->CreateVisual(&objects.visual);
      FAILED(result)) {
    return std::unexpected(win32_composition_error("IDCompositionDevice::CreateVisual", result));
  }
  if (const HRESULT result = objects.visual->SetContent(objects.swapchain.Get());
      FAILED(result)) {
    return std::unexpected(win32_composition_error("IDCompositionVisual::SetContent", result));
  }
  if (const HRESULT result = objects.target->SetRoot(objects.visual.Get());
      FAILED(result)) {
    return std::unexpected(win32_composition_error("IDCompositionTarget::SetRoot", result));
  }
  return {};
}

bool win32_composition_available() {
  // Renderer bridge smoke-test only. Platform host support is independent,
  // and actual Vulkan interop validates its selected adapter separately.
  static const bool available = [] {
    Win32ComPtr<ID3D11Device> device;
    Win32ComPtr<IDXGIDevice> dxgi_device;
    Win32ComPtr<IDCompositionDevice> composition;
    return SUCCEEDED(D3D11CreateDevice(
               nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
               D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION,
               &device, nullptr, nullptr)) &&
           SUCCEEDED(device.As(&dxgi_device)) &&
           SUCCEEDED(DCompositionCreateDevice(
               dxgi_device.Get(), IID_PPV_ARGS(&composition)));
  }();
  return available;
}

} // namespace cgpui
