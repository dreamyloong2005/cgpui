#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/renderer/planning.hpp"
#include "cgpui/renderer/renderer_backend_info.hpp"
#include "cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp"
#include "cgpui/renderer/renderer_frame_pixels.hpp"
#include "cgpui/renderer/renderer_surface_composition.hpp"

#include <memory>
#include <span>
#include <vector>

namespace cgpui {

class RenderFrame {
 public:
  virtual ~RenderFrame() = default;
  virtual void clear(Color color) = 0;
  virtual void draw_rect(const SolidRect& rect) = 0;
  virtual void draw_rounded_rect(const RoundedRectDraw& rect) { (void)rect; }
  virtual void draw_box_shadow(const BoxShadowDraw& shadow) { (void)shadow; }
  // Backends without backdrop blur draw the tint only.
  virtual void draw_backdrop_blur(const BackdropBlurDraw& backdrop);
  virtual void draw_text(const TextDraw& text) { (void)text; }
  virtual void draw_text_selection(const TextSelectionDraw& selection) {
    (void)selection;
  }
  virtual void draw_text_caret(const TextCaretDraw& caret) { (void)caret; }
  virtual void upload_image(const ImageAsset& image) { (void)image; }
  virtual void invalidate_image(ImageAssetId asset_id) { (void)asset_id; }
  virtual void draw_image(const ImageDraw& image) { (void)image; }
  virtual Result<void> request_pixel_capture();
  virtual Result<void> present() = 0;
};

class Renderer {
 public:
  virtual ~Renderer() = default;

  virtual Result<void> resize(Size framebuffer_size, DpiScale scale) = 0;
  // frame_not_ready means no frame was acquired and no work submitted; callers
  // should arrange a future attempt without a busy loop. It is begin-only:
  // present/resize must not use it. frame_retry_required retains recovery semantics.
  virtual Result<std::unique_ptr<RenderFrame>> begin_frame() = 0;
  [[nodiscard]] virtual RendererBackendInfo backend_info() const;
  [[nodiscard]] virtual const RendererFrameDiagnosticSnapshot*
  last_frame_diagnostic_snapshot() const;
  [[nodiscard]] virtual const RendererFramePixels* last_frame_pixels() const;
  [[nodiscard]] virtual RendererSurfaceComposition surface_composition() const;
  [[nodiscard]] virtual RendererEffectSupport backdrop_blur_support() const;
  // Whether this backend supports independent transparent popup windows.
  // The runtime also requires platform support. Custom renderers opt in;
  // this does not select an OS-specific presentation mode.
  [[nodiscard]] virtual bool popup_windows_supported() const;
  // Recreates the presentation surface with or without alpha composition.
  virtual Result<RendererSurfaceComposition> set_transparent_surface(
      bool transparent);
};

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor);
} // namespace cgpui
