#include "win32_composition_objects_internal.hpp"

namespace cgpui {

// Render target for a D3D backend, an importable D3D11 texture for Vulkan
// (the D3D11 texture external memory handle type), and a copy source for
// present().
Result<void> win32_composition_create_texture(Win32CompositionObjects& objects) {
  win32_composition_release_texture(objects);
  const D3D11_TEXTURE2D_DESC description{
      .Width = objects.size.width,
      .Height = objects.size.height,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
      .SampleDesc = {.Count = 1, .Quality = 0},
      .Usage = D3D11_USAGE_DEFAULT,
      .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
      .MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE |
                   D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX,
  };
  if (const HRESULT result =
          objects.device->CreateTexture2D(&description, nullptr, &objects.texture);
      FAILED(result)) {
    return std::unexpected(win32_composition_error("ID3D11Device::CreateTexture2D", result));
  }
  Win32ComPtr<IDXGIResource1> resource;
  HRESULT result = objects.texture.As(&resource);
  if (SUCCEEDED(result)) {
    result = resource->CreateSharedHandle(
        nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
        &objects.handle);
  }
  if (SUCCEEDED(result)) result = objects.texture.As(&objects.mutex);
  if (FAILED(result)) {
    win32_composition_release_texture(objects);
    return std::unexpected(
        win32_composition_error("IDXGIResource1::CreateSharedHandle", result));
  }
  return {};
}

void win32_composition_release_texture(Win32CompositionObjects& objects) {
  if (objects.handle != nullptr) CloseHandle(objects.handle);
  objects.handle = nullptr;
  objects.mutex.Reset();
  objects.texture.Reset();
}

} // namespace cgpui
