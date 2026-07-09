#include "cgpui/prelude.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

constexpr std::string_view kWrappedParagraph =
    "CGPUI wraps public text through measurement, line metadata, and paint "
    "records.\nThis keeps examples on the public prelude surface.";

struct TextWrapperExampleSnapshot {
  cgpui::TextMeasurement measurement;
  cgpui::TextWrapLayout wrap_layout;
  std::vector<cgpui::TextGlyphPaint> glyph_paint;
  cgpui::TextMeasurementResult cold_measurement;
  cgpui::TextMeasurementResult warm_measurement;
  std::size_t cache_entries = 0;
  cgpui::LayoutOutput text_layout;
  cgpui::AccessibilityRole text_role = cgpui::AccessibilityRole::generic;
  std::string text_accessibility;
  cgpui::LayoutOutput label_layout;
  cgpui::AccessibilityRole label_role = cgpui::AccessibilityRole::generic;
  std::string label_accessibility;
};

class PublicTextWrapperExamplesView final : public cgpui::View {
 public:
  PublicTextWrapperExamplesView() {
    snapshot_ = prepare_snapshot();
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicTextWrapperExamplesView>& context) override {
    return cgpui::into_element(
        cgpui::v_flex()
            .size(context.viewport_size)
            .padding(cgpui::edges(24.0F))
            .gap(10.0F)
            .background(cgpui::rgb(21, 27, 33))
            .child(cgpui::label("Public text wrapper examples")
                       .font_size(20.0F)
                       .foreground(cgpui::rgb(242, 246, 249))
                       .build())
            .child(cgpui::label(std::string(kWrappedParagraph))
                       .style(wrapper_label_style())
                       .key("phase-d-public-text-wrapper-label")
                       .build())
            .child(cgpui::label(wrap_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(181, 197, 210))
                       .build())
            .child(cgpui::label(accessibility_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(201, 213, 222))
                       .build())
            .child(cgpui::label(cache_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(157, 180, 198))
                       .build()));
  }

 private:
  static cgpui::Style wrapper_label_style() {
    return cgpui::Style{}
        .with_font(cgpui::FontDescriptor{.family = "Inter"})
        .with_font_size(14.0F)
        .with_foreground_color(cgpui::rgb(237, 242, 246))
        .with_background_color(cgpui::rgb(42, 51, 60))
        .with_preferred_size(cgpui::Size{360.0F, 92.0F})
        .with_padding(cgpui::edges(12.0F, 10.0F))
        .with_border_width(cgpui::edges(1.0F))
        .with_border_color(cgpui::rgb(83, 102, 120))
        .with_border_radius(cgpui::BorderRadii::all(6.0F));
  }

  static cgpui::LayoutInput constrained_text_input() {
    return cgpui::LayoutInput{
        .constraints =
            cgpui::LayoutConstraints{
                .min_size = {},
                .max_size = {.width = 240.0F, .height = 140.0F},
            },
        .scale = cgpui::DpiScale{.value = 1.0F},
    };
  }

  static TextWrapperExampleSnapshot prepare_snapshot() {
    const cgpui::Style style = wrapper_label_style();
    TextWrapperExampleSnapshot snapshot;
    snapshot.measurement =
        cgpui::measure_text(kWrappedParagraph,
                            style.font,
                            style.font_size,
                            cgpui::DpiScale{.value = 1.0F});
    snapshot.wrap_layout =
        cgpui::wrap_text_measurement(snapshot.measurement, 240.0F);
    snapshot.glyph_paint = cgpui::text_glyph_paint_metadata(
        snapshot.measurement.shape_run,
        snapshot.wrap_layout.lines,
        cgpui::Point{.x = 12.0F, .y = 24.0F});

    cgpui::TextMeasurementCache cache;
    snapshot.cold_measurement =
        cache.measure(kWrappedParagraph,
                      style.font,
                      style.font_size,
                      cgpui::DpiScale{.value = 1.0F});
    snapshot.warm_measurement =
        cache.measure(kWrappedParagraph,
                      style.font,
                      style.font_size,
                      cgpui::DpiScale{.value = 1.0F});
    snapshot.cache_entries = cache.entry_count();

    cgpui::TextModel text_model{std::string(kWrappedParagraph)};
    cgpui::TextElement text_element{&text_model, style};
    snapshot.text_layout = text_element.layout(constrained_text_input());
    snapshot.text_role = text_element.accessibility_role();
    snapshot.text_accessibility = text_element.accessibility_text();

    cgpui::AnyElement label_element =
        cgpui::label("Wrapped public label")
            .style(style)
            .key("phase-d-public-label-wrapper")
            .build();
    snapshot.label_layout = label_element->layout(constrained_text_input());
    snapshot.label_role = label_element->accessibility_role();
    snapshot.label_accessibility = label_element->accessibility_text();
    return snapshot;
  }

