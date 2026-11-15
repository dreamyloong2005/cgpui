#include "text_glyph_raster_win32_internal.hpp"
#include "text_glyph_coverage_win32_internal.hpp"

#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace cgpui {
namespace {
using Microsoft::WRL::ComPtr;

Error native_error(std::string message) {
  return {.code = ErrorCode::renderer_initialization_failed,
          .message = "DirectWrite glyph rasterization: " + std::move(message)};
}

Result<std::wstring> wide_text(std::string_view text) {
  if (text.empty()) return std::wstring{};
  if (text.size() > 16384) return std::unexpected(native_error("UTF-8 input exceeds limit"));
  const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
      text.data(), static_cast<int>(text.size()), nullptr, 0);
  if (length <= 0) return std::unexpected(native_error("invalid UTF-8"));
  std::wstring output(static_cast<std::size_t>(length), L'\0');
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
      static_cast<int>(text.size()), output.data(), length) != length)
    return std::unexpected(native_error("UTF-8 conversion failed"));
  return output;
}

struct Factory {
  ComPtr<IDWriteFactory> value;
  HRESULT status = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
      __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(value.GetAddressOf()));
};

void crop_to_ink(RasterizedGlyph& glyph) {
  const auto& bitmap = glyph.bitmap;
  std::uint32_t left = bitmap.width, top = bitmap.height, right = 0, bottom = 0;
  for (std::uint32_t y = 0; y < bitmap.height; ++y) {
    for (std::uint32_t x = 0; x < bitmap.width; ++x) {
      if (bitmap.pixel(x, y) == 0) continue;
      left = std::min(left, x); top = std::min(top, y);
      right = std::max(right, x + 1); bottom = std::max(bottom, y + 1);
    }
  }
  if (right <= left || bottom <= top) {
    glyph.bitmap = {.width = 1, .height = 1, .stride = 1, .alpha = {0}};
    return;
  }
  GlyphBitmap cropped{.width = right - left, .height = bottom - top, .stride = right - left};
  cropped.alpha.resize(static_cast<std::size_t>(cropped.width) * cropped.height);
  for (std::uint32_t y = 0; y < cropped.height; ++y) {
    const auto start = bitmap.alpha.begin() + static_cast<std::size_t>(top + y) * bitmap.stride + left;
    std::copy_n(start, cropped.width, cropped.alpha.begin() + static_cast<std::size_t>(y) * cropped.stride);
  }
  glyph.bitmap = std::move(cropped);
  glyph.left_bearing = static_cast<float>(left);
  glyph.top_bearing = static_cast<float>(top);
}
} // namespace

Result<RasterizedGlyph> rasterize_win32_text_glyph(
    const TextGlyphPaint& glyph,
    std::string_view utf8) {
  auto content = wide_text(utf8);
  auto family = wide_text(glyph.key.font_family.empty() ? "Segoe UI" : glyph.key.font_family);
  if (!content) return std::unexpected(content.error());
  if (!family) return std::unexpected(family.error());
  thread_local Factory factory;
  if (FAILED(factory.status)) return std::unexpected(native_error("factory creation failed"));
  ComPtr<IDWriteTextFormat> format;
  HRESULT hr = factory.value->CreateTextFormat(family->c_str(), nullptr,
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
      glyph.key.font_size, L"en-us", format.GetAddressOf());
  if (FAILED(hr)) return std::unexpected(native_error("format creation failed"));
  hr = format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
  if (FAILED(hr)) return std::unexpected(native_error("word wrapping configuration failed"));
  ComPtr<IDWriteTextLayout> layout;
  hr = factory.value->CreateTextLayout(content->data(), static_cast<UINT32>(content->size()),
      format.Get(), 32768.0F, 32768.0F, layout.GetAddressOf());
  if (FAILED(hr)) return std::unexpected(native_error("layout creation failed"));
  DWRITE_TEXT_METRICS metrics{};
  hr = layout->GetMetrics(&metrics);
  if (FAILED(hr)) return std::unexpected(native_error("layout metrics failed"));
  UINT32 line_count = 0;
  (void)layout->GetLineMetrics(nullptr, 0, &line_count);
  if (line_count == 0 || line_count > 16384)
    return std::unexpected(native_error("invalid line metrics"));
  std::vector<DWRITE_LINE_METRICS> lines(line_count);
  hr = layout->GetLineMetrics(lines.data(), line_count, &line_count);
  if (FAILED(hr)) return std::unexpected(native_error("line metrics failed"));

  const float scale = glyph.key.scale;
  const float horizontal_fit = metrics.widthIncludingTrailingWhitespace > 0
      ? std::min(1.0F, glyph.device_advance / (metrics.widthIncludingTrailingWhitespace * scale))
      : 1.0F;
  const float vertical_fit = metrics.height > 0
      ? glyph.key.device_font_size / (metrics.height * scale) : 1.0F;
  const float baseline = lines.front().baseline * vertical_fit * scale;
  if (glyph.device_advance == 0.0F) {
    return RasterizedGlyph{
        .key = glyph.key,
        .bitmap = {.width = 1, .height = 1, .stride = 1, .alpha = {0}},
        .advance = 0.0F,
        .device_font_size = glyph.key.device_font_size,
        .baseline = baseline,
    };
  }
  const auto width = static_cast<std::uint32_t>(std::max(1.0F, std::ceil(glyph.device_advance)));
  const auto height = static_cast<std::uint32_t>(std::max(1.0F, std::ceil(glyph.key.device_font_size)));
  auto bitmap = win32_text_glyph_coverage(factory.value.Get(), layout.Get(), scale,
      horizontal_fit, vertical_fit, width, height);
  if (!bitmap) return std::unexpected(bitmap.error());
  RasterizedGlyph output{
      .key = glyph.key,
      .bitmap = std::move(*bitmap),
      .advance = glyph.device_advance,
      .device_font_size = glyph.key.device_font_size,
      .baseline = baseline,
  };
  crop_to_ink(output);
  return output;
}

} // namespace cgpui
