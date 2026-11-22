#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string header =
      read_source("include/cgpui/renderer/image_decode.hpp");
  const std::string source =
      read_source("src/renderer/image_decode_stb.cpp");
  const std::string behavior =
      read_source("tests/assets/image_decode_test.cpp");
  const std::string vendor = read_source("third_party/stb/README.md");
  const std::string stb = read_source("third_party/stb/stb_image.h");
  const std::string prelude = read_source("include/cgpui/prelude.hpp");
  const std::string cleanliness =
      read_source("tests/header_cleanliness/prelude_header_cleanliness.cpp");
  const std::string inventory =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string manifests = read_source("packages/renderer/mcpp.toml") +
      read_source("tests/assets/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &header, &source, &behavior, &vendor, &stb, &prelude, &cleanliness,
      &inventory, &manifests, &vocabulary, &core, &ledger_json};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "enum class EncodedImageFormat") ||
      !contains(header, "enum class ImageDecodeStatus") ||
      !contains(header, "struct ImageDecodeLimits") ||
      !contains(header, "struct ImageDecodeResult") ||
      !contains(header, "decode_image(") || contains(header, "stb_image")) {
    return 2;
  }
  if (!contains(source, "STBI_ONLY_PNG") ||
      !contains(source, "STBI_ONLY_JPEG") ||
      !contains(source, "STBI_NO_STDIO") ||
      !contains(source, "stbi_info_from_memory(") ||
      !contains(source, "exceeds_decode_limits(") ||
      !contains(source, "stbi_load_from_memory(") ||
      !contains(source, "std::unique_ptr<stbi_uc") ||
      source.find("stbi_info_from_memory(") >
          source.find("stbi_load_from_memory(")) return 3;
  if (!contains(behavior, "test_decodes_png_to_rgba8") ||
      !contains(behavior, "test_decodes_jpeg_to_rgba8") ||
      !contains(behavior, "test_rejects_invalid_and_oversized_inputs") ||
      !contains(behavior, "max_decoded_bytes = 7") ||
      !contains(cleanliness, "ImageDecodeStatus::empty_input")) return 4;
  if (!contains(vendor, "28d546d5eb77d4585506a20480f4de2e706dff4c") ||
      !contains(vendor,
                "594C2FE35D49488B4382DBFAEC8F98366DEFCA819D916AC95BECF3E75F4200B3") ||
      !contains(stb, "stbi_load_from_memory") ||
      !contains(manifests, "\"../../third_party/stb\"") ||
      contains(manifests, "vendor.stb") ||
      !contains(prelude, "cgpui/renderer/image_decode.hpp")) return 5;
  if (!contains(inventory, "include/cgpui/renderer/image_decode.hpp") ||
      !contains(inventory, "src/renderer/image_decode_stb.cpp") ||
      !contains(manifests, "\"image_decode_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_png_jpeg_decode_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(header) > 55 || line_count(source) > 125 ||
      line_count(behavior) > 130) return 7;

  if (!contains(vocabulary, "`decode_image(...)`") ||
      !contains(core, "Signature-detected PNG and JPEG decoding") ||
      !contains(ledger_json, "\"phase_g_step_652_sources\"")) return 9;
  return 0;
}
