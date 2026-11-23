#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};

std::string read(const std::filesystem::path& path) {
  std::ifstream stream(root / path);
  if (!stream) throw std::runtime_error("missing runtime recovery owner: " + path.string());
  return {std::istreambuf_iterator<char>(stream), {}};
}

bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}

void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
} // namespace

int main() {
  try {
    const auto directory = std::filesystem::path("tests/renderer-dx12");
    const auto manifest = read(directory / "mcpp.toml");
    for (const auto* stem : {"dx12_runtime_capture_test_support",
                            "dx12_runtime_recovery_scene_test_support",
                            "dx12_runtime_recovery_loop_test_support"}) {
      for (const auto* extension : {".hpp", ".cpp"}) {
        const auto name = std::string(stem) + extension;
        const auto source = read(directory / name);
        const auto limit = std::string(extension) == ".cpp" ? 280 : 200;
        require(std::count(source.begin(), source.end(), '\n') <= limit,
                "native capture, scene, and loop helpers require focused leaves");
        require(!has(source, "ui_internal.hpp") && !has(source, "dx12_state_internal.hpp") &&
                    !has(source, "TestApp") && !has(source, "TestRenderer") &&
                    !has(source, "try_draw_frame(") && !has(source, "WindowRedrawRequested"),
                "native runtime recovery acceptance must use public runtime and real event-loop seams");
        if (std::string(extension) == ".hpp") {
          require(!has(source, "create_dx12_renderer(") && !has(source, "run_app(") &&
                      !has(source, "list.fill_") && !has(source, "->begin_frame("),
                  "helper headers declare ownership; non-template implementations belong in cpp leaves");
        } else {
          require(has(manifest, name.c_str()), "helper cpp must be linked by the test support library");
        }
      }
    }
    const auto capture = read(directory / "dx12_runtime_capture_test_support.cpp");
    require(has(capture, "cgpui::create_dx12_renderer(descriptor)") &&
                has(capture, "renderer_->begin_frame()") &&
                has(capture, "ErrorCode::frame_retry_required") &&
                has(capture, "(*frame)->request_pixel_capture()") &&
                has(capture, "return std::move(*frame)") && !has(capture, "public cgpui::RenderFrame") &&
                !has(capture, "present()") && !has(capture, "ID3D12Device"),
            "capture observer must return the same real frame and never fake presentation or retain a COM device");
    for (const auto* forwarding : {"renderer_->resize(", "renderer_->backend_info()",
                                   "renderer_->last_frame_pixels()",
                                   "renderer_->last_frame_diagnostic_snapshot()",
                                   "renderer_->surface_composition()",
                                   "renderer_->backdrop_blur_support()",
                                   "renderer_->popup_windows_supported()",
                                   "renderer_->set_transparent_surface("})
      require(has(capture, forwarding), "capture observer must preserve every real renderer report/operation");
    const auto scene_header = read(directory / "dx12_runtime_recovery_scene_test_support.hpp");
    const auto scene = read(directory / "dx12_runtime_recovery_scene_test_support.cpp");
    require(has(scene_header, "cgpui::ImageAsset image_") && has(scene, "list.upload_image(image_)") &&
                has(scene, "state_.upload_intents == 0") && has(scene, "list.fill_text(") &&
                has(scene, "context.scale.value") && has(scene, "glyph == glyph_reference"),
            "the once-uploaded scene must own image storage and compare actual scaled glyph capture bytes");
    const auto loop = read(directory / "dx12_runtime_recovery_loop_test_support.cpp");
    for (const auto* operation : {"cgpui::create_platform_application()", "cgpui::run_app(",
                                  "Window(runtime, ids[index]).request_render()",
                                  "try_open_window(", "set_after_frame_callback(",
                                  "dx12_remove_native_device(", "dx12_recovered_native_device_clean(",
                                  "GetWindowThreadProcessId(", "observed.baseline.device_generation + 1"})
      require(has(loop, operation), "native recovery loop must retain its real platform/loss/retry/capture oracle");
    require(has(read(directory / "dx12_runtime_root_recovery_test.cpp"), "dx12_runtime_recovery_case(false)") &&
                has(read(directory / "dx12_runtime_child_recovery_test.cpp"), "dx12_runtime_recovery_case(true)"),
            "root-first and child-first acceptance must remain separate native processes");
    require(has(manifest, "[targets.cgpui_tests_renderer_dx12_support]") &&
                has(manifest, "cgpui.ui = { path = \"../../packages/ui\" }") &&
                has(manifest, "dx12_runtime_root_recovery_test.cpp") &&
                has(manifest, "dx12_runtime_child_recovery_test.cpp") &&
                has(manifest, "../../tools/mcpp/empty_library.cpp") &&
                has(read("tests/architecture/mcpp.toml"), "dx12_runtime_recovery_structure_test.cpp") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "dx12_runtime_recovery_structure_test|tests/architecture/dx12_runtime_recovery_structure_test.cpp"),
            "runtime recovery behavior/support leaves and structure checks require explicit mcpp/CI registration");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
