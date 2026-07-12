#include "gif_metadata_internal.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace cgpui::detail {
namespace {

class GifReader {
 public:
  explicit GifReader(std::span<const std::uint8_t> bytes) : bytes_(bytes) {}

  bool read_u8(std::uint8_t& value) {
    if (cursor_ == bytes_.size()) return false;
    value = bytes_[cursor_++];
    return true;
  }

  bool read_u16(std::uint16_t& value) {
    std::uint8_t low = 0;
    std::uint8_t high = 0;
    if (!read_u8(low) || !read_u8(high)) return false;
    value = static_cast<std::uint16_t>(low | (high << 8));
    return true;
  }

  bool read_bytes(std::size_t count, std::span<const std::uint8_t>& value) {
    if (count > bytes_.size() - cursor_) return false;
    value = bytes_.subspan(cursor_, count);
    cursor_ += count;
    return true;
  }

  bool skip(std::size_t count) {
    std::span<const std::uint8_t> ignored;
    return read_bytes(count, ignored);
  }

 private:
  std::span<const std::uint8_t> bytes_;
  std::size_t cursor_ = 0;
};

std::size_t color_table_bytes(std::uint8_t packed) {
  return 3U * (1U << ((packed & 0x07U) + 1U));
}

bool skip_sub_blocks(GifReader& reader) {
  for (;;) {
    std::uint8_t size = 0;
    if (!reader.read_u8(size)) return false;
    if (size == 0) return true;
    if (!reader.skip(size)) return false;
  }
}

bool gif_signature(std::span<const std::uint8_t> encoded) {
  constexpr std::array<std::uint8_t, 6> gif87{'G', 'I', 'F', '8', '7', 'a'};
  constexpr std::array<std::uint8_t, 6> gif89{'G', 'I', 'F', '8', '9', 'a'};
  return encoded.size() >= 6 &&
         (std::equal(gif87.begin(), gif87.end(), encoded.begin()) ||
          std::equal(gif89.begin(), gif89.end(), encoded.begin()));
}

bool screen_exceeds_limits(
    std::uint16_t width,
    std::uint16_t height,
    const GifDecodeLimits& limits,
    std::size_t& frame_bytes) {
  const auto pixels = static_cast<std::uint64_t>(width) * height;
  const auto bytes = pixels * 4U;
  if (width == 0 || height == 0 || width > limits.image.max_width ||
      height > limits.image.max_height || pixels > limits.image.max_pixels ||
      bytes > limits.image.max_decoded_bytes ||
      bytes > std::numeric_limits<std::size_t>::max()) {
    return true;
  }
  frame_bytes = static_cast<std::size_t>(bytes);
  return false;
}

bool read_loop_extension(
    GifReader& reader,
    std::span<const std::uint8_t> identifier,
    GifLoopBehavior& loop) {
  constexpr std::array<std::uint8_t, 11> netscape{
      'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0'};
  constexpr std::array<std::uint8_t, 11> animexts{
      'A', 'N', 'I', 'M', 'E', 'X', 'T', 'S', '1', '.', '0'};
  const bool recognized =
      std::equal(identifier.begin(), identifier.end(), netscape.begin()) ||
      std::equal(identifier.begin(), identifier.end(), animexts.begin());
  for (;;) {
    std::uint8_t size = 0;
    if (!reader.read_u8(size)) return false;
    if (size == 0) return true;
    std::span<const std::uint8_t> payload;
    if (!reader.read_bytes(size, payload)) return false;
    if (recognized && payload.size() >= 3 && payload[0] == 1) {
      loop.repeat_count = static_cast<std::uint16_t>(
          payload[1] | (static_cast<std::uint16_t>(payload[2]) << 8));
      loop.infinite = loop.repeat_count == 0;
    }
  }
}

} // namespace

