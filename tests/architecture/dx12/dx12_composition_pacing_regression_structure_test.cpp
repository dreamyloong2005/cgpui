#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream input(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!input) throw std::runtime_error("missing composition regression leaf: " + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
std::string leaf(const char* name) { return read(std::string("tests/renderer-dx12/") + name); }
void limit(const char* name, unsigned cap) {
  const auto text = leaf(name);
  require(std::count(text.begin(), text.end(), '\n') <= cap, "composition regression exceeded focused cap");
}
}
int main() {
  try {
    const auto header = leaf("dx12_composition_pacing_test_support.hpp");
    for (const auto* token : {"return ", "begin_frame(", "present(", "CreateWindow", "ShowWindow"})
      require(!has(header, token), "shared helper header must remain declaration-only");
    const auto resized = leaf("dx12_composition_pacing_resize_test.cpp");
    const auto recovery = leaf("dx12_composition_pacing_recovery_test.cpp");
    const auto orphan = leaf("dx12_composition_pacing_orphan_lifetime_test.cpp");
    for (const auto* source : {&resized, &recovery, &orphan}) {
      require(has(*source, "Dx12CompositionFirstBeginWindows windows;") &&
          has(*source, "Dx12PacingForwardApi api;") &&
          has(*source, "dx12_create_renderer(windows.descriptor(), api)") &&
          has(*source, "pacing_begin_bounded(*renderer, api)"),
          "composition regressions must observe the real renderer through strictly native waits");
      for (const auto* token : {"Dx12PacingControlApi", "WAIT_OBJECT_0", ".waited =", "ShowWindow("})
        require(!has(*source, token), "regression cannot synthesize readiness or show hidden fixture");
    }
    require(has(resized, "renderer->resize({96, 96}, {1})") &&
        has(resized, "api.polls != polls || api.ready != 1") &&
        has(resized, "api.polls <= after_present_polls") &&
        has(resized, "composition_pacing_capture(**first, *renderer, false, 96)"),
        "actual resize/cached-grant/new-admission requires changed capture dimensions");
    require(has(recovery, "dx12_remove_native_device(before)") &&
        has(recovery, "const auto retry = renderer->begin_frame();") &&
        has(recovery, "retry.error().code != ErrorCode::frame_retry_required") &&
        has(recovery, "api.closes != 1") && has(recovery, "api.sets != 2 || api.gets != 2") &&
        has(recovery, "after.device_generation != before.device_generation + 1") &&
        has(recovery, "after.adapter_luid != before.adapter_luid") &&
        has(recovery, "api.ready != old_ready + 1"),
        "real removal must expose one raw typed retry, close old owner and freshly admit replacement");
    require(has(orphan, "for (const bool finished_first : {false, true})") &&
        has(orphan, "renderer.reset();") && has(orphan, "const auto orphan_present = (*orphan)->present();") &&
        has(orphan, "if (!orphan_capture || !orphan_present ||") &&
        has(orphan, "const auto repeated_orphan = (*finished)->present();") &&
        has(orphan, "const auto overlap = renderer->begin_frame();") &&
        has(orphan, "api.closes != 1 || !api.close_ok") && has(orphan, "api.polls != polls"),
        "both last-frame orders, healthy orphan success and raw finished/overlap rejection preserve native lifetime");
    const auto window = leaf("dx12_composition_pacing_window_test_support.cpp");
    require(has(window, "SetWindowPos(") && has(window, "SWP_NOACTIVATE") &&
        has(window, "!IsWindowVisible(popup)") && !has(window, "ShowWindow("),
        "real native extent helper cannot show then hide the initial-hidden fixture");
    const auto pixels = leaf("dx12_composition_pacing_pixels_test_support.cpp");
    require(has(pixels, "frame.present()") && has(pixels, "pixels->width == extent") &&
        has(pixels, "dx12_pixel(pixels, 30, 32, {0, 0, 0, 0}, 2)"),
        "capture must inspect actual premultiplied transparent pixels at the requested extent");
    const auto cold = leaf("dx12_composition_pacing_cold_resize_test.cpp");
    require(cold.find("renderer->resize({96, 96}, {1})") < cold.find("pacing_begin_bounded(*renderer, api)") &&
        has(cold, "api.polls || api.ready || api.sets != 1") &&
        has(cold, "renderer->resize({64, 64}, {1})") && has(cold, "api.polls <= polls || api.ready != 2"),
        "cold and post-Present resizes must check real extent and strict native admission");
    const auto lifetime = leaf("dx12_popup_lifetime_test.cpp");
    const auto cohort = leaf("dx12_popup_recovery_test.cpp");
    require(has(lifetime, "duplicate_api.closes != 1") && has(lifetime, "api.polls != polls") &&
        has(cohort, "api.closes != epoch + 1") && has(cohort, "api.sets != epoch + 2") &&
        has(cohort, "api.ready != ready + 1") && has(cohort, "borrowed->rgba8 != saved"),
        "failed construction and real composition cohort need exact handle epochs without losing pixel/lifetime assertions");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_composition_pacing_pixels_test_support.cpp",
                             "dx12_composition_pacing_window_test_support.cpp"}) {
      require(has(manifest, name), "register focused composition helper");
      limit(name, 80);
    }
    for (const auto* name : {"dx12_composition_pacing_resize_test.cpp",
                             "dx12_composition_pacing_cold_resize_test.cpp",
                             "dx12_composition_pacing_recovery_test.cpp",
                             "dx12_composition_pacing_orphan_lifetime_test.cpp"}) {
      const auto test = manifest.find("[target.'cfg(os = \"windows\")'.test]");
      require(test != std::string::npos && manifest.find(name, test) != std::string::npos,
          "register actual composition lifecycle regressions in Windows discovery");
      limit(name, 140);
    }
    limit("dx12_composition_pacing_test_support.hpp", 30);
    require(has(read("scripts/ci/architecture-header-targets.txt"),
        "dx12_composition_pacing_regression_structure_test|tests/architecture/dx12/dx12_composition_pacing_regression_structure_test.cpp"),
        "register exact lifecycle regression guard inventory");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
