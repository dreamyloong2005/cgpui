#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error(std::string("missing capture owner: ") + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* token) { return source.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
void limit(const char* path, unsigned cap) {
  const auto source = read(path);
  require(std::count(source.begin(), source.end(), '\n') <= cap, "capture publication exceeded focused cap");
}
}
int main() {
  try {
    const auto capture = read("src/renderer/dx12/dx12_capture.cpp");
    require(has(capture, "Result<RendererFramePixels> dx12_read_capture(") &&
        has(capture, "return pixels;") && !has(capture, "state.pixels"),
        "readback extraction must return an owned unpublished value");
    require(capture.find("pixels.rgba8.resize") < capture.find("state.readback->Map") &&
        capture.find("state.readback->Unmap") < capture.find("return pixels;"),
        "allocate before Map and complete Unmap before returning the candidate");
    require(has(read("src/renderer/dx12/dx12_capture_internal.hpp"),
        "Result<RendererFramePixels> dx12_read_capture(Dx12State& state);") &&
        has(read("src/renderer/dx12/dx12_state_operations_internal.hpp"),
            "#include \"dx12_capture_internal.hpp\"") &&
        !has(read("src/renderer/dx12/dx12_state_internal.hpp"), "dx12_read_capture("),
        "private capture leaf owns the value boundary; state aggregates only include it");
    const auto submit = read("src/renderer/dx12/dx12_presentation.cpp");
    const auto publish = submit.find("if (candidate_pixels) state.pixels = std::move(candidate_pixels);");
    const auto completion = submit.find("diagnostics.complete_successful_frame();");
    require(has(submit, "std::optional<RendererFramePixels> candidate_pixels;") &&
        has(submit, "is_nothrow_move_assignable_v<std::optional<RendererFramePixels>>") &&
        publish != std::string::npos && publish > submit.find("dx12_check_debug_errors(state)") &&
        completion != std::string::npos &&
        completion > submit.find("dx12_commit_composition(state)") && publish > completion &&
        !has(submit, "diagnostics.publish();"),
        "only a requested successful candidate publishes, after all native checks and diagnostic completion");
    const auto frame = read("src/renderer/dx12/dx12_frame.cpp");
    require(!has(frame, "state_->pixels.reset()") &&
        has(read("src/renderer/dx12/dx12_renderer.cpp"), "!state_->recovery_pending && state_->pixels"),
        "Present preserves successful history while pending recovery retains its getter boundary");
    const auto images = frame.find("images_.prepare(*state_, draws_)");
    const auto image_prepare = read("src/renderer/dx12/dx12_image_prepare.cpp");
    require(images != std::string::npos && images > frame.find("dx12_prepare_present(*state_, generation_)") &&
        images < frame.find("dx12_submit_frame(") &&
        has(frame, "if (auto prepared = images_.prepare(*state_, draws_); !prepared) return prepared;") &&
        !has(image_prepare, "state.pixels") && !has(image_prepare, "state_->pixels"),
        "deferred image preparation preserves capture history and returns CPU input failures before submission");
    require(has(read("src/renderer/dx12/dx12_swapchain_resize.cpp"), "state.pixels.reset();") &&
        has(read("src/renderer/dx12/dx12_recovery.cpp"), "state.pixels.reset();"),
        "stable-surface history must not remove explicit resize/recovery invalidation");
    const auto probe = read("tests/renderer-dx12/dx12_composition_probe_frame_test_support.cpp");
    require(has(probe, "auto pixels = dx12_read_capture(*state);") &&
        probe.find("state->pixels = std::move(*pixels);") > probe.find("Commit_after_first_Present") &&
        probe.find("state->pixels = std::move(*pixels);") > probe.find("if (!debug_clean())"),
        "native capability probe must also keep readback owned through Commit/debug checks");
    const auto test = read("tests/renderer-dx12/dx12_capture_publication_test.cpp");
    require(has(test, "AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, marker)") &&
        has(test, "only_injected_error(*messages.Get())") && has(test, "injected == 1") &&
        has(test, "for (const bool warm : {false, true})") &&
        has(test, "saved_pixels = *(*renderer)->last_frame_pixels();") &&
        has(test, "warm ? !same_capture((*renderer)->last_frame_pixels(), *saved_pixels)") &&
        has(test, "last_frame_pixels() != nullptr") && has(test, "diagnostic_same("),
        "native late-error regression must deep-compare warm history, require cold null, and inspect every native error");
    const auto common = read("tests/renderer/renderer_capture_history_test.cpp");
    require(has(common, "create_pixel_test_renderer(") && has(common, "actual->rgba8 == expected.rgba8") &&
        has(common, "actual->encoding == expected.encoding") && has(common, "const auto red =") &&
        has(common, "const auto blue =") && has(common, "reject_capture(") &&
        !has(common, "RendererBackend::dx12") && !has(common, "create_dx12_renderer("),
        "common capture history has one exact oracle, real selected renderer and owned before/after captures");
    require(has(read("tests/renderer/mcpp.toml"), "\"renderer_capture_history_test.cpp\"") &&
        has(read("include/cgpui/renderer/renderer_frame.hpp"), "unchanged valid surface/device"),
        "register the common scene and publish its bounded capture-capable contract");
    require(has(read("tests/renderer-dx12/mcpp.toml"), "\"dx12_capture_publication_test.cpp\"") &&
        has(read("scripts/ci/architecture-header-targets.txt"),
        "dx12_capture_publication_structure_test|tests/architecture/dx12/dx12_capture_publication_structure_test.cpp"),
        "register both native and structure regressions");
    limit("src/renderer/dx12/dx12_capture.cpp", 60);
    limit("src/renderer/dx12/dx12_capture_internal.hpp", 20);
    limit("src/renderer/dx12/dx12_presentation.cpp", 110);
    limit("src/renderer/dx12/dx12_renderer.cpp", 100);
    limit("tests/renderer-dx12/dx12_capture_publication_test.cpp", 140);
    limit("tests/renderer/renderer_capture_history_test.cpp", 120);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