GifMetadataScan scan_gif_metadata(
    std::span<const std::uint8_t> encoded,
    const GifDecodeLimits& limits) {
  GifMetadataScan result;
  if (encoded.empty()) {
    result.status = GifDecodeStatus::empty_input;
    return result;
  }
  if (!gif_signature(encoded)) {
    result.status = GifDecodeStatus::unsupported_format;
    return result;
  }

  GifReader reader(encoded.subspan(6));
  std::uint16_t width = 0;
  std::uint16_t height = 0;
  std::uint8_t packed = 0;
  std::uint8_t ignored = 0;
  if (!reader.read_u16(width) || !reader.read_u16(height) ||
      !reader.read_u8(packed) || !reader.read_u8(ignored) ||
      !reader.read_u8(ignored)) return result;
  std::size_t frame_bytes = 0;
  if (screen_exceeds_limits(width, height, limits, frame_bytes)) {
    result.status = GifDecodeStatus::exceeds_limits;
    return result;
  }
  if ((packed & 0x80U) != 0 && !reader.skip(color_table_bytes(packed))) {
    return result;
  }

  std::uint16_t pending_delay = 0;
  bool trailer = false;
  for (;;) {
    std::uint8_t marker = 0;
    if (!reader.read_u8(marker)) break;
    if (marker == 0x3b) {
      trailer = true;
      break;
    }
    if (marker == 0x21) {
      std::uint8_t label = 0;
      if (!reader.read_u8(label)) return result;
      if (label == 0xf9) {
        std::uint8_t size = 0;
        std::uint8_t terminator = 0;
        if (!reader.read_u8(size) || size != 4 || !reader.skip(1) ||
            !reader.read_u16(pending_delay) || !reader.skip(1) ||
            !reader.read_u8(terminator) || terminator != 0) return result;
      } else if (label == 0xff) {
        std::uint8_t size = 0;
        std::span<const std::uint8_t> identifier;
        if (!reader.read_u8(size) || size != 11 ||
            !reader.read_bytes(size, identifier) ||
            !read_loop_extension(reader, identifier, result.loop)) return result;
      } else {
        if (label == 0x01) {
          std::uint8_t size = 0;
          if (!reader.read_u8(size) || !reader.skip(size)) return result;
        }
        if (!skip_sub_blocks(reader)) return result;
      }
      continue;
    }
    if (marker != 0x2c) return result;

    std::uint16_t left = 0;
    std::uint16_t top = 0;
    std::uint16_t frame_width = 0;
    std::uint16_t frame_height = 0;
    if (!reader.read_u16(left) || !reader.read_u16(top) ||
        !reader.read_u16(frame_width) || !reader.read_u16(frame_height) ||
        !reader.read_u8(packed) || frame_width == 0 || frame_height == 0 ||
        static_cast<std::uint32_t>(left) + frame_width > width ||
        static_cast<std::uint32_t>(top) + frame_height > height) return result;
    if ((packed & 0x80U) != 0 && !reader.skip(color_table_bytes(packed))) {
      return result;
    }
    std::uint8_t lzw_code_size = 0;
    if (!reader.read_u8(lzw_code_size) || lzw_code_size < 2 ||
        lzw_code_size > 8 || !skip_sub_blocks(reader)) return result;

    const std::size_t frame_count = result.frame_durations_ms.size() + 1;
    if (frame_count > limits.max_frames ||
        frame_count > limits.max_total_decoded_bytes / frame_bytes) {
      result.status = GifDecodeStatus::exceeds_limits;
      return result;
    }
    result.frame_durations_ms.push_back(
        static_cast<std::uint32_t>(pending_delay) * 10U);
    pending_delay = 0;
  }
  if (!trailer || result.frame_durations_ms.empty()) return result;

  result.status = GifDecodeStatus::ready;
  result.width = width;
  result.height = height;
  result.total_decoded_bytes =
      frame_bytes * result.frame_durations_ms.size();
  return result;
}

} // namespace cgpui::detail
