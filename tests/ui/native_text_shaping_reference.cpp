#include "native_text_shaping_reference.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwrite.h>
#include <wrl/client.h>
#elif defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
#include <hb.h>
#include <hb-ot.h>
#endif

namespace native_text_test {

#if defined(_WIN32)
// Width of the same string laid out by IDWriteTextLayout (no wrapping).
float reference_layout_width(const cgpui::TextShapeRun& run) {
  using Microsoft::WRL::ComPtr;
  ComPtr<IDWriteFactory> factory;
  if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
          reinterpret_cast<IUnknown**>(factory.GetAddressOf())))) {
    return -1.0F;
  }
  const std::string& family = run.font_fallback_faces.front().font.family;
  std::wstring wide_family(family.begin(), family.end());
  std::wstring text(run.text.begin(), run.text.end());
  ComPtr<IDWriteTextFormat> format;
  ComPtr<IDWriteTextLayout> layout;
  DWRITE_TEXT_METRICS metrics{};
  if (FAILED(factory->CreateTextFormat(wide_family.c_str(), nullptr,
          DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
          DWRITE_FONT_STRETCH_NORMAL, run.font_size, L"en-us", format.GetAddressOf())) ||
      FAILED(factory->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.size()),
          format.Get(), 100000.0F, 1000.0F, layout.GetAddressOf())) ||
      FAILED(layout->GetMetrics(&metrics))) {
    return -1.0F;
  }
  return metrics.widthIncludingTrailingWhitespace;
}
#elif defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
// Width from hb_shape over the whole string with the resolved primary face.
float reference_layout_width(const cgpui::TextShapeRun& run) {
  const cgpui::FontFaceDescriptor& face = run.font_fallback_faces.front();
  hb_blob_t* blob = hb_blob_create_from_file(face.path.c_str());
  hb_face_t* hb_face = hb_face_create(blob, face.face_index & 0xFFFFU);
  hb_font_t* font = hb_font_create(hb_face);
  const int scale = static_cast<int>(run.font_size * 64.0F);
  hb_font_set_scale(font, scale, scale);
  hb_ot_font_set_funcs(font);
  hb_buffer_t* buffer = hb_buffer_create();
  hb_buffer_add_utf8(buffer, run.text.data(), static_cast<int>(run.text.size()), 0, -1);
  hb_buffer_guess_segment_properties(buffer);
  hb_shape(font, buffer, nullptr, 0);
  unsigned int count = 0;
  const hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, &count);
  float width = 0.0F;
  for (unsigned int index = 0; index < count; ++index) {
    width += static_cast<float>(positions[index].x_advance) / 64.0F;
  }
  hb_buffer_destroy(buffer);
  hb_font_destroy(font);
  hb_face_destroy(hb_face);
  hb_blob_destroy(blob);
  return width;
}
#else
float reference_layout_width(const cgpui::TextShapeRun&) {
  return -1.0F;
}
#endif

} // namespace native_text_test
