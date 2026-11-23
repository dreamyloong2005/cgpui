#include "dx12_runtime_capture_test_support.hpp"

#include "cgpui/renderer/renderer_backend.hpp"

#include <utility>

namespace cgpui_test {
namespace {

class CaptureRenderer final : public cgpui::Renderer {
 public:
  CaptureRenderer(std::unique_ptr<cgpui::Renderer> renderer,
                  Dx12RuntimeCaptureCounts& counts)
      : renderer_(std::move(renderer)), counts_(counts) {}

  cgpui::Result<void> resize(cgpui::Size size, cgpui::DpiScale scale) override {
    ++counts_.resize_attempts;
    auto result = renderer_->resize(size, scale);
    if (!result && result.error().code == cgpui::ErrorCode::frame_retry_required)
      ++counts_.resize_retries;
    return result;
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    ++counts_.begin_attempts;
    auto frame = renderer_->begin_frame();
    if (!frame) {
      if (frame.error().code == cgpui::ErrorCode::frame_retry_required)
        ++counts_.begin_retries;
      else if (frame.error().code == cgpui::ErrorCode::frame_not_ready)
        ++counts_.begin_pending;
      else
        ++counts_.begin_errors;
      return std::unexpected(frame.error());
    }
    // Runtime receives the SAME native frame; no present/result interception.
    if (auto capture = (*frame)->request_pixel_capture(); !capture) {
      ++counts_.capture_errors;
      return std::unexpected(capture.error());
    }
    return std::move(*frame);
  }

  cgpui::RendererBackendInfo backend_info() const override {
    return renderer_->backend_info();
  }
  const cgpui::RendererFrameDiagnosticSnapshot*
  last_frame_diagnostic_snapshot() const override {
    return renderer_->last_frame_diagnostic_snapshot();
  }
  const cgpui::RendererFramePixels* last_frame_pixels() const override {
    return renderer_->last_frame_pixels();
  }
  cgpui::RendererSurfaceComposition surface_composition() const override {
    return renderer_->surface_composition();
  }
  cgpui::RendererEffectSupport backdrop_blur_support() const override {
    return renderer_->backdrop_blur_support();
  }
  bool popup_windows_supported() const override {
    return renderer_->popup_windows_supported();
  }
  cgpui::Result<cgpui::RendererSurfaceComposition> set_transparent_surface(
      bool transparent) override {
    return renderer_->set_transparent_surface(transparent);
  }

 private:
  std::unique_ptr<cgpui::Renderer> renderer_;
  Dx12RuntimeCaptureCounts& counts_;
};

} // namespace

cgpui::Result<std::unique_ptr<cgpui::Renderer>> dx12_runtime_capture_renderer(
    const cgpui::RenderSurfaceDescriptor& descriptor,
    Dx12RuntimeCaptureCounts& counts) {
  if (const auto* native = std::get_if<cgpui::Win32SurfaceHandle>(&descriptor.native_surface))
    counts.hwnd = native->hwnd;
  auto renderer = cgpui::create_dx12_renderer(descriptor);
  if (!renderer) return std::unexpected(renderer.error());
  return std::make_unique<CaptureRenderer>(std::move(*renderer), counts);
}

} // namespace cgpui_test
