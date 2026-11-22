#pragma once

#include "win32_composition_presenter_internal.hpp"

#include <d3d11.h>
#include <dcomp.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

namespace cgpui {

template <typename T>
using Win32ComPtr = Microsoft::WRL::ComPtr<T>;

// How long present() waits for the backend to release the present key.
inline constexpr DWORD win32_composition_acquire_timeout_ms = 1000;

struct Win32CompositionObjects {
  HWND window = nullptr;
  Win32CompositionSize size;
  Win32ComPtr<ID3D11Device> device;
  Win32ComPtr<ID3D11DeviceContext> context;
  Win32ComPtr<IDXGISwapChain1> swapchain;
  Win32ComPtr<IDCompositionDevice> composition;
  Win32ComPtr<IDCompositionTarget> target;
  Win32ComPtr<IDCompositionVisual> visual;
  Win32ComPtr<ID3D11Texture2D> texture;
  Win32ComPtr<IDXGIKeyedMutex> mutex;
  HANDLE handle = nullptr;  // NT handle of `texture`, closed with it
  bool committed = false;
};

// "<call> failed (HRESULT 0x...)".
[[nodiscard]] Error win32_composition_error(const char* call, HRESULT result);
// The D3D11 device and immediate context on the adapter with `adapter`.
Result<void> win32_composition_create_device(
    LUID adapter, Win32CompositionObjects& objects);
// The premultiplied composition swapchain of objects.size, and the DComp
// device, target and visual that show it on objects.window.
Result<void> win32_composition_create_swapchain(Win32CompositionObjects& objects);
// Replaces the shared texture (and its handle) with one of objects.size.
Result<void> win32_composition_create_texture(Win32CompositionObjects& objects);
void win32_composition_release_texture(Win32CompositionObjects& objects);

} // namespace cgpui
