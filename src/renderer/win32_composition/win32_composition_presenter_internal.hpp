#pragma once

#include "cgpui/core/error.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <memory>

namespace cgpui {

// Keys of the shared texture's keyed mutex. The GPU backend acquires the
// render key, renders into the texture and releases the present key;
// Win32CompositionPresenter::present acquires the present key and
// releases the render key. A new texture starts at the render key.
inline constexpr UINT64 win32_composition_render_key = 0;
inline constexpr UINT64 win32_composition_present_key = 1;

// Physical pixels; zero extents are raised to one pixel.
struct Win32CompositionSize {
  UINT width = 0;
  UINT height = 0;
};

struct Win32CompositionObjects;

// DirectComposition presentation of one window (popup windows, see
// docs/superpowers/plans/2026-11-22-popup-windows-windows-linux-follow-up.md):
// a D3D11 device on the GPU backend's adapter, a premultiplied
// composition swapchain shown by a DComp visual on the window, and a
// shared texture (B8G8R8A8_UNORM, premultiplied alpha, NT handle, keyed
// mutex) that the backend renders into. It holds no GPU backend code:
// Vulkan imports the handle (src/renderer/vulkan/vulkan_composition_*).
class Win32CompositionPresenter {
 public:
  // `adapter` is the LUID of the backend's GPU. The window should have no
  // redirection surface (WS_EX_NOREDIRECTIONBITMAP).
  static Result<std::unique_ptr<Win32CompositionPresenter>> create(
      LUID adapter, HWND window, Win32CompositionSize size);
  ~Win32CompositionPresenter();
  Win32CompositionPresenter(const Win32CompositionPresenter&) = delete;
  Win32CompositionPresenter& operator=(const Win32CompositionPresenter&) = delete;

  // Resizes the swapchain and replaces the shared texture and its handle.
  // The backend must be done with the previous texture.
  Result<void> resize(Win32CompositionSize size);
  // NT handle of the shared texture; the presenter owns and closes it.
  [[nodiscard]] HANDLE shared_texture_handle() const;
  [[nodiscard]] Win32CompositionSize size() const;
  // Acquires the present key, copies the texture into the back buffer,
  // releases the render key and presents. The first present commits the
  // visual, so the window shows nothing before the first frame.
  Result<void> present();

 private:
  explicit Win32CompositionPresenter(std::unique_ptr<Win32CompositionObjects> objects);
  std::unique_ptr<Win32CompositionObjects> objects_;
};

// True when a hardware D3D11 device and a DirectComposition device can be
// created (checked once per process). PlatformApplication's
// popup_windows_supported() on Win32 is this probe.
[[nodiscard]] bool win32_composition_available();

} // namespace cgpui
