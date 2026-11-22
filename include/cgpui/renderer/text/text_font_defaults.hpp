#pragma once

#include "cgpui/core/font.hpp"

#include <string>
#include <string_view>

namespace cgpui {

// Windows: "Segoe UI" (CJK through DirectWrite system fallback, e.g.
// "Microsoft YaHei UI"); Linux: fontconfig "sans-serif".
[[nodiscard]] FontDescriptor default_ui_font_descriptor();

// Windows: "Cascadia Mono" when installed, else "Consolas"; Linux:
// fontconfig "monospace". Ligatures (`liga`, `calt`) are disabled.
[[nodiscard]] FontDescriptor default_code_font_descriptor();

// Maps an empty or generic family ("sans-serif", "system-ui", "monospace")
// to the concrete platform family; other names are returned unchanged.
[[nodiscard]] std::string resolve_platform_font_family(std::string_view family);

} // namespace cgpui
