#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!stream) throw std::runtime_error(std::string("missing image owner: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
bool before(const std::string& s, const char* a, const char* b) {
  return has(s, a) && has(s, b) && s.find(a) < s.find(b);
}
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
void cap(const char* path, unsigned count) {
  const auto text = read(path);
  require(std::count(text.begin(), text.end(), '\n') <= count, "missing-image owner exceeded focused cap");
}
}
int main() {
  try {
    const auto leaf = read("src/renderer/renderer_image_validation_internal.hpp");
    const auto validation = read("src/renderer/renderer_image_validation.cpp");
    const auto resolve = read("src/renderer/dx12/dx12_image_resolve.cpp");
    const auto marker = read("src/renderer/dx12/dx12_missing_image_internal.hpp");
    require(!has(leaf, "return ") && has(leaf, "RendererValidatedImageDraw") && !has(leaf, "Dx12") && !has(leaf, "Vk"),
        "shared validation must be private, declaration-only and native-independent");
    require(before(validation, "finite(draw.bounds)", "build_image_vertices(") &&
        has(validation, "draw.composition_stack.entries") && has(validation, "draw.clip_stack.current_clip_rect") &&
        has(validation, "vertex.image_uv") && has(validation, "vertex.color"), "raw and derived validation split regressed");
    require(before(resolve, "renderer_validate_image_draw(device)", "state.image_cache->lookup(device.asset)") &&
        before(resolve, "D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION", "state.image_cache->lookup(device.asset)") &&
        has(resolve, "if (!texture) return std::unexpected(texture.error())") &&
        has(resolve, "if (!*texture) return Dx12ResolvedImageDraw{Dx12MissingImageDraw"), "only valid true absence may skip");
    require(has(marker, "std::optional<Rect> clip_rect") && !has(marker, "Texture") && !has(marker, "ImageAsset") &&
        has(read("src/renderer/dx12/dx12_frame_diagnostic_plan.cpp"), "std::is_same_v<Payload, Dx12MissingImageDraw>") &&
        has(read("src/renderer/dx12/dx12_draw_dispatch.cpp"), "operator()(const Dx12MissingImageDraw&) const { return {}; }") &&
        has(read("src/renderer/dx12/dx12_draw_dispatch.cpp"), "operator()(const Dx12PendingImageDraw&)"),
        "missing must plan, never submit/native-own, and remain distinct from unresolved");
    const auto prepare = read("src/renderer/dx12/dx12_image_prepare.cpp");
    require(before(prepare, "dx12_validate_image_request_identities(draws)", "state.image_cache->invalidate(id)") &&
        has(prepare, "uploads.swap(uploads_)") && has(prepare, "invalidations.swap(invalidations_)") &&
        has(prepare, "command.payload = std::move(payload)") && !has(prepare, "command_index ="),
        "same-ID preflight, raw lifetime or original slot preservation regressed");
    const auto accounting = read("src/renderer/vulkan/vulkan_image_submission_diagnostics.cpp");
    const auto recording = read("src/renderer/vulkan/vulkan_command_recording.cpp");
    require(has(accounting, "VulkanFrameDrawOrderCursor cursor") && has(accounting, "vulkan_resolve_clip_stack_scissor") &&
        !has(accounting, "image_texture") && !has(accounting, "vkCmd") &&
        has(recording, ".submitted_image_indices = vulkan_submitted_image_indices") &&
        has(read("src/renderer/vulkan/vulkan_presentation.cpp"), "recording->submitted_image_indices") &&
        !has(read("src/renderer/vulkan/vulkan_frame_diagnostic_resources.cpp"), "image_draws.size()"),
        "Vulkan diagnostics need current successful selected commands, not request count or stale cache signature");
    const auto vkframe = read("src/renderer/vulkan/vulkan_image_texture_frame.cpp");
    require(before(vkframe, "vulkan_validate_image_frame_requests", "vulkan_prepare_image_upload_retirement") &&
        has(read("src/renderer/vulkan/vulkan_image_request_validation.cpp"), "vulkan_image_texture_resource_matches"),
        "Vulkan request/cache compatibility must precede resource mutation");
    require(has(read("tests/renderer/mcpp.toml"), "renderer_missing_image_contract_test.cpp") &&
        has(read("tests/renderer/mcpp.toml"), "renderer_missing_image_validation_test.cpp") &&
        has(read("tests/renderer-dx12/mcpp.toml"), "dx12_missing_image_test.cpp") &&
        has(read("tests/architecture/mcpp.toml"), "dx12/*_structure_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"), "dx12_missing_image_structure_test|"), "missing-image tests unregistered");
    cap("src/renderer/renderer_image_validation_internal.hpp", 25);
    cap("src/renderer/renderer_image_validation.cpp", 90);
    cap("src/renderer/dx12/dx12_missing_image_internal.hpp", 20);
    cap("src/renderer/dx12/dx12_image_request_validation.cpp", 45);
    cap("src/renderer/dx12/dx12_image_resolve.cpp", 80);
    cap("src/renderer/vulkan/vulkan_image_submission_diagnostics_internal.hpp", 20);
    cap("src/renderer/vulkan/vulkan_image_submission_diagnostics.cpp", 40);
    cap("src/renderer/vulkan/vulkan_image_request_validation_internal.hpp", 20);
    cap("src/renderer/vulkan/vulkan_image_request_validation.cpp", 80);
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
