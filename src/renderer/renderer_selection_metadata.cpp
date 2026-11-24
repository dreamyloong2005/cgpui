#include "renderer_selection_metadata_internal.hpp"

#include <utility>

namespace cgpui::detail {
namespace {

// Only selection metadata changes; frame lifetime and every backend capability
// remain owned by the selected renderer.
class SelectedRenderer final : public Renderer {
 public:
  SelectedRenderer(std::unique_ptr<Renderer> renderer,
                   RendererBackendSelection selection, std::string fallback_reason)
      : renderer_(std::move(renderer)), selection_(selection), fallback_reason_(std::move(fallback_reason)) {}
  Result<void> resize(Size size, DpiScale scale) override {
    return renderer_->resize(size, scale);
  }
  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return renderer_->begin_frame();
  }
  RendererBackendInfo backend_info() const override {
    auto info = renderer_->backend_info();
    info.selection = selection_;
    if (!fallback_reason_.empty()) info.fallback_reason = fallback_reason_;
    return info;
  }
  const RendererFrameDiagnosticSnapshot*
  last_frame_diagnostic_snapshot() const override {
    return renderer_->last_frame_diagnostic_snapshot();
  }
  const RendererFramePixels* last_frame_pixels() const override {
    return renderer_->last_frame_pixels();
  }
  RendererSurfaceComposition surface_composition() const override {
    return renderer_->surface_composition();
  }
  RendererEffectSupport backdrop_blur_support() const override {
    return renderer_->backdrop_blur_support();
  }
  bool popup_windows_supported() const override {
    return renderer_->popup_windows_supported();
  }
  Result<RendererSurfaceComposition> set_transparent_surface(bool value) override {
    return renderer_->set_transparent_surface(value);
  }
 private:
  std::unique_ptr<Renderer> renderer_;
  RendererBackendSelection selection_;
  std::string fallback_reason_;
};

} // namespace

std::unique_ptr<Renderer> renderer_with_selection_metadata(
    std::unique_ptr<Renderer> renderer, RendererBackendSelection selection, std::string fallback_reason) {
  return std::make_unique<SelectedRenderer>(std::move(renderer), selection, std::move(fallback_reason));
}

} // namespace cgpui::detail