  static std::string break_kind_name(cgpui::TextWrapBreakKind kind) {
    switch (kind) {
      case cgpui::TextWrapBreakKind::soft:
        return "soft";
      case cgpui::TextWrapBreakKind::hard:
        return "hard";
      case cgpui::TextWrapBreakKind::none:
        return "none";
    }
    return "none";
  }

  [[nodiscard]] std::string wrap_line() const {
    const cgpui::TextWrapLine& first_line = snapshot_.wrap_layout.lines.front();
    return "lines=" + std::to_string(snapshot_.wrap_layout.lines.size()) +
           " glyphs=" + std::to_string(snapshot_.glyph_paint.size()) +
           " first_break=" + break_kind_name(first_line.break_kind) +
           " width=" +
           std::to_string(static_cast<int>(snapshot_.wrap_layout.logical_size.width));
  }

  [[nodiscard]] std::string accessibility_line() const {
    return "text_role=" + role_name(snapshot_.text_role) + " text_bytes=" +
           std::to_string(snapshot_.text_accessibility.size()) +
           " label_role=" + role_name(snapshot_.label_role) +
           " label_bytes=" +
           std::to_string(snapshot_.label_accessibility.size());
  }

  [[nodiscard]] std::string cache_line() const {
    return "cache_entries=" + std::to_string(snapshot_.cache_entries) +
           " cold_hit=" +
           (snapshot_.cold_measurement.cache_hit ? "yes" : "no") +
           " warm_hit=" +
           (snapshot_.warm_measurement.cache_hit ? "yes" : "no") +
           " text_layout=" +
           std::to_string(static_cast<int>(snapshot_.text_layout.size.width)) +
           "x" +
           std::to_string(static_cast<int>(snapshot_.text_layout.size.height)) +
           " label_layout=" +
           std::to_string(static_cast<int>(snapshot_.label_layout.size.width)) +
           "x" +
           std::to_string(static_cast<int>(snapshot_.label_layout.size.height));
  }

  static std::string role_name(cgpui::AccessibilityRole role) {
    switch (role) {
      case cgpui::AccessibilityRole::label:
        return "label";
      case cgpui::AccessibilityRole::text:
        return "text";
      default:
        return "other";
    }
  }

  TextWrapperExampleSnapshot snapshot_;
};

using TextElementRef = cgpui::TextElement&;
using TextCacheRef = cgpui::TextMeasurementCache&;
using LabelBuilderValue = decltype(cgpui::label(std::string_view{}));

static_assert(cgpui::Render<PublicTextWrapperExamplesView>);
static_assert(
    std::is_same_v<decltype(cgpui::measure_text(
                       std::declval<std::string_view>(),
                       std::declval<cgpui::FontDescriptor>(),
                       14.0F,
                       std::declval<cgpui::DpiScale>())),
                   cgpui::TextMeasurement>);
static_assert(
    std::is_same_v<decltype(cgpui::wrap_text_measurement(
                       std::declval<const cgpui::TextMeasurement&>(),
                       240.0F)),
                   cgpui::TextWrapLayout>);
static_assert(
    std::is_same_v<decltype(cgpui::text_glyph_paint_metadata(
                       std::declval<const cgpui::TextShapeRun&>(),
                       std::declval<std::span<const cgpui::TextWrapLine>>(),
                       std::declval<cgpui::Point>())),
                   std::vector<cgpui::TextGlyphPaint>>);
static_assert(
    std::is_same_v<decltype(std::declval<TextCacheRef>().measure(
                       std::declval<std::string_view>(),
                       std::declval<cgpui::FontDescriptor>(),
                       14.0F,
                       std::declval<cgpui::DpiScale>())),
                   cgpui::TextMeasurementResult>);
static_assert(std::is_same_v<decltype(std::declval<TextElementRef>().layout(
                                      std::declval<cgpui::LayoutInput>())),
                             cgpui::LayoutOutput>);
static_assert(
    std::is_same_v<decltype(std::declval<TextElementRef>().accessibility_text()),
                   std::string>);
static_assert(std::is_same_v<decltype(std::declval<LabelBuilderValue>()
                                          .build()),
                             cgpui::AnyElement>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Text Wrapper Examples")
                       .size(620.0F, 360.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_TEXT_WRAPPER_EXAMPLES") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicTextWrapperExamplesView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}
