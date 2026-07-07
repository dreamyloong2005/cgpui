#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(path);
  if (!source) {
    source.open((std::string("../../../../") + path).c_str());
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t count = 0;
  for (const char value : text) {
    if (value == '\n') {
      count += 1;
    }
  }
  return count;
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  if (!contains(xmake, "add_files(\"src/ui/widgets/*.cpp\")") ||
      !contains(xmake, "target(\"builtin_widget_test\")") ||
      !contains(xmake, "target(\"widget_source_structure_test\")")) {
    return 1;
  }

  const std::string widget_aggregate =
      read_source("include/cgpui/ui/widget_builders.hpp");
  if (!contains(widget_aggregate,
                "#include \"cgpui/ui/text_input_builder.hpp\"") ||
      !contains(widget_aggregate,
                "#include \"cgpui/ui/label_builder.hpp\"") ||
      !contains(widget_aggregate,
                "#include \"cgpui/ui/button_builder.hpp\"") ||
      !contains(widget_aggregate,
                "#include \"cgpui/ui/toggle_builder.hpp\"") ||
      !contains(widget_aggregate,
                "#include \"cgpui/ui/slider_builder.hpp\"")) {
    return 2;
  }
  if (!contains(widget_aggregate,
                "#include \"cgpui/ui/item_builder.hpp\"") ||
      !contains(widget_aggregate,
                "#include \"cgpui/ui/image_builder.hpp\"") ||
      !contains(widget_aggregate,
                "#include \"cgpui/ui/container_builder.hpp\"")) {
    return 2;
  }
  if (line_count(widget_aggregate) > 20 ||
      contains(widget_aggregate, "class ButtonBuilder") ||
      contains(widget_aggregate, "class TextInputBuilder") ||
      contains(widget_aggregate, "class ToggleBuilder") ||
      contains(widget_aggregate, "class SliderBuilder") ||
      contains(widget_aggregate, "class ItemBuilder") ||
      contains(widget_aggregate, "class ImageBuilder") ||
      contains(widget_aggregate, "h_stack()")) {
    return 3;
  }

  const std::string button_header =
      read_source("include/cgpui/ui/button_builder.hpp");
  const std::string label_header =
      read_source("include/cgpui/ui/label_builder.hpp");
  const std::string text_input_header =
      read_source("include/cgpui/ui/text_input_builder.hpp");
  const std::string toggle_header =
      read_source("include/cgpui/ui/toggle_builder.hpp");
  const std::string slider_header =
      read_source("include/cgpui/ui/slider_builder.hpp");
  const std::string item_header =
      read_source("include/cgpui/ui/item_builder.hpp");
  const std::string image_header =
      read_source("include/cgpui/ui/image_builder.hpp");
  const std::string container_header =
      read_source("include/cgpui/ui/container_builder.hpp");
  const std::string element_nodes_header =
      read_source("include/cgpui/ui/element_nodes.hpp");
  const std::string element_choice_header =
      read_source("include/cgpui/ui/element_choice_nodes.hpp");
  const std::string element_slider_header =
      read_source("include/cgpui/ui/element_slider_nodes.hpp");
  const std::string element_item_header =
      read_source("include/cgpui/ui/element_item_nodes.hpp");
  const std::string element_image_header =
      read_source("include/cgpui/ui/element_image_nodes.hpp");
  if (!contains(button_header, "ButtonBuilder label(std::string_view text)") ||
      !contains(button_header, "class ButtonBuilder") ||
      !contains(label_header, "class LabelBuilder") ||
      !contains(text_input_header, "class TextInputBuilder") ||
      !contains(toggle_header, "class ToggleBuilder") ||
      !contains(toggle_header, "checkbox(std::string_view action_name)") ||
      !contains(toggle_header, "radio(std::string_view action_name)") ||
      !contains(toggle_header, "toggle_switch(std::string_view action_name)") ||
      !contains(slider_header, "class SliderBuilder") ||
      !contains(slider_header, "slider(std::string_view action_name)") ||
      !contains(item_header, "class ItemBuilder") ||
      !contains(item_header, "list_item(std::string_view action_name)") ||
      !contains(item_header, "menu_item(std::string_view action_name)") ||
      !contains(image_header, "class ImageBuilder") ||
      !contains(image_header, "image(ImageAssetDescriptor asset)") ||
      !contains(image_header, "icon(ImageAssetDescriptor asset)") ||
      !contains(container_header, "h_stack()") ||
      !contains(container_header, "div()") ||
      !contains(container_header, "h_flex()") ||
      !contains(container_header, "v_flex()") ||
      !contains(container_header, "v_stack()") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_choice_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_slider_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_item_nodes.hpp\"") ||
      !contains(element_nodes_header,
                "#include \"cgpui/ui/element_image_nodes.hpp\"") ||
      !contains(element_choice_header, "class ToggleControlElement") ||
      !contains(element_slider_header, "class SliderElement") ||
      !contains(element_item_header, "class ItemElement") ||
      !contains(element_image_header, "class ImageElement")) {
    return 4;
  }

  const std::string widget_button =
      read_source("src/ui/widgets/button_builder.cpp");
  const std::string widget_label =
      read_source("src/ui/widgets/label_builder.cpp");
  const std::string widget_text_input =
      read_source("src/ui/widgets/text_input_builder.cpp");
  const std::string widget_toggle =
      read_source("src/ui/widgets/toggle_builder.cpp");
  const std::string widget_slider =
      read_source("src/ui/widgets/slider_builder.cpp");
  const std::string widget_item =
      read_source("src/ui/widgets/item_builder.cpp");
  const std::string widget_image =
      read_source("src/ui/widgets/image_builder.cpp");
  const std::string widget_container =
      read_source("src/ui/widgets/container_builder.cpp");
  const std::string element_choice =
      read_source("src/ui/element_choice_nodes.cpp");
  const std::string element_choice_layout =
      read_source("src/ui/element_choice_layout.cpp");
  const std::string element_choice_paint =
      read_source("src/ui/element_choice_paint.cpp");
  const std::string element_slider =
      read_source("src/ui/element_slider_nodes.cpp");
  const std::string element_slider_layout =
      read_source("src/ui/element_slider_layout.cpp");
  const std::string element_slider_paint =
      read_source("src/ui/element_slider_paint.cpp");
  const std::string element_item =
      read_source("src/ui/element_item_nodes.cpp");
  const std::string element_item_layout =
      read_source("src/ui/element_item_layout.cpp");
  const std::string element_item_paint =
      read_source("src/ui/element_item_paint.cpp");
  const std::string element_image =
      read_source("src/ui/element_image_nodes.cpp");
  const std::string element_image_layout =
      read_source("src/ui/element_image_layout.cpp");
  const std::string element_image_paint =
      read_source("src/ui/element_image_paint.cpp");
  if (widget_button.empty() || widget_label.empty() ||
      widget_text_input.empty() || widget_toggle.empty() ||
      element_choice.empty() || element_choice_layout.empty() ||
      element_choice_paint.empty() || widget_slider.empty() ||
      element_slider.empty() || element_slider_layout.empty() ||
      element_slider_paint.empty() || widget_item.empty() ||
      element_item.empty() || element_item_layout.empty() ||
      element_item_paint.empty() || widget_image.empty() ||
      widget_container.empty() ||
      element_image.empty() || element_image_layout.empty() ||
      element_image_paint.empty()) {
    return 5;
  }
  if (!contains(widget_button, "ButtonBuilder::label(") ||
      !contains(widget_button, "cgpui::label(text).build()") ||
      !contains(widget_label, "LabelBuilder::build()") ||
      !contains(widget_text_input, "TextInputBuilder::build()") ||
      !contains(widget_toggle, "ToggleBuilder::label(") ||
      !contains(widget_toggle, "ToggleControlKind::checkbox") ||
      !contains(widget_toggle, "ToggleControlKind::radio") ||
      !contains(widget_toggle, "ToggleControlKind::toggle_switch") ||
      !contains(widget_slider, "SliderBuilder::range(") ||
      !contains(widget_slider, "SliderElement") ||
      !contains(widget_item, "ItemBuilder::label(") ||
      !contains(widget_item, "ItemKind::list_item") ||
      !contains(widget_item, "ItemKind::menu_item") ||
      !contains(widget_image, "ImageBuilder::alt(") ||
      !contains(widget_image, "ImageElementKind::image") ||
      !contains(widget_image, "ImageElementKind::icon") ||
      !contains(widget_container, "ElementBuilder h_stack()") ||
      !contains(widget_container, "ElementBuilder::row()") ||
      !contains(widget_container, "ElementBuilder div()") ||
      !contains(element_choice, "ToggleControlElement::accessibility_value()") ||
      !contains(element_choice_layout, "ToggleControlElement::layout(") ||
      !contains(element_choice_paint, "ToggleControlElement::paint(") ||
      !contains(element_slider, "SliderElement::accessibility_value()") ||
      !contains(element_slider_layout, "SliderElement::layout(") ||
      !contains(element_slider_paint, "SliderElement::paint(") ||
      !contains(element_item, "ItemElement::accessibility_value()") ||
      !contains(element_item_layout, "ItemElement::layout(") ||
      !contains(element_item_paint, "ItemElement::paint(") ||
      !contains(element_image, "ImageElement::accessibility_name()") ||
      !contains(element_image_layout, "ImageElement::layout(") ||
      !contains(element_image_paint, "ImageElement::paint(")) {
    return 6;
  }

  const std::string root_button = read_source("src/ui/button_builder.cpp");
  const std::string root_label = read_source("src/ui/label_builder.cpp");
  const std::string root_text_input =
      read_source("src/ui/text_input_builder.cpp");
  const std::string root_toggle = read_source("src/ui/toggle_builder.cpp");
  const std::string root_slider = read_source("src/ui/slider_builder.cpp");
  const std::string root_item = read_source("src/ui/item_builder.cpp");
  const std::string root_image = read_source("src/ui/image_builder.cpp");
  const std::string element_builder_factories =
      read_source("src/ui/element_builder_factories.cpp");
  if (contains(root_button, "ButtonBuilder::build()") ||
      contains(root_button, "ButtonBuilder::label(") ||
      contains(root_label, "LabelBuilder::build()") ||
      contains(root_text_input, "TextInputBuilder::build()") ||
      contains(root_toggle, "ToggleBuilder::build()") ||
      contains(root_slider, "SliderBuilder::build()") ||
      contains(root_item, "ItemBuilder::build()") ||
      contains(root_image, "ImageBuilder::build()") ||
      contains(element_builder_factories, "ElementBuilder div()") ||
      contains(element_builder_factories, "ElementBuilder h_flex()") ||
      contains(element_builder_factories, "ElementBuilder v_flex()") ||
      contains(element_builder_factories, "ElementBuilder v_stack()")) {
    return 7;
  }

  if (line_count(widget_button) > 140 || line_count(widget_label) > 100 ||
      line_count(widget_text_input) > 100 || line_count(widget_toggle) > 180 ||
      line_count(widget_slider) > 180 ||
      line_count(element_choice) > 180 ||
      line_count(element_choice_layout) > 80 ||
      line_count(element_choice_paint) > 80 ||
      line_count(element_slider) > 180 ||
      line_count(element_slider_layout) > 80 ||
      line_count(element_slider_paint) > 100 ||
      line_count(widget_item) > 180 ||
      line_count(element_item) > 180 ||
      line_count(element_item_layout) > 80 ||
      line_count(element_item_paint) > 100 ||
      line_count(widget_image) > 180 ||
      line_count(widget_container) > 120 ||
      line_count(element_image) > 160 ||
      line_count(element_image_layout) > 80 ||
      line_count(element_image_paint) > 80) {
    return 8;
  }

  return 0;
}
