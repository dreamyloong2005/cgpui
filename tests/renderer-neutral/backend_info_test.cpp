#include "../../src/renderer/renderer_selection_metadata_internal.hpp"

namespace {
class DefaultRenderer : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unique_ptr<cgpui::RenderFrame>{};
  }
};

class ProbeRenderer final : public DefaultRenderer {
 public:
  cgpui::RendererBackendInfo backend_info() const override {
    return {.backend = cgpui::RendererBackend::vulkan, .adapter_name = "adapter",
            .vendor_id = 22, .adapter_luid = 55, .software_adapter = true,
            .selection = cgpui::RendererBackendSelection::vulkan,
            .fallback_reason = "backend reason", .device_generation = generation};
  }
  cgpui::Result<void> resize(cgpui::Size size, cgpui::DpiScale scale) override {
    last_size = size;
    last_scale = scale;
    return std::unexpected(cgpui::Error{cgpui::ErrorCode::frame_retry_required, "resize"});
  }
  const cgpui::RendererFrameDiagnosticSnapshot*
  last_frame_diagnostic_snapshot() const override { return &snapshot; }
  const cgpui::RendererFramePixels* last_frame_pixels() const override { return &pixels; }
  cgpui::RendererSurfaceComposition surface_composition() const override {
    return {.supported = true, .transparent_requested = true,
            .supported_composite_alpha = 4,
            .composite_alpha = cgpui::RendererCompositeAlpha::post_multiplied};
  }
  cgpui::RendererEffectSupport backdrop_blur_support() const override {
    return {.supported = true, .reason = "probe blur"};
  }
  bool popup_windows_supported() const override { return true; }
  cgpui::Result<cgpui::RendererSurfaceComposition> set_transparent_surface(bool value) override {
    last_transparent = value;
    return surface_composition();
  }
  cgpui::RendererFrameDiagnosticSnapshot snapshot;
  cgpui::RendererFramePixels pixels;
  cgpui::Size last_size;
  cgpui::DpiScale last_scale;
  bool last_transparent = false;
  std::uint64_t generation = 3;
};
} // namespace

int main() {
  const DefaultRenderer default_renderer;
  const auto unknown = default_renderer.backend_info();
  if (unknown.backend != cgpui::RendererBackend::unknown ||
      !unknown.adapter_name.empty() || unknown.vendor_id != 0 ||
      unknown.adapter_luid || unknown.software_adapter ||
      unknown.selection != cgpui::RendererBackendSelection::automatic ||
      !unknown.fallback_reason.empty() || unknown.device_generation != 0) return 1;
  auto backend = std::make_unique<ProbeRenderer>();
  auto* probe = backend.get();
  auto selected = cgpui::detail::renderer_with_selection_metadata(
      std::move(backend), cgpui::RendererBackendSelection::automatic);
  const auto info = selected->backend_info();
  if (info.backend != cgpui::RendererBackend::vulkan || info.adapter_name != "adapter" ||
      info.vendor_id != 22 || info.adapter_luid != 55 || !info.software_adapter ||
      info.selection != cgpui::RendererBackendSelection::automatic ||
      info.fallback_reason != "backend reason" || info.device_generation != 3) return 2;
  ++probe->generation;
  if (selected->backend_info().device_generation != 4) return 3;
  const auto resized = selected->resize({123, 456}, {1.5F});
  if (resized || resized.error().code != cgpui::ErrorCode::frame_retry_required ||
      resized.error().message != "resize" || probe->last_size.width != 123 ||
      probe->last_size.height != 456 || probe->last_scale.value != 1.5F) return 4;
  if (selected->last_frame_diagnostic_snapshot() != &probe->snapshot ||
      selected->last_frame_pixels() != &probe->pixels) return 5;
  const auto composition = selected->surface_composition();
  if (!composition.supported || !composition.transparent_requested ||
      composition.supported_composite_alpha != 4 ||
      composition.composite_alpha != cgpui::RendererCompositeAlpha::post_multiplied) return 6;
  const auto blur = selected->backdrop_blur_support();
  if (!blur.supported || blur.reason != "probe blur" ||
      !selected->popup_windows_supported()) return 7;
  if (!selected->set_transparent_surface(true) || !probe->last_transparent) return 8;
  return 0;
}
