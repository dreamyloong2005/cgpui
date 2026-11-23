#include "dx12_runtime_pacing_test_support.hpp"
#include <utility>

namespace cgpui_test {
namespace {
class PacingCaptureRenderer final : public cgpui::Renderer {
 public:
  PacingCaptureRenderer(std::unique_ptr<cgpui::Renderer> renderer, Dx12RuntimePacingWindow& window)
      : renderer_(std::move(renderer)), window_(window) {}
  cgpui::Result<void> resize(cgpui::Size size, cgpui::DpiScale scale) override {
    ++window_.resizes;
    auto resized = renderer_->resize(size, scale);
    if (resized) {
      window_.width = static_cast<unsigned>(size.width);
      window_.height = static_cast<unsigned>(size.height);
    } else ++window_.errors;
    return resized;
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    ++window_.begins;
    auto frame = renderer_->begin_frame(); // Exactly one real call; NEVER wait/retry here.
    if (!frame) {
      if (frame.error().code == cgpui::ErrorCode::frame_not_ready) ++window_.pending;
      else ++window_.errors;
      return std::unexpected(frame.error());
    }
    if (!*frame) {
      ++window_.errors;
      return std::unexpected(cgpui::Error{.code = cgpui::ErrorCode::frame_acquisition_failed,
          .message = "native pacing observer received a null frame"});
    }
    if (auto captured = (*frame)->request_pixel_capture(); !captured) {
      ++window_.errors;
      return std::unexpected(captured.error());
    }
    ++window_.captures;
    return std::move(*frame); // The SAME native frame, with no Present interceptor.
  }
  cgpui::RendererBackendInfo backend_info() const override { return renderer_->backend_info(); }
  const cgpui::RendererFramePixels* last_frame_pixels() const override {
    return renderer_->last_frame_pixels();
  }
  const cgpui::RendererFrameDiagnosticSnapshot* last_frame_diagnostic_snapshot() const override {
    return renderer_->last_frame_diagnostic_snapshot();
  }
  cgpui::RendererSurfaceComposition surface_composition() const override {
    return renderer_->surface_composition();
  }
  cgpui::RendererEffectSupport backdrop_blur_support() const override {
    return renderer_->backdrop_blur_support();
  }
  bool popup_windows_supported() const override { return renderer_->popup_windows_supported(); }
  cgpui::Result<cgpui::RendererSurfaceComposition> set_transparent_surface(bool transparent) override {
    return renderer_->set_transparent_surface(transparent);
  }
 private:
  std::unique_ptr<cgpui::Renderer> renderer_;
  Dx12RuntimePacingWindow& window_;
};
}

cgpui::Result<std::unique_ptr<cgpui::Renderer>> dx12_runtime_pacing_capture(
    const cgpui::RenderSurfaceDescriptor& descriptor, Dx12RuntimePacingWindow& window) {
  const auto* native = std::get_if<cgpui::Win32SurfaceHandle>(&descriptor.native_surface);
  if (!native || !native->hwnd || native->composition || descriptor.transparent_background)
    return std::unexpected(cgpui::Error{.code = cgpui::ErrorCode::renderer_initialization_failed,
        .message = "pacing runtime test requires actual opaque Win32 HWNDs"});
  window.hwnd = static_cast<HWND>(native->hwnd);
  window.width = static_cast<unsigned>(descriptor.framebuffer_size.width);
  window.height = static_cast<unsigned>(descriptor.framebuffer_size.height);
  auto renderer = cgpui::dx12_create_renderer(descriptor, window.api);
  if (!renderer) return std::unexpected(renderer.error());
  window.initial = (*renderer)->backend_info();
  return std::make_unique<PacingCaptureRenderer>(std::move(*renderer), window);
}
} // namespace cgpui_test
