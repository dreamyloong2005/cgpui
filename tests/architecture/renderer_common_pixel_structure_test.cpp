#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::filesystem::path& path) {
  std::ifstream file(root / path);
  if (!file) throw std::runtime_error("missing common pixel owner: " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, std::string_view token) {
  return source.find(token) != std::string::npos;
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
std::string section(const std::string& manifest, std::string_view header) {
  const auto start = manifest.find(header);
  require(start != std::string::npos, "missing platform-scoped manifest section");
  return manifest.substr(start, manifest.find("\n[", start + header.size()) - start);
}
bool factory_bypass(const std::string& source) {
  return has(source, "create_vulkan_renderer(") || has(source, "create_dx12_renderer(") ||
         has(source, "create_renderer(");
}
std::string pixel(const char* name) { return read(std::filesystem::path("tests/renderer") / name); }
} // namespace

int main() {
  try {
    const auto manifest = read("tests/renderer/mcpp.toml");
    const auto windows_tests = section(manifest, "[target.'cfg(os = \"windows\")'.test]");
    for (const auto* suffix : {"backdrop_blur_pixel", "box_shadow_pixel", "clip_pixel",
         "color_encoding_pixel", "frame_pixel_capture", "image_pixel", "opacity_pixel",
         "resize_pixel", "rounded_rect_pixel", "scale_pixel", "text_pixel",
         "text_ink_extent_pixel", "transform_pixel"}) {
      const auto filename = std::string("renderer_") + suffix + "_test.cpp";
      const auto source = pixel(filename.c_str());
      require(has(windows_tests, "\"" + filename + "\""), "all thirteen common scenes must run on Windows");
      require(!factory_bypass(source), "common scenes cannot bypass selected backend identity");
      require(has(source, "capture_pixels(") || has(source, "capture_pixels_at_scale(") ||
                  has(source, "create_pixel_test_renderer("), "common scene must use the shared native entry");
      require(!std::filesystem::exists(root / "tests/renderer" /
                  (std::string("vulkan_") + suffix + "_test.cpp")), "do not keep duplicate old pixel programs");
    }
    struct Limit { const char* file; std::size_t lines; };
    for (const auto& owner : std::array{
         Limit{"renderer_pixel_test_backend.hpp", 30}, Limit{"renderer_pixel_test_backend.cpp", 70},
         Limit{"renderer_pixel_test_backend_policy.cpp", 70}, Limit{"renderer_pixel_test_capture.cpp", 50},
         Limit{"renderer_pixel_test_probes.hpp", 25}, Limit{"renderer_pixel_test_probes.cpp", 60},
         Limit{"renderer_pixel_test_window.hpp", 40}, Limit{"renderer_pixel_test_window.cpp", 100},
         Limit{"renderer_pixel_test_support.hpp", 90}, Limit{"renderer_text_pixel_shape_support.hpp", 25},
         Limit{"renderer_text_pixel_shape_support.cpp", 45}, Limit{"renderer_box_shadow_pixel_support.hpp", 25},
         Limit{"renderer_box_shadow_pixel_support.cpp", 80}, Limit{"renderer_backdrop_pass_test_support.hpp", 20},
         Limit{"renderer_backdrop_pass_test_support.cpp", 70}, Limit{"renderer_pixel_backend_identity_test.cpp", 90},
         Limit{"vulkan_backdrop_pass_diagnostics_test.cpp", 40}, Limit{"dx12_backdrop_pass_diagnostics_test.cpp", 40}}) {
      const auto source = pixel(owner.file);
      require(std::count(source.begin(), source.end(), '\n') + 1 <= owner.lines,
              "common pixel support must retain focused implementation owners and caps");
    }
    const auto support = pixel("renderer_pixel_test_support.hpp");
    const auto backend = pixel("renderer_pixel_test_backend.cpp");
    const auto policy = pixel("renderer_pixel_test_backend_policy.cpp");
    const auto capture = pixel("renderer_pixel_test_capture.cpp");
    const auto probes = pixel("renderer_pixel_test_probes.cpp");
    require(has(support, "template <typename Paint>") && has(support, "create_pixel_test_renderer(") &&
                has(support, "begin_pixel_capture(") && has(support, "completed_pixel_capture(") &&
                !factory_bypass(support) && !has(support, "std::getenv(") &&
                !has(support, "CreateWindowExW(") && !has(support, "inline bool pixel_near("),
            "capture templates stay thin; selection, HWND hosting and probes have cpp owners");
    require(has(backend, "parse_pixel_test_backend(requested)") &&
                has(backend, "auto renderer = *expected == cgpui::RendererBackend::dx12") &&
                has(backend, "? cgpui::create_dx12_renderer(descriptor)") &&
                has(backend, ": cgpui::create_vulkan_renderer(descriptor)") && !has(backend, "create_renderer(") &&
                has(backend, "(*renderer)->backend_info()") && has(backend, "validate_pixel_backend_info(*expected, info, warp)") &&
                has(backend, "if (!*renderer)") && has(backend, "return std::unexpected(identity.error())") &&
                has(backend, "CGPUI_RENDERER") && has(backend, "CGPUI_DX12_ADAPTER") &&
                has(backend, "adapter == \"warp\"") && has(backend, "actual="),
            "one real creation owner must select the requested named backend, reject mismatch/fallback and verify WARP identity");
    require(has(policy, "requested.empty() || requested == \"auto\" || requested == \"vulkan\"") &&
                has(policy, "requested == \"dx12\"") && has(policy, "ErrorCode::invalid_argument") &&
                has(policy, "info.backend != expected") && has(policy, "info.selection != selection") &&
                has(policy, "!info.fallback_reason.empty()") && has(policy, "require_software && !info.software_adapter"),
            "selector defaults, invalid inputs and strict actual-identity policy cannot become silent skips");
    const auto frame_wait = read("tests/support/frame_wait_test_support.cpp");
    require(has(capture, "wait_for_test_frame(renderer)") &&
                has(frame_wait, "auto frame = renderer.begin_frame()") &&
                has(frame_wait, "frame.error().code != cgpui::ErrorCode::frame_not_ready) return frame") &&
                has(capture, "request_pixel_capture()") &&
                has(capture, "!pixels || !pixels->valid()") && has(capture, "report_pixel_test_error(") &&
                has(probes, "bool pixel_near(") && has(probes, "encoded_channel("),
            "non-template capture/error and pixel-probe implementations must remain in focused cpp leaves");
    const auto window = pixel("renderer_pixel_test_window.hpp");
    require(!has(window, "CreateWindowExW(") && !has(window, "PeekMessageW(") &&
                has(pixel("renderer_pixel_test_window.cpp"), "CreateWindowExW("),
            "native test-window implementation cannot return to its declaration header");
    const auto identity = pixel("renderer_pixel_backend_identity_test.cpp");
    require(!factory_bypass(identity) && has(identity, "capture_pixels(") && has(identity, "pixel_near(") &&
                has(identity, "RendererBackend::unknown") && has(identity, "unexpected fallback") &&
                has(identity, "\"both\"") && has(identity, "info.software_adapter = true"),
            "identity regression must exercise strict negative cases and the same real pixel route as the scenes");
    const auto diagnostic_scene = pixel("renderer_backdrop_pass_test_support.cpp");
    require(!factory_bypass(diagnostic_scene) && has(diagnostic_scene, "blur.offscreen_pass_count == expected_passes") &&
                has(diagnostic_scene, "blur.blurred_backdrop_count == effects") &&
                has(diagnostic_scene, "snapshot->submitted_draws.backdrop_blur_count == effects") &&
                has(diagnostic_scene, "blur.offscreen_targets_allocated == (effects != 0)") &&
                has(diagnostic_scene, "completed_pixel_capture(renderer)"),
            "backend-specific diagnostics retain exact topology/count/target assertions and real capture");
    for (const auto* name : {"vulkan", "dx12"}) {
      const auto filename = std::string(name) + "_backdrop_pass_diagnostics_test.cpp";
      const auto source = pixel(filename.c_str());
      require(has(windows_tests, filename) && has(source, std::string("create_") + name + "_renderer(") &&
                  has(source, "validate_pixel_backend_info(") && has(source, "effects == 1 ? 2 : 1") &&
                  has(source, std::string_view(name) == "vulkan" ? "{0, 5, 9}" : "{0, 3, 6}"),
              "focused named-backend wrappers preserve exact Vulkan 0/5/9 and DX12 0/3/6 separately");
    }
    const auto windows_build = section(manifest, "[target.'cfg(os = \"windows\")'.build]");
    const auto neutral_build = section(manifest, "[build]");
    for (const auto* name : {"renderer_pixel_test_backend.cpp", "renderer_pixel_test_backend_policy.cpp",
         "renderer_pixel_test_window.cpp", "renderer_pixel_test_capture.cpp", "renderer_backdrop_pass_test_support.cpp"})
      require(has(windows_build, name) && !has(neutral_build, name), "native support sources must stay Windows-only");
    for (const auto* name : {"renderer_box_shadow_pixel_support.cpp", "renderer_text_pixel_shape_support.cpp",
                            "renderer_pixel_test_probes.cpp"})
      require(has(neutral_build, name) && !has(pixel(name), "<windows.h>"), "math/probe support remains portable cpp code");
    require(has(manifest, "[targets.cgpui_tests_renderer_support]") &&
                has(section(manifest, "[target.'cfg(os = \"windows\")'.dependencies]"), "cgpui.renderer-dx12") &&
                !has(section(manifest, "[dependencies]"), "cgpui.renderer-dx12") &&
                has(section(manifest, "[target.'cfg(os = \"linux\")'.test]"), "wayland_frame_pixel_capture_test.cpp") &&
                has(section(manifest, "[target.'cfg(os = \"macos\")'.test]"), "metal/metal_*.mm"),
            "test-only support library and DX12 dependency must preserve Linux/Metal registration boundaries");
    require(has(pixel("metal/metal_box_shadow_pixel_test.mm"), "renderer_box_shadow_pixel_support.hpp") &&
                has(pixel("renderer_text_ink_extent_pixel_test.cpp"), "cgpui::build_textured_glyph_quads("),
            "Metal math reference and common ink-extent scene use the neutral owners");
    require(has(read("tests/architecture/mcpp.toml"), "renderer_common_pixel_structure_test.cpp") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "renderer_common_pixel_structure_test|tests/architecture/renderer_common_pixel_structure_test.cpp"),
            "common parity ownership guard must be registered in mcpp and CI");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
