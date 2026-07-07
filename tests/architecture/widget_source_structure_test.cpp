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
                "#include \"cgpui/ui/button_builder.hpp\"")) {
    return 2;
  }
  if (line_count(widget_aggregate) > 20 ||
      contains(widget_aggregate, "class ButtonBuilder") ||
      contains(widget_aggregate, "class TextInputBuilder")) {
    return 3;
  }

  const std::string button_header =
      read_source("include/cgpui/ui/button_builder.hpp");
  const std::string label_header =
      read_source("include/cgpui/ui/label_builder.hpp");
  const std::string text_input_header =
      read_source("include/cgpui/ui/text_input_builder.hpp");
  if (!contains(button_header, "ButtonBuilder label(std::string_view text)") ||
      !contains(button_header, "class ButtonBuilder") ||
      !contains(label_header, "class LabelBuilder") ||
      !contains(text_input_header, "class TextInputBuilder")) {
    return 4;
  }

  const std::string widget_button =
      read_source("src/ui/widgets/button_builder.cpp");
  const std::string widget_label =
      read_source("src/ui/widgets/label_builder.cpp");
  const std::string widget_text_input =
      read_source("src/ui/widgets/text_input_builder.cpp");
  if (widget_button.empty() || widget_label.empty() ||
      widget_text_input.empty()) {
    return 5;
  }
  if (!contains(widget_button, "ButtonBuilder::label(") ||
      !contains(widget_button, "cgpui::label(text).build()") ||
      !contains(widget_label, "LabelBuilder::build()") ||
      !contains(widget_text_input, "TextInputBuilder::build()")) {
    return 6;
  }

  const std::string root_button = read_source("src/ui/button_builder.cpp");
  const std::string root_label = read_source("src/ui/label_builder.cpp");
  const std::string root_text_input =
      read_source("src/ui/text_input_builder.cpp");
  if (contains(root_button, "ButtonBuilder::build()") ||
      contains(root_button, "ButtonBuilder::label(") ||
      contains(root_label, "LabelBuilder::build()") ||
      contains(root_text_input, "TextInputBuilder::build()")) {
    return 7;
  }

  if (line_count(widget_button) > 140 || line_count(widget_label) > 100 ||
      line_count(widget_text_input) > 100) {
    return 8;
  }

  return 0;
}
