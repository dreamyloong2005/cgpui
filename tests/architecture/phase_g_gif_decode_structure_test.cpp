#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
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
      read_source("include/cgpui/renderer/gif_decode.hpp");
  const std::string image_header =
      read_source("include/cgpui/renderer/image_decode.hpp");
  const std::string internal =
      read_source("src/renderer/gif_metadata_internal.hpp");
  const std::string metadata =
      read_source("src/renderer/gif_metadata.cpp");
  const std::string decoder =
      read_source("src/renderer/gif_decode_stb.cpp");
  const std::string image_decoder =
      read_source("src/renderer/image_decode_stb.cpp");
  const std::string format_internal =
      read_source("src/renderer/image_format_detection_internal.hpp");
  const std::string format_source =
      read_source("src/renderer/image_format_detection.cpp");
  const std::string behavior = read_source("tests/assets/gif_decode_test.cpp");
  const std::string prelude = read_source("include/cgpui/prelude.hpp");
  const std::string cleanliness =
      read_source("tests/header_cleanliness/prelude_header_cleanliness.cpp");
  const std::string inventory =
      read_source("tests/architecture/renderer_source_structure_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_png_jpeg_decode_structure_test.cpp");
  const std::string stb = read_source("third_party/stb/stb_image.h");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &image_header, &internal, &metadata, &decoder, &image_decoder,
      &format_internal, &format_source, &behavior, &prelude, &cleanliness,
      &inventory, &previous, &stb, &xmake, &vocabulary, &core, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "enum class GifDecodeStatus") ||
      !contains(header, "struct GifLoopBehavior") ||
      !contains(header, "struct DecodedGifFrame") ||
      !contains(header, "struct DecodedGif") ||
      !contains(header, "struct GifDecodeLimits") ||
      !contains(header, "decode_gif(") || contains(header, "stb_image")) {
    return 2;
  }
  if (!contains(internal, "struct GifMetadataScan") ||
      !contains(internal, "scan_gif_metadata(") ||
      !contains(metadata, "class GifReader") ||
      !contains(metadata, "skip_sub_blocks(") ||
      !contains(metadata, "read_loop_extension(") ||
      !contains(metadata, "frame_count > limits.max_frames") ||
      !contains(metadata, "limits.max_total_decoded_bytes / frame_bytes")) {
    return 3;
  }
  if (!contains(decoder, "scan_gif_metadata(") ||
      !contains(decoder, "stbi_load_gif_from_memory(") ||
      !contains(decoder, "metadata.frame_durations_ms") ||
      !contains(decoder, "std::unique_ptr<stbi_uc") ||
      decoder.find("scan_gif_metadata(") >
          decoder.find("stbi_load_gif_from_memory(")) return 4;
  if (!contains(behavior, "test_decodes_all_frames_delays_and_infinite_loop") ||
      !contains(behavior, "test_preserves_finite_loop_count") ||
      !contains(behavior, "test_rejects_invalid_and_oversized_gifs") ||
      !contains(behavior, "max_frames = 1") ||
      !contains(behavior, "max_total_decoded_bytes = 7") ||
      !contains(behavior, "EncodedImageFormat::gif")) return 5;
  if (!contains(image_header, "gif,") ||
      !contains(image_decoder, "STBI_ONLY_GIF") ||
      !contains(image_decoder, "detect_encoded_image_format(") ||
      !contains(format_internal, "detect_encoded_image_format(") ||
      !contains(format_source, "EncodedImageFormat::gif") ||
      !contains(stb, "stbi_load_gif_from_memory") ||
      !contains(prelude, "cgpui/renderer/gif_decode.hpp") ||
      !contains(cleanliness, "GifDecodeStatus::empty_input")) return 6;
  if (!contains(inventory, "include/cgpui/renderer/gif_decode.hpp") ||
      !contains(inventory, "src/renderer/gif_metadata_internal.hpp") ||
      !contains(inventory, "src/renderer/gif_metadata.cpp") ||
      !contains(inventory, "src/renderer/gif_decode_stb.cpp") ||
      !contains(inventory,
                "src/renderer/image_format_detection_internal.hpp") ||
      !contains(inventory, "src/renderer/image_format_detection.cpp") ||
      !contains(previous, "Step 653 GIF ") ||
      !contains(previous, "decode boundary production behavior is next.") ||
      !contains(xmake, "target(\"phase_g_gif_decode_test\")") ||
      !contains(xmake, "target(\"phase_g_gif_decode_structure_test\")")) {
    return 7;
  }
  if (line_count(header) > 65 || line_count(internal) > 35 ||
      line_count(metadata) > 230 || line_count(decoder) > 100 ||
      line_count(behavior) > 120) return 8;

  constexpr const char* completion =
      "Phase G Step 653 adds bounded animated GIF decoding with complete "
      "composited RGBA8 frames, per-frame delays, finite and infinite loop "
      "metadata, structured pre-decode block scanning, and frame plus total-"
      "byte limits. Step 654 SVG asset decode boundary production behavior "
      "is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 9;
  }
  if (!contains(vocabulary, "`decode_gif(...)`") ||
      !contains(core, "Animated GIF decoding preserves bounded") ||
      !contains(ledger_json, "\"phase_g_step_653_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 669 Windows and Linux release build and packaging coverage\"")) return 10;
  return 0;
}
