#include "text_dwrite_win32_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

// Stack-owned analysis source and sink; COM reference counts are unused.
class Analysis final : public IDWriteTextAnalysisSource,
                       public IDWriteTextAnalysisSink {
 public:
  Analysis(const std::wstring& text, const std::wstring& locale, bool rtl)
      : text_(text), locale_(locale), rtl_(rtl),
        scripts_(text.size()), levels_(text.size(), rtl ? 1U : 0U) {}

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) override {
    if (result == nullptr) return E_POINTER;
    *result = nullptr;
    if (iid == __uuidof(IDWriteTextAnalysisSource)) {
      *result = static_cast<IDWriteTextAnalysisSource*>(this);
    } else if (iid == __uuidof(IDWriteTextAnalysisSink)) {
      *result = static_cast<IDWriteTextAnalysisSink*>(this);
    } else if (iid == __uuidof(IUnknown)) {
      *result = static_cast<IDWriteTextAnalysisSource*>(this);
    } else {
      return E_NOINTERFACE;
    }
    return S_OK;
  }
  ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
  ULONG STDMETHODCALLTYPE Release() override { return 1; }

  HRESULT STDMETHODCALLTYPE GetTextAtPosition(
      UINT32 position, WCHAR const** text, UINT32* length) override {
    const bool inside = position < text_.size();
    *text = inside ? text_.data() + position : nullptr;
    *length = inside ? static_cast<UINT32>(text_.size() - position) : 0U;
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE GetTextBeforePosition(
      UINT32 position, WCHAR const** text, UINT32* length) override {
    const bool inside = position > 0 && position <= text_.size();
    *text = inside ? text_.data() : nullptr;
    *length = inside ? position : 0U;
    return S_OK;
  }
  DWRITE_READING_DIRECTION STDMETHODCALLTYPE GetParagraphReadingDirection() override {
    return rtl_ ? DWRITE_READING_DIRECTION_RIGHT_TO_LEFT
                : DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
  }
  HRESULT STDMETHODCALLTYPE GetLocaleName(
      UINT32 position, UINT32* length, WCHAR const** locale) override {
    *length = static_cast<UINT32>(text_.size() - std::min<std::size_t>(position, text_.size()));
    *locale = locale_.c_str();
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE GetNumberSubstitution(
      UINT32 position, UINT32* length, IDWriteNumberSubstitution** substitution) override {
    *length = static_cast<UINT32>(text_.size() - std::min<std::size_t>(position, text_.size()));
    *substitution = nullptr;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE SetScriptAnalysis(
      UINT32 position, UINT32 length, DWRITE_SCRIPT_ANALYSIS const* script) override {
    for (UINT32 index = position; index < position + length && index < scripts_.size(); ++index) {
      scripts_[index] = *script;
    }
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE SetLineBreakpoints(
      UINT32, UINT32, DWRITE_LINE_BREAKPOINT const*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE SetBidiLevel(
      UINT32 position, UINT32 length, UINT8, UINT8 resolved) override {
    for (UINT32 index = position; index < position + length && index < levels_.size(); ++index) {
      levels_[index] = resolved;
    }
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE SetNumberSubstitution(
      UINT32, UINT32, IDWriteNumberSubstitution*) override { return S_OK; }

  [[nodiscard]] std::vector<DWriteItem> items() const {
    std::vector<DWriteItem> result;
    for (std::size_t index = 0; index < text_.size(); ++index) {
      const bool same = !result.empty() &&
          result.back().script.script == scripts_[index].script &&
          result.back().script.shapes == scripts_[index].shapes &&
          result.back().bidi_level == levels_[index];
      if (same) {
        result.back().length += 1U;
        continue;
      }
      result.push_back(DWriteItem{
          .start = static_cast<std::uint32_t>(index),
          .length = 1U,
          .script = scripts_[index],
          .bidi_level = levels_[index],
      });
    }
    return result;
  }

 private:
  const std::wstring& text_;
  const std::wstring& locale_;
  bool rtl_;
  std::vector<DWRITE_SCRIPT_ANALYSIS> scripts_;
  std::vector<std::uint8_t> levels_;
};

ComPtr<IDWriteFontFallback> build_fallback(bool prefer_simplified_chinese) {
  IDWriteFactory2* factory = dwrite_factory();
  ComPtr<IDWriteFontFallback> system;
  if (factory == nullptr || FAILED(factory->GetSystemFontFallback(system.GetAddressOf()))) {
    return {};
  }
  ComPtr<IDWriteFontFallbackBuilder> builder;
  ComPtr<IDWriteFontFallback> fallback;
  if (!prefer_simplified_chinese ||
      FAILED(factory->CreateFontFallbackBuilder(builder.GetAddressOf()))) {
    return system;
  }
  const DWRITE_UNICODE_RANGE ranges[]{
      {0x3000U, 0x303FU}, {0x3400U, 0x4DBFU}, {0x4E00U, 0x9FFFU},
      {0xF900U, 0xFAFFU}, {0xFF00U, 0xFFEFU}, {0x20000U, 0x2FA1FU},
  };
  const WCHAR* families[]{L"Microsoft YaHei UI"};
  if (FAILED(builder->AddMapping(ranges, static_cast<UINT32>(std::size(ranges)),
                                 families, 1U)) ||
      FAILED(builder->AddMappings(system.Get())) ||
      FAILED(builder->CreateFontFallback(fallback.GetAddressOf()))) {
    return system;
  }
  return fallback;
}

IDWriteFontFallback* font_fallback(const std::wstring& locale) {
  static const ComPtr<IDWriteFontFallback> preferred = build_fallback(true);
  static const ComPtr<IDWriteFontFallback> system = build_fallback(false);
  const bool japanese_or_korean = locale.starts_with(L"ja") || locale.starts_with(L"ko");
  return japanese_or_korean ? system.Get() : preferred.Get();
}

} // namespace

std::vector<DWriteItem> dwrite_itemize(
    const std::wstring& text,
    const std::wstring& locale,
    bool right_to_left) {
  Analysis analysis(text, locale, right_to_left);
  IDWriteFactory2* factory = dwrite_factory();
  ComPtr<IDWriteTextAnalyzer> analyzer;
  if (factory != nullptr && !text.empty() &&
      SUCCEEDED(factory->CreateTextAnalyzer(analyzer.GetAddressOf()))) {
    const auto length = static_cast<UINT32>(text.size());
    (void)analyzer->AnalyzeScript(&analysis, 0, length, &analysis);
    (void)analyzer->AnalyzeBidi(&analysis, 0, length, &analysis);
  }
  return analysis.items();
}

std::vector<DWriteFallbackSegment> dwrite_map_fallback(
    const std::wstring& text,
    const std::wstring& locale,
    const DWriteItem& item,
    const std::wstring& family,
    IDWriteFont* base_font) {
  std::vector<DWriteFallbackSegment> segments;
  IDWriteFactory2* factory = dwrite_factory();
  IDWriteFontFallback* fallback = font_fallback(locale);
  ComPtr<IDWriteFontCollection> collection;
  if (factory == nullptr || fallback == nullptr ||
      FAILED(factory->GetSystemFontCollection(collection.GetAddressOf()))) {
    segments.push_back({item.start, item.length, base_font, 1.0F});
    return segments;
  }
  Analysis source(text, locale, (item.bidi_level & 1U) != 0U);
  std::uint32_t position = item.start;
  const std::uint32_t end = item.start + item.length;
  while (position < end) {
    UINT32 mapped = 0;
    ComPtr<IDWriteFont> font;
    FLOAT scale = 1.0F;
    const HRESULT result = fallback->MapCharacters(
        &source, position, end - position, collection.Get(), family.c_str(),
        base_font->GetWeight(), base_font->GetStyle(), base_font->GetStretch(),
        &mapped, font.GetAddressOf(), &scale);
    if (FAILED(result) || mapped == 0) {
      segments.push_back({position, end - position, nullptr, 1.0F});
      break;
    }
    segments.push_back({position, mapped, font, scale > 0.0F ? scale : 1.0F});
    position += mapped;
  }
  return segments;
}

} // namespace cgpui
