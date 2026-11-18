#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// C8 module layout: IMM32 reads stay in the Win32 window IME files, the
// UTF-16 -> UTF-8 clause conversion is IMM-free, and the widget draws the
// composition from its own paint module.
namespace {

std::string read(const std::string& path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream stream(std::filesystem::path(root ? root : ".") / path);
  if (!stream) throw std::runtime_error("missing focused module: " + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}

void require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}

bool has(const std::string& source, std::string_view text) {
  return source.find(text) != std::string::npos;
}

std::size_t lines(const std::string& source) {
  return static_cast<std::size_t>(
             std::count(source.begin(), source.end(), '\n')) + 1U;
}

const std::vector<std::pair<std::string, std::size_t>> limits{
    {"include/cgpui/ui/text_composition.hpp", 50},
    {"src/ui/text_composition.cpp", 90},
    {"src/ui/text_composition_paint.hpp", 40},
    {"src/ui/text_composition_paint.cpp", 120},
    {"src/platform/win32/win32_ime_composition_detail_internal.hpp", 50},
    {"src/platform/win32/win32_ime_composition_detail.cpp", 150},
    {"src/platform/win32/win32_window_ime_read_internal.hpp", 30},
    {"src/platform/win32/win32_window_ime_detail.cpp", 40},
};

} // namespace

int main() {
  try {
    for (const auto& [path, limit] : limits) {
      require(lines(read(path)) <= limit,
              path + " exceeds " + std::to_string(limit) + " lines");
    }
    const std::string event = read("include/cgpui/core/event_text.hpp");
    require(has(event, "enum class ImeClauseStyle") &&
                has(event, "ImeClauseStyle clause") &&
                !has(event, "std::vector<"),
            "clause styles are fixed-capacity fields of the public event");
    const std::string header = read("include/cgpui/ui/text_composition.hpp");
    require(!has(header, "<windows.h>") && !has(header, "imm.h") &&
                !has(header, "wayland"),
            "the public composition header must not leak platform types");
    const std::string detail =
        read("src/platform/win32/win32_ime_composition_detail.cpp");
    require(!has(detail, "ImmGetCompositionStringW") &&
                !has(detail, "ImmGetContext") &&
                has(detail, "ATTR_TARGET_CONVERTED") &&
                has(detail, "ImeCompositionPhase::update"),
            "the IMM32 attribute conversion must stay free of IMM calls");
    const std::string reader =
        read("src/platform/win32/win32_window_ime_detail.cpp");
    require(has(reader, "GCS_CURSORPOS") && has(reader, "GCS_COMPATTR") &&
                has(reader, "GCS_COMPCLAUSE"),
            "caret, attributes and clauses are read in the IME detail module");
    const std::string ime = read("src/platform/win32/win32_window_ime.cpp");
    require(has(ime, "read_win32_ime_composition_detail(") &&
                has(ime, "win32_ime_composition_update("),
            "WM_IME_COMPOSITION updates go through the conversion layer");
    const std::string paint = read("src/ui/element_text_paint.cpp");
    const std::string composition_paint =
        read("src/ui/text_composition_paint.cpp");
    require(has(paint, "paint_text_composition(") &&
                !has(paint, "ImeClauseStyle"),
            "TextElement delegates composition drawing to its module");
    require(has(composition_paint, "text_range_line_rects(") &&
                has(composition_paint, "paint_text_caret_geometry("),
            "composition underlines and caret use the shaped text geometry");
    const std::string placement = read("src/ui/runtime_platform_services.cpp");
    require(has(placement, "composition_rect.value_or(") &&
                has(placement, ".candidate_rect = candidate->rect"),
            "candidates must follow the composition caret");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "IME composition structure failed: " << error.what() << '\n';
    return 1;
  }
}
