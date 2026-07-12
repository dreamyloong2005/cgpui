#include "cgpui/renderer/image_decode.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace {

std::vector<std::uint8_t> decode_base64(std::string_view encoded) {
  const auto value = [](char character) -> int {
    if (character >= 'A' && character <= 'Z') return character - 'A';
    if (character >= 'a' && character <= 'z') return character - 'a' + 26;
    if (character >= '0' && character <= '9') return character - '0' + 52;
    if (character == '+') return 62;
    if (character == '/') return 63;
    return -1;
  };
  std::vector<std::uint8_t> bytes;
  int accumulator = 0;
  int bits = -8;
  for (const char character : encoded) {
    if (character == '=') break;
    const int next = value(character);
    if (next < 0) continue;
    accumulator = (accumulator << 6) | next;
    bits += 6;
    if (bits >= 0) {
      bytes.push_back(static_cast<std::uint8_t>((accumulator >> bits) & 0xff));
      bits -= 8;
    }
  }
  return bytes;
}

std::vector<std::uint8_t> png_bytes() {
  return decode_base64(
      "iVBORw0KGgoAAAANSUhEUgAAAAIAAAABCAYAAAD0In+KAAAAAXNSR0IArs4c6QAA"
      "AARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAAOSURBVBhXY/jP"
      "wPAfBAEQ+AP9OGZFWQAAAABJRU5ErkJggg==");
}

std::vector<std::uint8_t> jpeg_bytes() {
  return decode_base64(
      "/9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAMCAgMCAgMDAwMEAwMEBQgFBQQEBQoH"
      "BwYIDAoMDAsKCwsNDhIQDQ4RDgsLEBYQERMUFRUVDA8XGBYUGBIUFRT/2wBDAQME"
      "BAUEBQkFBQkUDQsNFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQU"
      "FBQUFBQUFBQUFBQUFBT/wAARCAABAAIDASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEA"
      "AAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIh"
      "MUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6"
      "Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZ"
      "mqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx"
      "8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREA"
      "AgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAV"
      "YnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hp"
      "anN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPE"
      "xcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwDo"
      "vBn/ACJ+hf8AXhB/6LWiiiv4mr/xZ+r/ADP4J4p/5H+P/wCv1X/0uR//2Q==");
}

int test_decodes_png_to_rgba8() {
  const auto result = cgpui::decode_image(png_bytes());
  const std::vector<std::uint8_t> expected{
      255, 0, 0, 255, 0, 255, 0, 255};
  return result.ready() && result.source_format == cgpui::EncodedImageFormat::png &&
                 result.bitmap.width == 2 && result.bitmap.height == 1 &&
                 result.bitmap.stride == 8 &&
                 result.bitmap.format == cgpui::ImageFormat::rgba8_unorm &&
                 result.bitmap.pixels == expected
             ? 0
             : 10;
}

int test_decodes_jpeg_to_rgba8() {
  const auto result = cgpui::decode_image(jpeg_bytes());
  if (!result.ready() ||
      result.source_format != cgpui::EncodedImageFormat::jpeg ||
      result.bitmap.width != 2 || result.bitmap.height != 1 ||
      result.bitmap.stride != 8 || result.bitmap.pixels.size() != 8) {
    return 20;
  }
  return result.bitmap.pixels[3] == 255 && result.bitmap.pixels[7] == 255
             ? 0
             : 21;
}

int test_rejects_invalid_and_oversized_inputs() {
  auto corrupt = png_bytes();
  corrupt.resize(12);
  const auto empty = cgpui::decode_image({});
  const auto unsupported = cgpui::decode_image(
      std::vector<std::uint8_t>{0x47, 0x49, 0x46, 0x38});
  const auto invalid = cgpui::decode_image(corrupt);
  const auto wide = cgpui::decode_image(
      png_bytes(), cgpui::ImageDecodeLimits{.max_width = 1});
  const auto bytes = cgpui::decode_image(
      png_bytes(), cgpui::ImageDecodeLimits{.max_decoded_bytes = 7});
  return empty.status == cgpui::ImageDecodeStatus::empty_input &&
                 unsupported.status ==
                     cgpui::ImageDecodeStatus::unsupported_format &&
                 invalid.status == cgpui::ImageDecodeStatus::invalid_data &&
                 wide.status == cgpui::ImageDecodeStatus::exceeds_limits &&
                 bytes.status == cgpui::ImageDecodeStatus::exceeds_limits
             ? 0
             : 30;
}

} // namespace

int main() {
  if (const int result = test_decodes_png_to_rgba8()) return result;
  if (const int result = test_decodes_jpeg_to_rgba8()) return result;
  return test_rejects_invalid_and_oversized_inputs();
}
