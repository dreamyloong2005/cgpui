#include "text_glyph_coverage_win32_internal.hpp"

#include <wrl/client.h>

#include <algorithm>
#include <atomic>
#include <utility>
#include <vector>

namespace cgpui {
namespace {
using Microsoft::WRL::ComPtr;

class CoverageRenderer final : public IDWriteTextRenderer {
 public:
  CoverageRenderer(
      IDWriteFactory* factory,
      float scale,
      DWRITE_MATRIX transform,
      std::uint32_t width,
      std::uint32_t height)
      : factory_(factory), scale_(scale), transform_(transform) {
    bitmap.width = width;
    bitmap.height = height;
    bitmap.stride = width;
    bitmap.alpha.resize(static_cast<std::size_t>(width) * height);
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) override {
    if (result == nullptr) return E_POINTER;
    *result = nullptr;
    if (iid != __uuidof(IUnknown) && iid != __uuidof(IDWritePixelSnapping) &&
        iid != __uuidof(IDWriteTextRenderer)) return E_NOINTERFACE;
    *result = static_cast<IDWriteTextRenderer*>(this);
    AddRef();
    return S_OK;
  }
  ULONG STDMETHODCALLTYPE AddRef() override { return ++references_; }
  ULONG STDMETHODCALLTYPE Release() override {
    const ULONG remaining = --references_;
    if (remaining == 0) delete this;
    return remaining;
  }
  HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(void*, BOOL* disabled) override {
    if (disabled == nullptr) return E_POINTER;
    *disabled = TRUE;
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE GetCurrentTransform(void*, DWRITE_MATRIX* matrix) override {
    if (matrix == nullptr) return E_POINTER;
    *matrix = transform_;
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void*, FLOAT* scale) override {
    if (scale == nullptr) return E_POINTER;
    *scale = scale_;
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE DrawGlyphRun(
      void*, FLOAT x, FLOAT y, DWRITE_MEASURING_MODE measuring_mode,
      const DWRITE_GLYPH_RUN* glyph_run,
      const DWRITE_GLYPH_RUN_DESCRIPTION*, IUnknown*) override {
    try {
      ComPtr<IDWriteGlyphRunAnalysis> analysis;
      HRESULT hr = factory_->CreateGlyphRunAnalysis(
          glyph_run, scale_, &transform_, DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC,
          measuring_mode, x, y, analysis.GetAddressOf());
      if (FAILED(hr)) return hr;
      RECT bounds{};
      hr = analysis->GetAlphaTextureBounds(DWRITE_TEXTURE_CLEARTYPE_3x1, &bounds);
      if (FAILED(hr)) return hr;
      bounds.left = std::max<LONG>(0, bounds.left);
      bounds.top = std::max<LONG>(0, bounds.top);
      bounds.right = std::min<LONG>(bitmap.width, bounds.right);
      bounds.bottom = std::min<LONG>(bitmap.height, bounds.bottom);
      if (bounds.right <= bounds.left || bounds.bottom <= bounds.top) return S_OK;
      const auto width = static_cast<std::size_t>(bounds.right - bounds.left);
      const auto height = static_cast<std::size_t>(bounds.bottom - bounds.top);
      std::vector<std::uint8_t> samples(width * height * 3);
      hr = analysis->CreateAlphaTexture(DWRITE_TEXTURE_CLEARTYPE_3x1, &bounds,
          samples.data(), static_cast<UINT32>(samples.size()));
      if (FAILED(hr)) return hr;
      for (LONG py = bounds.top; py < bounds.bottom; ++py) {
        for (LONG px = bounds.left; px < bounds.right; ++px) {
          const auto sample = ((py - bounds.top) * width + px - bounds.left) * 3;
          // Collapse ClearType channels to grayscale coverage. Atlas alpha
          // must not contain a colored fringe or an opaque background.
          const unsigned alpha = (static_cast<unsigned>(samples[sample]) +
              samples[sample + 1] + samples[sample + 2] + 1) / 3;
          auto& pixel = bitmap.alpha[static_cast<std::size_t>(py) * bitmap.stride + px];
          pixel = static_cast<std::uint8_t>(alpha + (pixel * (255 - alpha) + 127) / 255);
        }
      }
      return S_OK;
    } catch (...) {
      return E_OUTOFMEMORY;
    }
  }
  HRESULT STDMETHODCALLTYPE DrawUnderline(
      void*, FLOAT, FLOAT, const DWRITE_UNDERLINE*, IUnknown*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE DrawStrikethrough(
      void*, FLOAT, FLOAT, const DWRITE_STRIKETHROUGH*, IUnknown*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE DrawInlineObject(
      void*, FLOAT, FLOAT, IDWriteInlineObject*, BOOL, BOOL, IUnknown*) override {
    return E_NOTIMPL;
  }

  GlyphBitmap bitmap;

 private:
  std::atomic<ULONG> references_{1};
  ComPtr<IDWriteFactory> factory_;
  float scale_;
  DWRITE_MATRIX transform_;
};
} // namespace

Result<GlyphBitmap> win32_text_glyph_coverage(
    IDWriteFactory* factory,
    IDWriteTextLayout* layout,
    float scale,
    float horizontal_fit,
    float vertical_fit,
    std::uint32_t width,
    std::uint32_t height) {
  ComPtr<CoverageRenderer> renderer;
  renderer.Attach(new CoverageRenderer(factory, scale,
      {horizontal_fit, 0, 0, vertical_fit, 0, 0}, width, height));
  if (FAILED(layout->Draw(nullptr, renderer.Get(), 0, 0))) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "DirectWrite glyph coverage generation failed",
    });
  }
  return std::move(renderer->bitmap);
}

} // namespace cgpui
