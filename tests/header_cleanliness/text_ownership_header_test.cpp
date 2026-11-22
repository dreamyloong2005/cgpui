#include "cgpui/renderer/text/text_glyph_rasterizer.hpp"
#include "cgpui/renderer/text/text_font_defaults.hpp"
#include "cgpui/renderer/text/text_layout.hpp"
#include "cgpui/renderer/text/text_shape_cache.hpp"
#include "cgpui/renderer/text/text_rich_text.hpp"
#include "cgpui/renderer/text/text_rich_text_inline_image.hpp"
#include "cgpui/core/font.hpp"
#include "cgpui/core/font_face.hpp"
#include "cgpui/core/paint_values.hpp"
#include "cgpui/core/pixel_scale.hpp"
#include "cgpui/platform/font_database.hpp"

// Existing source includes must remain valid alongside the new owners.
#include "cgpui/ui/style_font.hpp"
#include "cgpui/ui/text.hpp"
#include "cgpui/ui/text_glyph_rasterizer.hpp"

#include <type_traits>

static_assert(std::is_default_constructible_v<cgpui::FontDescriptor>);
static_assert(std::is_default_constructible_v<cgpui::FontFaceDescriptor>);
static_assert(std::is_default_constructible_v<cgpui::FontDatabase>);
static_assert(std::is_default_constructible_v<cgpui::TextShapeRun>);
static_assert(std::is_default_constructible_v<cgpui::TextMeasurement>);
static_assert(std::is_default_constructible_v<cgpui::TextParagraphLayout>);
static_assert(std::is_default_constructible_v<cgpui::RichTextRun>);
static_assert(cgpui::to_device_pixels(cgpui::Size{2, 3}, {2}).width == 4);
static_assert(cgpui::font_feature("liga", 0).value == 0);

int main() { return 0; }
