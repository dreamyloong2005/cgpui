#include "text_coretext_macos_internal.hpp"

#include "text_shaping_native_internal.hpp"

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

// Owns one Core Foundation reference.
template <typename T>
struct Cf {
  T value = nullptr;
  explicit Cf(T held = nullptr) : value(held) {}
  ~Cf() { if (value != nullptr) CFRelease(value); }
  Cf(const Cf&) = delete;
  Cf& operator=(const Cf&) = delete;
  [[nodiscard]] T get() const { return value; }
};

std::string utf8(CFStringRef string) {
  if (string == nullptr) return {};
  const CFIndex length = CFStringGetLength(string);
  const CFIndex size =
      CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
  std::string result(static_cast<std::size_t>(size), '\0');
  if (!CFStringGetCString(string, result.data(), size, kCFStringEncodingUTF8)) {
    return {};
  }
  result.resize(std::char_traits<char>::length(result.c_str()));
  return result;
}

CFStringRef cf_string(std::string_view text) {
  return CFStringCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(text.data()),
                                 static_cast<CFIndex>(text.size()),
                                 kCFStringEncodingUTF8, false);
}

CFNumberRef cf_number(double value) {
  return CFNumberCreate(nullptr, kCFNumberDoubleType, &value);
}

// CSS weight to the Core Text weight trait (-1 .. 1), the values AppKit
// uses for NSFontWeightUltraLight .. NSFontWeightBlack.
double core_text_weight(std::uint16_t weight) {
  constexpr std::array<std::pair<double, double>, 9> table{{
      {100, -0.8}, {200, -0.6}, {300, -0.4}, {400, 0.0}, {500, 0.23},
      {600, 0.3}, {700, 0.4}, {800, 0.56}, {900, 0.62}}};
  const double css = std::clamp<double>(weight, 100.0, 900.0);
  for (std::size_t index = 1; index < table.size(); ++index) {
    if (css <= table[index].first) {
      const auto [x0, y0] = table[index - 1];
      const auto [x1, y1] = table[index];
      return y0 + (y1 - y0) * (css - x0) / (x1 - x0);
    }
  }
  return table.back().second;
}

struct Registry {
  std::mutex mutex;
  std::vector<CTFontDescriptorRef> descriptors;  // retained, never released
  std::map<std::string, std::uint32_t> slots;
};

Registry& registry() {
  static Registry* value = new Registry;
  return *value;
}

std::string font_path(CTFontRef font) {
  Cf<CFURLRef> url(static_cast<CFURLRef>(CTFontCopyAttribute(font, kCTFontURLAttribute)));
  if (url.get() == nullptr) return {};
  Cf<CFStringRef> path(CFURLCopyFileSystemPath(url.get(), kCFURLPOSIXPathStyle));
  return utf8(path.get());
}

}  // namespace

std::uint32_t core_text_register_face(CTFontRef font) {
  Cf<CFStringRef> name(CTFontCopyPostScriptName(font));
  Cf<CFDictionaryRef> variation(CTFontCopyVariation(font));
  Cf<CFStringRef> axes(variation.get() == nullptr ? nullptr
                                                  : CFCopyDescription(variation.get()));
  std::string key = utf8(name.get()) + '|' + utf8(axes.get()) + '|' + font_path(font);
  Registry& faces = registry();
  const std::lock_guard lock(faces.mutex);
  if (const auto found = faces.slots.find(key); found != faces.slots.end()) {
    return (found->second + 1U) << 16U;
  }
  const auto slot = static_cast<std::uint32_t>(faces.descriptors.size());
  if (slot >= 0xFFFFU) return 0;
  faces.descriptors.push_back(CTFontCopyFontDescriptor(font));
  faces.slots.emplace(std::move(key), slot);
  return (slot + 1U) << 16U;
}

CTFontRef core_text_font_for_face(std::uint32_t face_index, float size) {
  const std::uint32_t slot = face_index >> 16U;
  Registry& faces = registry();
  const std::lock_guard lock(faces.mutex);
  if (slot == 0 || slot > faces.descriptors.size()) return nullptr;
  return CTFontCreateWithFontDescriptor(faces.descriptors[slot - 1U], size, nullptr);
}

