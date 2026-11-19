// Leaf elements paint the box style set on their builder (shadow,
// background, border, radius) beneath their content: text(), text_input(),
// label(), image(), icon(), svg(), button(), list_item(), checkbox(),
// slider(). text() and label() also take their size and padding as a CSS
// content box, and draw the text inside it.
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text_model.hpp"
#include "cgpui/ui/ui.hpp"

#include <cstddef>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>

namespace {

using cgpui::Color;
using cgpui::PaintCommand;
using cgpui::PaintCommandKind;
using cgpui::Rect;

constexpr Color box_color = cgpui::rgb(200, 10, 10);
constexpr Color border_color = cgpui::rgb(10, 10, 200);

bool same_color(Color lhs, Color rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

bool same_rect(Rect lhs, Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width &&
         lhs.size.height == rhs.size.height;
}

// Index of the first command painting `box_color`, as a solid or rounded
// rect, and its rect.
struct BoxCommand {
  std::size_t index = 0;
  Rect rect;
};

std::optional<BoxCommand> box_command(const cgpui::PaintList& list) {
  const auto& commands = list.commands();
  for (std::size_t index = 0; index < commands.size(); ++index) {
    const PaintCommand& command = commands[index];
    if (command.kind == PaintCommandKind::solid_rect &&
        same_color(command.solid_rect.color, box_color)) {
      return BoxCommand{.index = index, .rect = command.solid_rect.rect};
    }
    if (command.kind == PaintCommandKind::rounded_rect &&
        command.rounded_rect.fill_enabled &&
        same_color(command.rounded_rect.color, box_color)) {
      return BoxCommand{.index = index, .rect = command.rounded_rect.rect};
    }
  }
  return std::nullopt;
}

std::optional<std::size_t> first_command(
    const cgpui::PaintList& list,
    PaintCommandKind kind) {
  const auto& commands = list.commands();
  for (std::size_t index = 0; index < commands.size(); ++index) {
    if (commands[index].kind == kind) {
      return index;
    }
  }
  return std::nullopt;
}

cgpui::Style box_style() {
  return cgpui::Style{}
      .with_preferred_size({.width = 120.0F, .height = 30.0F})
      .with_background_color(box_color)
      .with_border_width(cgpui::edges(1.0F))
      .with_border_color(border_color)
      .with_border_radius(cgpui::BorderRadii::all(4.0F));
}

int fail(int code, std::string_view message) {
  std::cerr << message << '\n';
  return code;
}

struct Painted {
  cgpui::PaintList list;
  std::optional<Rect> bounds;
};

// Lays out `element` as the root at the origin and paints it.
Painted paint_root(cgpui::AnyElement element) {
  const cgpui::Element* raw = element.get();
  cgpui::ElementTree tree;
  (void)tree.set_root(std::move(element));
  (void)tree.layout_root(cgpui::LayoutInput{
      .constraints = {.max_size = {400.0F, 300.0F}}});
  Painted painted;
  tree.paint(painted.list);
  painted.bounds = raw->layout_bounds();
  return painted;
}

// The box covers the element bounds and paints before `content`, if any.
int check_box(cgpui::AnyElement element, int code,
              std::optional<PaintCommandKind> content) {
  const Painted painted = paint_root(std::move(element));
  const std::optional<BoxCommand> box = box_command(painted.list);
  if (!box.has_value() || !painted.bounds.has_value()) {
    return fail(code, "the element's background must be painted");
  }
  if (!same_rect(box->rect, *painted.bounds)) {
    return fail(code + 1, "the background must cover the element bounds");
  }
  if (content.has_value()) {
    const std::optional<std::size_t> index =
        first_command(painted.list, *content);
    if (!index.has_value() || *index < box->index) {
      return fail(code + 2, "the content must paint above the box");
    }
  }
  return 0;
}

// text() and label(): size fixes the content box, padding surrounds it,
// and the text is drawn at the content origin.
int check_text_box(cgpui::AnyElement element, int code) {
  const Painted painted = paint_root(std::move(element));
  const std::optional<std::size_t> text =
      first_command(painted.list, PaintCommandKind::text);
  if (!painted.bounds.has_value() || !text.has_value()) {
    return fail(code, "the text must be laid out and painted");
  }
  if (!same_rect(*painted.bounds, Rect{{0, 0}, {132, 42}})) {
    return fail(code + 1, "size plus padding must give the element bounds");
  }
  const Rect text_rect = painted.list.commands()[*text].text.bounds;
  if (!same_rect(text_rect, Rect{{6, 6}, {120, 30}})) {
    return fail(code + 2, "the text must be drawn in the content box");
  }
  return 0;
}

} // namespace

int main() {
  cgpui::TextModel model;
  model.insert_text("Field");
  const cgpui::Style padded =
      box_style().with_padding(cgpui::edges(6.0F));
  const cgpui::ImageAssetDescriptor asset{
      .id = cgpui::ImageAssetId{7},
      .logical_size = {.width = 20.0F, .height = 12.0F},
  };
  constexpr std::string_view markup =
      "<svg viewBox=\"0 0 18 10\"><path d=\"M0 0h18v10H0z\"/></svg>";
  std::pair<cgpui::AnyElement, std::optional<PaintCommandKind>> cases[] = {
          {cgpui::into_element(cgpui::text(model).style(box_style())),
           PaintCommandKind::text},
          {cgpui::text_input(model).style(box_style()).build(),
           PaintCommandKind::text},
          {cgpui::label("Label").style(box_style()).build(),
           PaintCommandKind::text},
          {cgpui::image(asset).style(box_style()).build(),
           PaintCommandKind::image},
          {cgpui::icon(asset).style(box_style()).build(),
           PaintCommandKind::image},
          {cgpui::svg(cgpui::ImageAssetId{8}, {18.0F, 10.0F}, markup)
               .style(box_style())
               .build(),
           PaintCommandKind::image},
          {cgpui::button("demo.button").style(box_style()).label("Go").build(),
           PaintCommandKind::text},
          {cgpui::list_item("demo.item").style(box_style()).label("I").build(),
           PaintCommandKind::text},
          {cgpui::checkbox("demo.check").style(box_style()).label("C").build(),
           std::nullopt},
          {cgpui::slider("demo.slider").style(box_style()).build(),
           std::nullopt},
      };
  int code = 10;
  for (auto& [element, content] : cases) {
    if (const int result = check_box(std::move(element), code, content);
        result != 0) {
      return result;
    }
    code += 10;
  }
  if (const int result = check_text_box(
          cgpui::into_element(cgpui::text(model).style(padded)), 200);
      result != 0) {
    return result;
  }
  return check_text_box(cgpui::label("Label").style(padded).build(), 210);
}
