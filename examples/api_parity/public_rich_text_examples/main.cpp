#include "cgpui/prelude.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

constexpr std::string_view kRichTextSource = "fn link() -> value";

struct RichTextExampleSnapshot {
  std::vector<cgpui::RichTextSpan> spans;
  std::vector<cgpui::RichTextRun> runs;
  std::vector<cgpui::RichTextInlineImageRun> inline_images;
  cgpui::TextMeasurement measurement;
  std::optional<cgpui::RichTextRunHit> byte_run_hit;
  std::optional<cgpui::RichTextLinkHit> byte_link_hit;
  std::optional<cgpui::RichTextRunPointHit> point_run_hit;
  std::optional<cgpui::RichTextLinkActivation> activation;
  bool pointer_can_activate = false;
  std::size_t paint_command_count = 0;
  std::size_t painted_run_count = 0;
  std::size_t painted_inline_image_count = 0;
};

class PublicRichTextExamplesView final : public cgpui::View {
 public:
  PublicRichTextExamplesView() {
    snapshot_ = prepare_rich_text_snapshot();
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicRichTextExamplesView>& context) override {
    return cgpui::into_element(
        cgpui::v_flex()
            .size(context.viewport_size)
            .padding(cgpui::edges(24.0F))
            .gap(10.0F)
            .background(cgpui::rgb(24, 29, 35))
            .child(cgpui::label("Public rich text examples")
                       .font_size(20.0F)
                       .foreground(cgpui::rgb(242, 247, 250))
                       .build())
            .child(cgpui::label(std::string(kRichTextSource))
                       .font_size(16.0F)
                       .foreground(cgpui::rgb(235, 240, 245))
                       .build())
            .child(cgpui::label(run_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(190, 205, 217))
                       .build())
            .child(cgpui::label(hit_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(177, 196, 210))
                       .build())
            .child(cgpui::label(paint_line())
                       .font_size(12.0F)
                       .foreground(cgpui::rgb(207, 218, 226))
                       .build()));
  }

 private:
  static cgpui::RichTextSyntaxTheme syntax_theme() {
    cgpui::RichTextSyntaxTheme theme;
    theme.text.foreground = cgpui::rgb(226, 232, 238);
    theme.keyword.foreground = cgpui::rgb(116, 190, 255);
    theme.function_name.foreground = cgpui::rgb(135, 221, 170);
    theme.operator_token.foreground = cgpui::rgb(240, 198, 116);
    theme.variable.foreground = cgpui::rgb(229, 216, 146);
    return theme;
  }

  static std::size_t find_token(std::string_view token) {
    return kRichTextSource.find(token);
  }

  static cgpui::RichTextAttributes link_attributes() {
    cgpui::RichTextAttributes attributes;
    attributes.foreground = cgpui::rgb(99, 179, 237);
    attributes.decorations = cgpui::RichTextDecoration::underline;
    attributes.link_id = cgpui::RichTextLinkId{.value = 42};
    return attributes;
  }

  static RichTextExampleSnapshot prepare_rich_text_snapshot() {
    RichTextExampleSnapshot snapshot;
    const cgpui::RichTextSyntaxTheme theme = syntax_theme();
    const std::array<cgpui::RichTextSyntaxToken, 4> tokens{
        cgpui::RichTextSyntaxToken{
            .role = cgpui::RichTextSyntaxRole::keyword,
            .byte_start = find_token("fn"),
            .byte_end = find_token("fn") + 2,
        },
        cgpui::RichTextSyntaxToken{
            .role = cgpui::RichTextSyntaxRole::function_name,
            .byte_start = find_token("link"),
            .byte_end = find_token("link") + 4,
        },
        cgpui::RichTextSyntaxToken{
            .role = cgpui::RichTextSyntaxRole::operator_token,
            .byte_start = find_token("->"),
            .byte_end = find_token("->") + 2,
        },
        cgpui::RichTextSyntaxToken{
            .role = cgpui::RichTextSyntaxRole::variable,
            .byte_start = find_token("value"),
            .byte_end = find_token("value") + 5,
        },
    };
    snapshot.spans =
        cgpui::build_rich_text_syntax_spans(kRichTextSource, tokens, theme);
    snapshot.spans.push_back(cgpui::RichTextSpan{
        .byte_start = find_token("link"),
        .byte_end = find_token("link") + 4,
        .attributes = link_attributes(),
    });

    const cgpui::RichTextAttributes function_attributes =
        cgpui::rich_text_syntax_attributes_for_role(
            theme,
            cgpui::RichTextSyntaxRole::function_name);
    snapshot.spans.push_back(cgpui::RichTextSpan{
        .byte_start = find_token("link"),
        .byte_end = find_token("link") + 4,
        .attributes = function_attributes,
    });

    snapshot.runs =
        cgpui::build_rich_text_runs(kRichTextSource, snapshot.spans);

    const std::array<cgpui::RichTextInlineImageSpan, 1> images{
        cgpui::RichTextInlineImageSpan{
            .image_id = cgpui::ImageAssetId{.value = 7},
            .byte_start = find_token("value"),
            .byte_end = find_token("value"),
            .logical_size = {.width = 14.0F, .height = 14.0F},
            .baseline_offset = -2.0F,
        },
    };
    snapshot.inline_images =
        cgpui::build_rich_text_inline_image_runs(kRichTextSource, images);

    const cgpui::FontDescriptor font{.family = "Inter"};
    snapshot.measurement =
        cgpui::measure_text(kRichTextSource,
                            font,
                            16.0F,
                            cgpui::DpiScale{.value = 1.0F});
    snapshot.byte_run_hit =
        cgpui::rich_text_run_at_byte_offset(snapshot.runs, find_token("link"));
    snapshot.byte_link_hit =
        cgpui::rich_text_link_at_byte_offset(snapshot.runs, find_token("link"));

    const cgpui::Rect bounds{
        .origin = {.x = 0.0F, .y = 0.0F},
        .size = {.width = 360.0F, .height = 44.0F},
    };
    const cgpui::PointerButton release{
        .button = cgpui::MouseButton::left,
        .pressed = false,
        .click_count = 1,
        .position = {.x = 30.0F, .y = 8.0F},
    };
    snapshot.pointer_can_activate =
        cgpui::rich_text_pointer_button_can_activate_link(release);
    snapshot.point_run_hit = cgpui::rich_text_run_at_point(
        snapshot.runs,
        snapshot.measurement,
        bounds,
        release.position);
    snapshot.activation = cgpui::rich_text_link_activation_at_point(
        snapshot.runs,
        snapshot.measurement,
        bounds,
        release);

    cgpui::PaintList paint_list;
    paint_list.fill_rich_text(bounds,
                              cgpui::rgb(232, 238, 244),
                              kRichTextSource,
                              snapshot.runs,
                              snapshot.inline_images,
                              font,
                              16.0F);
    snapshot.paint_command_count = paint_list.commands().size();
    if (!paint_list.commands().empty()) {
      const cgpui::PaintCommand& command = paint_list.commands().front();
      snapshot.painted_run_count = command.text.rich_text_runs.size();
      snapshot.painted_inline_image_count =
          command.text.rich_text_inline_images.size();
    }
    return snapshot;
  }