CTFontRef core_text_primary_font(const FontDescriptor& font, float size) {
  const std::string family = font.family.empty() ? "system-ui" : font.family;
  Cf<CTFontRef> base(family == "system-ui"
                         ? CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, nullptr)
                         : nullptr);
  Cf<CFStringRef> family_name(cf_string(family));
  // Weight and slant traits, the family (unless it is the UI font), and
  // the OpenType features; liga/calt default off for monospaced faces.
  Cf<CFNumberRef> weight(cf_number(core_text_weight(font.weight)));
  const CTFontSymbolicTraits italic =
      font.style == FontStyle::italic ? kCTFontTraitItalic : 0;
  Cf<CFNumberRef> symbolic(CFNumberCreate(nullptr, kCFNumberSInt32Type, &italic));
  const void* trait_keys[]{kCTFontWeightTrait, kCTFontSymbolicTrait};
  const void* trait_values[]{weight.get(), symbolic.get()};
  Cf<CFDictionaryRef> traits(CFDictionaryCreate(
      nullptr, trait_keys, trait_values, 2, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks));
  CFMutableDictionaryRef raw = CFDictionaryCreateMutable(
      nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  Cf<CFMutableDictionaryRef> attributes(raw);
  CFDictionarySetValue(raw, kCTFontTraitsAttribute, traits.get());
  if (base.get() == nullptr) {
    CFDictionarySetValue(raw, kCTFontFamilyNameAttribute, family_name.get());
  }
  Cf<CTFontDescriptorRef> styled(CTFontDescriptorCreateWithAttributes(raw));
  Cf<CTFontRef> matched(base.get() != nullptr
                            ? CTFontCreateCopyWithAttributes(base.get(), size, nullptr,
                                                             styled.get())
                            : CTFontCreateWithFontDescriptor(styled.get(), size, nullptr));
  if (matched.get() == nullptr) return nullptr;
  const bool monospaced =
      (CTFontGetSymbolicTraits(matched.get()) & kCTFontTraitMonoSpace) != 0;
  const std::vector<FontFeature> features = native_font_features(font, monospaced);
  if (features.empty()) return static_cast<CTFontRef>(CFRetain(matched.get()));
  Cf<CFMutableArrayRef> settings(
      CFArrayCreateMutable(nullptr, 0, &kCFTypeArrayCallBacks));
  for (const FontFeature& feature : features) {
    Cf<CFStringRef> tag(cf_string(std::string_view(feature.tag.data(), 4)));
    const int value = static_cast<int>(feature.value);
    Cf<CFNumberRef> number(CFNumberCreate(nullptr, kCFNumberIntType, &value));
    const void* keys[]{kCTFontOpenTypeFeatureTag, kCTFontOpenTypeFeatureValue};
    const void* values[]{tag.get(), number.get()};
    Cf<CFDictionaryRef> setting(CFDictionaryCreate(
        nullptr, keys, values, 2, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks));
    CFArrayAppendValue(settings.get(), setting.get());
  }
  const void* keys[]{kCTFontFeatureSettingsAttribute};
  const void* values[]{settings.get()};
  Cf<CFDictionaryRef> feature_attributes(CFDictionaryCreate(
      nullptr, keys, values, 1, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks));
  Cf<CTFontDescriptorRef> featured(CTFontDescriptorCreateWithAttributes(feature_attributes.get()));
  return CTFontCreateCopyWithAttributes(matched.get(), size, nullptr, featured.get());
}

FontFaceDescriptor core_text_face_descriptor(
    CTFontRef font,
    const FontDescriptor& requested) {
  Cf<CFStringRef> family(CTFontCopyFamilyName(font));
  Cf<CFStringRef> name(CTFontCopyPostScriptName(font));
  const CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(font);
  FontFaceDescriptor face{
      .font = {.family = utf8(family.get()),
               .weight = requested.weight,
               .style = (traits & kCTFontTraitItalic) != 0 ? FontStyle::italic
                                                         : FontStyle::normal},
      .postscript_name = utf8(name.get()),
      .source = FontSource::platform,
      .path = font_path(font),
      .face_index = core_text_register_face(font),
      .monospaced = (traits & kCTFontTraitMonoSpace) != 0};
  return face;
}

TextFontMetrics core_text_font_metrics(CTFontRef font) {
  return TextFontMetrics{
      .ascent = static_cast<float>(CTFontGetAscent(font)),
      .descent = static_cast<float>(CTFontGetDescent(font)),
      .line_gap = static_cast<float>(CTFontGetLeading(font)),
      .cap_height = static_cast<float>(CTFontGetCapHeight(font)),
      .x_height = static_cast<float>(CTFontGetXHeight(font)),
      // Positive below the baseline, as on the other backends.
      .underline_position = -static_cast<float>(CTFontGetUnderlinePosition(font)),
      .underline_thickness = static_cast<float>(CTFontGetUnderlineThickness(font))};
}

bool native_font_family_installed(std::string_view family) {
  Cf<CFStringRef> name(cf_string(family));
  Cf<CFArrayRef> families(CTFontManagerCopyAvailableFontFamilyNames());
  return families.get() != nullptr &&
         CFArrayContainsValue(families.get(),
                              CFRangeMake(0, CFArrayGetCount(families.get())), name.get());
}

}  // namespace cgpui
