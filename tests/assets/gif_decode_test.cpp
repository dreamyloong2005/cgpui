#include "cgpui/renderer/gif_decode.hpp"

#include <cstdint>
#include <vector>

namespace {

void append(std::vector<std::uint8_t>& bytes, const std::vector<std::uint8_t>& tail) {
  bytes.insert(bytes.end(), tail.begin(), tail.end());
}

std::vector<std::uint8_t> animated_gif(std::uint16_t repeat_count) {
  std::vector<std::uint8_t> bytes{
      'G', 'I', 'F', '8', '9', 'a', 1, 0, 1, 0, 0x80, 0, 0,
      0,   0,   0,   255, 255, 255};
  append(bytes, {0x21, 0xff, 0x0b, 'N', 'E', 'T', 'S', 'C', 'A', 'P',
                 'E', '2', '.', '0', 0x03, 0x01,
                 static_cast<std::uint8_t>(repeat_count & 0xff),
                 static_cast<std::uint8_t>(repeat_count >> 8), 0x00});
  const auto frame = [&](std::uint16_t delay_centiseconds) {
    append(bytes,
           {0x21, 0xf9, 0x04, 0x00,
            static_cast<std::uint8_t>(delay_centiseconds & 0xff),
            static_cast<std::uint8_t>(delay_centiseconds >> 8), 0x00, 0x00});
    append(bytes, {0x2c, 0, 0, 0, 0, 1, 0, 1, 0, 0,
                   0x02, 0x01, 0x4c, 0x00});
  };
  frame(5);
  frame(10);
  bytes.push_back(0x3b);
  return bytes;
}

int test_decodes_all_frames_delays_and_infinite_loop() {
  const auto result = cgpui::decode_gif(animated_gif(0));
  if (!result.ready() || result.gif.width != 1 || result.gif.height != 1 ||
      !result.gif.loop.infinite || result.gif.loop.repeat_count != 0 ||
      result.gif.frames.size() != 2 ||
      result.gif.frames[0].duration_ms != 50 ||
      result.gif.frames[1].duration_ms != 100) {
    return 10;
  }
  for (const auto& frame : result.gif.frames) {
    if (frame.bitmap.width != 1 || frame.bitmap.height != 1 ||
        frame.bitmap.stride != 4 ||
        frame.bitmap.format != cgpui::ImageFormat::rgba8_unorm ||
        frame.bitmap.pixels.size() != 4 || frame.bitmap.pixels[3] != 255) {
      return 11;
    }
  }
  return 0;
}

int test_preserves_finite_loop_count() {
  const auto result = cgpui::decode_gif(animated_gif(3));
  return result.ready() && !result.gif.loop.infinite &&
                 result.gif.loop.repeat_count == 3
             ? 0
             : 20;
}

int test_rejects_invalid_and_oversized_gifs() {
  auto corrupt = animated_gif(0);
  corrupt.resize(24);
  const auto empty = cgpui::decode_gif({});
  const auto unsupported =
      cgpui::decode_gif(std::vector<std::uint8_t>{0x89, 'P', 'N', 'G'});
  const auto invalid = cgpui::decode_gif(corrupt);
  const auto frames = cgpui::decode_gif(
      animated_gif(0), cgpui::GifDecodeLimits{.max_frames = 1});
  const auto bytes = cgpui::decode_gif(
      animated_gif(0),
      cgpui::GifDecodeLimits{.max_total_decoded_bytes = 7});
  const auto screen = cgpui::decode_gif(
      animated_gif(0),
      cgpui::GifDecodeLimits{
          .image = cgpui::ImageDecodeLimits{.max_pixels = 0}});
  const auto generic = cgpui::decode_image(animated_gif(0));
  return empty.status == cgpui::GifDecodeStatus::empty_input &&
                 unsupported.status ==
                     cgpui::GifDecodeStatus::unsupported_format &&
                 invalid.status == cgpui::GifDecodeStatus::invalid_data &&
                 frames.status == cgpui::GifDecodeStatus::exceeds_limits &&
                 bytes.status == cgpui::GifDecodeStatus::exceeds_limits &&
                 screen.status == cgpui::GifDecodeStatus::exceeds_limits &&
                 generic.source_format == cgpui::EncodedImageFormat::gif &&
                 generic.status == cgpui::ImageDecodeStatus::unsupported_format
             ? 0
             : 30;
}

} // namespace

int main() {
  if (const int result = test_decodes_all_frames_delays_and_infinite_loop()) {
    return result;
  }
  if (const int result = test_preserves_finite_loop_count()) return result;
  return test_rejects_invalid_and_oversized_gifs();
}
