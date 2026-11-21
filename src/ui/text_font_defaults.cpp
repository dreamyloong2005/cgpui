#include "cgpui/ui/text_font_defaults.hpp"

#include "text_shaping_native_internal.hpp"

namespace cgpui {
namespace {

bool is_generic_sans(std::string_view family) {
  return family.empty() || family == "sans-serif" || family == "system-ui" ||
      family == "ui-sans-serif";
}

bool is_generic_monospace(std::string_view family) {
  return family == "monospace" || family == "ui-monospace";
}

} // namespace

std::string resolve_platform_font_family(std::string_view family) {
#if defined(_WIN32)
  if (is_generic_sans(family)) {
    return "Segoe UI";
  }
  if (is_generic_monospace(family)) {
    static const bool cascadia = native_font_family_installed("Cascadia Mono");
    return cascadia ? "Cascadia Mono" : "Consolas";
  }
  if (family == "serif" || family == "ui-serif") {
    return "Times New Roman";
  }
#elif defined(__APPLE__)
  // Core Text maps "system-ui" to the system UI font (San Francisco).
  if (is_generic_sans(family)) {
    return "system-ui";
  }
  if (is_generic_monospace(family)) {
    return "Menlo";
  }
  if (family == "serif" || family == "ui-serif") {
    return "Times New Roman";
  }
#else
  if (family.empty()) {
    return "sans-serif";
  }
  (void)is_generic_sans;
  (void)is_generic_monospace;
#endif
  return std::string(family);
}

FontDescriptor default_ui_font_descriptor() {
  return FontDescriptor{.family = resolve_platform_font_family("sans-serif")};
}

FontDescriptor default_code_font_descriptor() {
  return FontDescriptor{
      .family = resolve_platform_font_family("monospace"),
      .features = {font_feature("liga", 0), font_feature("calt", 0)},
  };
}

} // namespace cgpui
