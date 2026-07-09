#include "cgpui/ui/text_rich_text_syntax.hpp"

#include <algorithm>
#include <optional>

namespace cgpui {
namespace {

bool syntax_attributes_are_empty(const RichTextAttributes& attributes) {
  return attributes == RichTextAttributes{};
}

std::optional<RichTextSpan> clipped_syntax_token(
    std::string_view text,
    const RichTextSyntaxToken& token,
    const RichTextSyntaxTheme& theme) {
  if (token.byte_start >= token.byte_end || token.byte_start >= text.size()) {
    return std::nullopt;
  }

  const RichTextAttributes attributes =
      rich_text_syntax_attributes_for_role(theme, token.role);
  if (syntax_attributes_are_empty(attributes)) {
    return std::nullopt;
  }

  return RichTextSpan{
      .byte_start = token.byte_start,
      .byte_end = std::min(token.byte_end, text.size()),
      .attributes = attributes,
  };
}

void sort_syntax_spans(std::vector<RichTextSpan>& spans) {
  std::sort(
      spans.begin(),
      spans.end(),
      [](const RichTextSpan& left, const RichTextSpan& right) {
        if (left.byte_start != right.byte_start) {
          return left.byte_start < right.byte_start;
        }
        return left.byte_end < right.byte_end;
      });
}

} // namespace

RichTextAttributes rich_text_syntax_attributes_for_role(
    const RichTextSyntaxTheme& theme,
    RichTextSyntaxRole role) {
  switch (role) {
    case RichTextSyntaxRole::text:
      return theme.text;
    case RichTextSyntaxRole::keyword:
      return theme.keyword;
    case RichTextSyntaxRole::string_literal:
      return theme.string_literal;
    case RichTextSyntaxRole::number:
      return theme.number;
    case RichTextSyntaxRole::comment:
      return theme.comment;
    case RichTextSyntaxRole::type_name:
      return theme.type_name;
    case RichTextSyntaxRole::function_name:
      return theme.function_name;
    case RichTextSyntaxRole::variable:
      return theme.variable;
    case RichTextSyntaxRole::punctuation:
      return theme.punctuation;
    case RichTextSyntaxRole::operator_token:
      return theme.operator_token;
  }

  return {};
}

void build_rich_text_syntax_spans(
    std::string_view text,
    std::span<const RichTextSyntaxToken> tokens,
    const RichTextSyntaxTheme& theme,
    std::vector<RichTextSpan>& spans) {
  spans.clear();
  spans.reserve(tokens.size());
  for (const RichTextSyntaxToken& token : tokens) {
    const std::optional<RichTextSpan> span =
        clipped_syntax_token(text, token, theme);
    if (span.has_value()) {
      spans.push_back(*span);
    }
  }
  sort_syntax_spans(spans);
}

std::vector<RichTextSpan> build_rich_text_syntax_spans(
    std::string_view text,
    std::span<const RichTextSyntaxToken> tokens,
    const RichTextSyntaxTheme& theme) {
  std::vector<RichTextSpan> spans;
  build_rich_text_syntax_spans(text, tokens, theme, spans);
  return spans;
}

} // namespace cgpui