  [[nodiscard]] std::string run_line() const {
    return "spans=" + std::to_string(snapshot_.spans.size()) +
           " runs=" + std::to_string(snapshot_.runs.size()) +
           " images=" + std::to_string(snapshot_.inline_images.size()) +
           " glyphs=" +
           std::to_string(snapshot_.measurement.shape_run.glyphs.size());
  }

  [[nodiscard]] std::string hit_line() const {
    return "byte_run=" + yes_no(snapshot_.byte_run_hit.has_value()) +
           " byte_link=" + yes_no(snapshot_.byte_link_hit.has_value()) +
           " point_run=" + yes_no(snapshot_.point_run_hit.has_value()) +
           " can_activate=" + yes_no(snapshot_.pointer_can_activate) +
           " activation=" + yes_no(snapshot_.activation.has_value());
  }

  [[nodiscard]] std::string paint_line() const {
    return "paint_commands=" +
           std::to_string(snapshot_.paint_command_count) +
           " painted_runs=" +
           std::to_string(snapshot_.painted_run_count) +
           " painted_images=" +
           std::to_string(snapshot_.painted_inline_image_count);
  }

  static std::string yes_no(bool value) {
    return value ? "yes" : "no";
  }

  RichTextExampleSnapshot snapshot_;
};

using RichTextRunSpan = std::span<const cgpui::RichTextRun>;
using RichTextSpanSpan = std::span<const cgpui::RichTextSpan>;
using RichTextImageSpanSpan = std::span<const cgpui::RichTextInlineImageSpan>;

static_assert(cgpui::Render<PublicRichTextExamplesView>);
static_assert(
    std::is_same_v<decltype(cgpui::build_rich_text_runs(
                       std::declval<std::string_view>(),
                       std::declval<RichTextSpanSpan>())),
                   std::vector<cgpui::RichTextRun>>);
static_assert(
    std::is_same_v<decltype(cgpui::rich_text_run_at_byte_offset(
                       std::declval<RichTextRunSpan>(),
                       0)),
                   std::optional<cgpui::RichTextRunHit>>);
static_assert(
    std::is_same_v<decltype(cgpui::rich_text_link_at_byte_offset(
                       std::declval<RichTextRunSpan>(),
                       0)),
                   std::optional<cgpui::RichTextLinkHit>>);
static_assert(
    std::is_same_v<decltype(cgpui::rich_text_link_activation_at_point(
                       std::declval<RichTextRunSpan>(),
                       std::declval<const cgpui::TextMeasurement&>(),
                       std::declval<cgpui::Rect>(),
                       std::declval<const cgpui::PointerButton&>())),
                   std::optional<cgpui::RichTextLinkActivation>>);
static_assert(
    std::is_same_v<decltype(cgpui::build_rich_text_inline_image_runs(
                       std::declval<std::string_view>(),
                       std::declval<RichTextImageSpanSpan>())),
                   std::vector<cgpui::RichTextInlineImageRun>>);

} // namespace

int main() {
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Rich Text Examples")
                       .size(620.0F, 320.0F)
                       .decorations(true)
                       .resizable(true)
                       .transparent(false)
                       .titlebar_visible(true)
                       .to_descriptor();

  if (std::getenv("CGPUI_RUN_PUBLIC_RICH_TEXT_EXAMPLES") == nullptr) {
    return 0;
  }

  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  PublicRichTextExamplesView view;
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      std::move(options));
}
