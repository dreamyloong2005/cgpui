#pragma once

#include "cgpui/ui/text_rich_text.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace cgpui {

enum class RichTextSyntaxRole : std::uint8_t {
  text,
  keyword,
  string_literal,
  number,
  comment,
  type_name,
  function_name,
  variable,
  punctuation,
  operator_token,
};

struct RichTextSyntaxToken {
  RichTextSyntaxRole role = RichTextSyntaxRole::text;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
};

struct RichTextSyntaxTheme {
  RichTextAttributes text;
  RichTextAttributes keyword;
  RichTextAttributes string_literal;
  RichTextAttributes number;
  RichTextAttributes comment;
  RichTextAttributes type_name;
  RichTextAttributes function_name;
  RichTextAttributes variable;
  RichTextAttributes punctuation;
  RichTextAttributes operator_token;
};

[[nodiscard]] RichTextAttributes rich_text_syntax_attributes_for_role(
    const RichTextSyntaxTheme& theme,
    RichTextSyntaxRole role);

void build_rich_text_syntax_spans(
    std::string_view text,
    std::span<const RichTextSyntaxToken> tokens,
    const RichTextSyntaxTheme& theme,
    std::vector<RichTextSpan>& spans);

[[nodiscard]] std::vector<RichTextSpan> build_rich_text_syntax_spans(
    std::string_view text,
    std::span<const RichTextSyntaxToken> tokens,
    const RichTextSyntaxTheme& theme);

} // namespace cgpui
