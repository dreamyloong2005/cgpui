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
  if (!stream) throw std::runtime_error(std::string("missing fallback owner: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
void require(bool value, const char* reason) {
  if (!value) throw std::runtime_error(reason);
}
bool has(const std::string& text, const char* token) {
  return text.find(token) != std::string::npos;
}
void limit(const char* path, std::size_t lines) {
  const auto source = read(path);
  require(std::count(source.begin(), source.end(), '\n') <= lines,
          "backdrop fallback modules must remain focused within fixed budgets");
}
} // namespace

int main() {
  try {
    const auto frame = read("src/renderer/dx12/dx12_backdrop_blur_frame.cpp");
    const auto validate = frame.find("dx12_valid_backdrop_blur_input(device)");
    const auto support = frame.find("if (!state_->backdrop_blur->support.supported)");
    const auto fallback = frame.find("RenderFrame::draw_backdrop_blur(backdrop);");
    const auto plan = frame.find("plan_backdrop_blur(");
    require(validate != std::string::npos && support != std::string::npos &&
                fallback != std::string::npos && plan != std::string::npos &&
                validate < support && support < fallback && fallback < plan,
            "validate raw device fields before capability fallback, then retain supported planning");
    require(!has(frame, "RenderFrame::draw_backdrop_blur(device)") &&
                !has(frame, "ErrorCode::unsupported_platform") &&
                has(frame, "dx12_valid_backdrop_blur_plan(plan, extent)"),
            "reuse base fallback with logical input; do not double-scale or reject capability absence");
    const auto base = read("src/renderer/renderer_effect_fallback.cpp");
    require(has(base, "if (!backdrop.tint.has_value())") && has(base, "draw_rounded_rect(RoundedRectDraw{") &&
                has(base, ".radius = backdrop.radius") && has(base, ".clip_rect = backdrop.clip_rect") &&
                has(base, ".clip_stack = backdrop.clip_stack") &&
                has(base, ".composition_stack = backdrop.composition_stack") &&
                has(base, ".metadata = backdrop.metadata"),
            "the shared fallback remains the sole field-forwarding owner");
    require(!has(read("src/renderer/dx12/dx12_frame.cpp"), "Dx12Frame::draw_backdrop_blur("),
            "the general frame entry must not absorb effect fallback implementation");
    limit("src/renderer/dx12/dx12_backdrop_blur_frame.cpp", 100);
    limit("src/renderer/renderer_effect_fallback.cpp", 40);
    limit("tests/renderer-dx12/dx12_backdrop_fallback_test_support.hpp", 40);
    limit("tests/renderer-dx12/dx12_backdrop_fallback_test_support.cpp", 100);
    limit("tests/renderer-dx12/dx12_backdrop_fallback_diagnostics_test_support.cpp", 60);
    const auto declarations = read("tests/renderer-dx12/dx12_backdrop_fallback_test_support.hpp");
    require(!has(declarations, "dx12_create_device(") && !has(declarations, "frame.present()"),
            "native fixture behavior belongs in its focused implementation, not a broad header");
    const auto module = read("tests/renderer-dx12/mcpp.toml");
    const auto native = module.find("[target.'cfg(os = \"windows\")'.test]");
    require(native != std::string::npos, "native fallback tests remain Windows-only");
    for (const auto* name : {"dx12_backdrop_blur_fallback_pixels_test.cpp",
                            "dx12_backdrop_blur_fallback_composition_test.cpp",
                            "dx12_backdrop_blur_fallback_validation_test.cpp",
                            "dx12_backdrop_blur_fallback_lifetime_test.cpp",
                            "dx12_backdrop_blur_fallback_recovery_test.cpp"}) {
      require(module.find(name, native) != std::string::npos, "register every native fallback regression");
      const auto path = std::string("tests/renderer-dx12/") + name;
      limit(path.c_str(), 120);
    }
    require(has(module, "\"dx12_backdrop_fallback_test_support.cpp\"") &&
                has(module, "\"dx12_backdrop_fallback_diagnostics_test_support.cpp\""),
            "explicit native support member must compile both focused implementation files");
    const auto registry = read("tests/architecture/mcpp.toml");
    const auto ci = read("scripts/ci/architecture-header-targets.txt");
    require(has(registry, "\"dx12/*_structure_test.cpp\"") &&
                has(ci, "dx12_backdrop_fallback_structure_test|tests/architecture/dx12/dx12_backdrop_fallback_structure_test.cpp") &&
                has(registry, "\"renderer_common_pixel_structure_test.cpp\"") &&
                has(ci, "dx12_runtime_recovery_structure_test|tests/architecture/dx12/dx12_runtime_recovery_structure_test.cpp"),
            "register fallback ownership without dropping common/runtime guard registrations");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
