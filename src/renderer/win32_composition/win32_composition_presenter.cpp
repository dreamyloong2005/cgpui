#include "win32_composition_objects_internal.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {
namespace {

Win32CompositionSize at_least_one_pixel(Win32CompositionSize size) {
  return Win32CompositionSize{
      .width = std::max<UINT>(size.width, 1U), .height = std::max<UINT>(size.height, 1U)};
}

} // namespace

Win32CompositionPresenter::Win32CompositionPresenter(
    std::unique_ptr<Win32CompositionObjects> objects)
    : objects_(std::move(objects)) {}

Win32CompositionPresenter::~Win32CompositionPresenter() {
  win32_composition_release_texture(*objects_);
}

Result<std::unique_ptr<Win32CompositionPresenter>> Win32CompositionPresenter::create(
    LUID adapter, HWND window, Win32CompositionSize size) {
  if (window == nullptr || IsWindow(window) == FALSE) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "DirectComposition presentation needs a window"});
  }
  auto objects = std::make_unique<Win32CompositionObjects>();
  objects->window = window;
  objects->size = at_least_one_pixel(size);
  if (auto result = win32_composition_create_device(adapter, *objects); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = win32_composition_create_swapchain(*objects); !result) {
    return std::unexpected(result.error());
  }
  if (auto result = win32_composition_create_texture(*objects); !result) {
    return std::unexpected(result.error());
  }
  return std::unique_ptr<Win32CompositionPresenter>(
      new Win32CompositionPresenter(std::move(objects)));
}

Result<void> Win32CompositionPresenter::resize(Win32CompositionSize size) {
  Win32CompositionObjects& objects = *objects_;
  size = at_least_one_pixel(size);
  win32_composition_release_texture(objects);
  if (size.width != objects.size.width || size.height != objects.size.height) {
    // The visual keeps the swapchain as its content across ResizeBuffers.
    if (const HRESULT result = objects.swapchain->ResizeBuffers(
            0, size.width, size.height, DXGI_FORMAT_UNKNOWN, 0);
        FAILED(result)) {
      return std::unexpected(win32_composition_error("IDXGISwapChain::ResizeBuffers", result));
    }
    objects.size = size;
  }
  return win32_composition_create_texture(objects);
}

HANDLE Win32CompositionPresenter::shared_texture_handle() const { return objects_->handle; }

Win32CompositionSize Win32CompositionPresenter::size() const { return objects_->size; }

Result<void> Win32CompositionPresenter::present() {
  Win32CompositionObjects& objects = *objects_;
  if (objects.mutex == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::frame_acquisition_failed,
        .message = "DirectComposition presentation has no shared texture"});
  }
  // WAIT_TIMEOUT and WAIT_ABANDONED are success codes; only S_OK owns it.
  if (const HRESULT acquired = objects.mutex->AcquireSync(
          win32_composition_present_key, win32_composition_acquire_timeout_ms);
      acquired != S_OK) {
    Error error = win32_composition_error("IDXGIKeyedMutex::AcquireSync", acquired);
    error.code = ErrorCode::frame_acquisition_failed;
    return std::unexpected(std::move(error));
  }
  Win32ComPtr<ID3D11Texture2D> back_buffer;
  const HRESULT buffer = objects.swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
  if (SUCCEEDED(buffer)) objects.context->CopyResource(back_buffer.Get(), objects.texture.Get());
  back_buffer.Reset();
  const HRESULT released = objects.mutex->ReleaseSync(win32_composition_render_key);
  if (FAILED(buffer)) {
    return std::unexpected(win32_composition_error("IDXGISwapChain::GetBuffer", buffer));
  }
  if (FAILED(released)) {
    return std::unexpected(win32_composition_error("IDXGIKeyedMutex::ReleaseSync", released));
  }
  if (const HRESULT presented = objects.swapchain->Present(1, 0); FAILED(presented)) {
    return std::unexpected(win32_composition_error("IDXGISwapChain::Present", presented));
  }
  if (!objects.committed) {
    if (const HRESULT committed = objects.composition->Commit(); FAILED(committed)) {
      return std::unexpected(win32_composition_error("IDCompositionDevice::Commit", committed));
    }
    objects.committed = true;
  }
  return {};
}

} // namespace cgpui
