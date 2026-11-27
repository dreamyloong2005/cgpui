#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const char* path) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!stream) throw std::runtime_error(std::string("missing recording owner: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
std::string compact(std::string_view source) {
  std::string result;
  for (std::size_t at = 0; at < source.size();) {
    if (source.substr(at, 2) == "//") {
      const auto end = source.find('\n', at + 2);
      at = end == source.npos ? source.size() : end + 1;
    } else if (source.substr(at, 2) == "/*") {
      const auto end = source.find("*/", at + 2);
      at = end == source.npos ? source.size() : end + 2;
    } else {
      const char value = source[at++];
      if (!std::isspace(static_cast<unsigned char>(value))) result += value;
      if (value != '"' && value != '\'') continue;
      while (at < source.size()) {
        const char character = source[at++];
        result += character;
        if (character == '\\' && at < source.size()) result += source[at++];
        else if (character == value) break;
      }
    }
  }
  return result;
}
void cap(const std::string& source, std::size_t limit) {
  const auto lines = std::count(source.begin(), source.end(), '\n') +
      (!source.empty() && source.back() != '\n' ? 1 : 0);
  require(static_cast<std::size_t>(lines) <= limit, "recording owner exceeds its existing budget");
}
void grouped_entry(const std::string& source) {
  constexpr std::string_view name = "record_vulkan_frame_command_buffer(";
  const auto begin = source.find(name);
  require(begin != source.npos && source.find(name, begin + name.size()) == source.npos,
      "recording has one grouped entry, not an expanded overload");
  const auto end = source.find(')', begin + name.size());
  require(end != source.npos, "recording declaration is incomplete");
  const std::string_view parameters(source.data() + begin + name.size(), end - begin - name.size());
  const auto comma = parameters.find(',');
  require(comma != parameters.npos && parameters.find(',', comma + 1) == parameters.npos &&
      parameters.starts_with("constVulkanFrameRecordingInput&") &&
      parameters.substr(comma + 1).starts_with("VulkanFrameCommandReuseState&"),
      "recording accepts borrowed grouped input and mutable reuse state only");
}
} // namespace

int main() {
  try {
    const auto input = read("src/renderer/vulkan/vulkan_frame_recording_input_internal.hpp");
    const auto header = read("src/renderer/vulkan/vulkan_command_recording_internal.hpp");
    const auto recording = read("src/renderer/vulkan/vulkan_command_recording.cpp");
    const auto presentation = read("src/renderer/vulkan/vulkan_presentation.cpp");
    const auto declarations = compact(input);
    // Exact field sets prevent adding owning containers or copying resource objects.
    constexpr std::array borrowed_groups{
        "structVulkanFrameRecordingTarget{VkCommandBuffercommand_buffer;"
        "VkRenderPassrender_pass;VkFramebufferframebuffer;VkExtent2Dextent;Colorclear_color;};",
        "structVulkanFrameGeometryResources{constVulkanRoundedRectPipelineResources&pipeline;"
        "constVulkanRoundedRectBufferResources&solid_rect_buffers;"
        "constVulkanRoundedRectBufferResources&rounded_rect_buffers;};",
        "structVulkanFrameTextResources{constVulkanTextPipelineResources&pipeline;"
        "constVulkanTextVertexBufferResources&vertex_buffer;constVulkanGlyphAtlasResources&glyph_atlas;"
        "std::span<constTexturedGlyphQuad>glyph_atlas_draw_quads;"
        "std::span<constVulkanGlyphAtlasDrawBinding>glyph_atlas_draw_bindings;"
        "constVulkanGlyphAtlasUploadResources&uploads;};",
        "structVulkanFrameImageResources{constVulkanImagePipelineResources&pipeline;"
        "constVulkanImageVertexBufferResources&vertex_buffer;std::span<constImageDraw>draws;"
        "constVulkanImageTextureResources&textures;constVulkanImageTextureUploadResources&uploads;};",
        "structVulkanFrameRecordingInput{VulkanFrameRecordingTargettarget;"
        "VulkanFrameGeometryResourcesgeometry;VulkanFrameTextResourcestext;VulkanFrameImageResourcesimages;"
        "std::span<constVulkanFrameDrawOrderEntry>draw_order;"
        "constVulkanFramePixelCaptureCommand&pixel_capture;VulkanFrameEffectseffects;};"};
    for (const auto* group : borrowed_groups)
      require(declarations.find(group) != declarations.npos, "recording inputs must remain borrowed resource groups");
    grouped_entry(compact(header));
    grouped_entry(compact(recording));
    for (const auto* source : {&input, &header, &recording}) {
      const auto code = compact(*source);
      require(code.find("#include\"vulkan_internal.hpp\"") == code.npos &&
          code.find("#include<vulkan_internal.hpp>") == code.npos &&
          code.find("VulkanRendererState") == code.npos,
          "recording must not depend on the broad renderer state");
    }
    const auto caller = compact(presentation);
    for (const auto* group : {".target={", ".geometry={", ".text={", ".images={"})
      require(caller.find(group) != caller.npos, "presentation assembles named recording groups");
    cap(input, 70);
    cap(header, 40);
    cap(recording, 220);
    cap(presentation, 180);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
