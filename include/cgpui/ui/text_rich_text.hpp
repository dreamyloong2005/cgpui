#pragma once

#include "cgpui/ui/style_values.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace cgpui {

enum class RichTextDecoration : std::uint8_t {
  none = 0,
  underline = 1,
  strikethrough = 2,
};

[[nodiscard]] constexpr RichTextDecoration operator|(
    RichTextDecoration left,
    RichTextDecoration right) {
  return static_cast<RichTextDecoration>(
      static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
}

[[nodiscard]] constexpr bool rich_text_has_decoration(
    RichTextDecoration decorations,
    RichTextDecoration value) {
  return (static_cast<std::uint8_t>(decorations) &
          static_cast<std::uint8_t>(value)) != 0U;
}

struct RichTextLinkId {
  std::uint64_t value = 0;

  friend bool operator==(const RichTextLinkId&, const RichTextLinkId&) =
      default;
};

struct RichTextAttributes {
  std::optional<Color> foreground;
  std::optional<Color> background;
  RichTextDecoration decorations = RichTextDecoration::none;
  std::optional<RichTextLinkId> link_id;
};

[[nodiscard]] bool operator==(
    const RichTextAttributes& left,
    const RichTextAttributes& right);

struct RichTextSpan {
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  RichTextAttributes attributes;
};

struct RichTextRun {
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  RichTextAttributes attributes;
};

struct RichTextRunBuildScratch {
  std::vector<std::size_t> endpoints;
};

struct RichTextRunHit {
  std::size_t run_index = 0;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  RichTextAttributes attributes;
};

struct RichTextLinkHit {
  RichTextLinkId link_id;
  std::size_t run_index = 0;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
};

[[nodiscard]] RichTextAttributes merge_rich_text_attributes(
    RichTextAttributes base,
    const RichTextAttributes& overlay);

void build_rich_text_runs(
    std::string_view text,
    std::span<const RichTextSpan> spans,
    std::vector<RichTextRun>& runs,
    RichTextRunBuildScratch& scratch);

void build_rich_text_runs(
    std::string_view text,
    std::span<const RichTextSpan> spans,
    std::vector<RichTextRun>& runs);

[[nodiscard]] std::vector<RichTextRun> build_rich_text_runs(
    std::string_view text,
    std::span<const RichTextSpan> spans);

[[nodiscard]] std::optional<RichTextRunHit> rich_text_run_at_byte_offset(
    std::span<const RichTextRun> runs,
    std::size_t byte_offset);

[[nodiscard]] std::optional<RichTextLinkHit> rich_text_link_at_byte_offset(
    std::span<const RichTextRun> runs,
    std::size_t byte_offset);

} // namespace cgpui
