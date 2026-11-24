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
    require(has(read("src/renderer/dx12/dx12_state_internal.hpp"),
        "Result<RendererFramePixels> dx12_read_capture(Dx12State& state);"),
        "capture value boundary stays private");
    const auto submit = read("src/renderer/dx12/dx12_presentation.cpp");
    const auto publish = submit.find("state.pixels = std::move(candidate_pixels);");
    require(has(submit, "std::optional<RendererFramePixels> candidate_pixels;") &&
        has(submit, "is_nothrow_move_assignable_v<std::optional<RendererFramePixels>>") &&
        publish != std::string::npos && publish > submit.find("dx12_check_debug_errors(state)") &&
        publish > submit.find("dx12_commit_composition(state)") && publish > submit.find("diagnostics.publish()"),
        "pixels publish only after all native checks and diagnostic preparation succeed");
    const auto frame = read("src/renderer/dx12/dx12_frame.cpp");
    require(frame.find("state_->pixels.reset()") < frame.find("dx12_submit_frame(") &&
        has(read("src/renderer/dx12/dx12_renderer.cpp"), "!state_->recovery_pending && state_->pixels"),
        "preserve old invalidation and getter rather than hiding failed candidate storage");
    const auto probe = read("tests/renderer-dx12/dx12_composition_probe_frame_test_support.cpp");
    require(has(probe, "auto pixels = dx12_read_capture(*state);") &&
        probe.find("state->pixels = std::move(*pixels);") > probe.find("Commit_after_first_Present") &&
        probe.find("state->pixels = std::move(*pixels);") > probe.find("if (!debug_clean())"),
        "native capability probe must also keep readback owned through Commit/debug checks");
    const auto test = read("tests/renderer-dx12/dx12_capture_publication_test.cpp");
    require(has(test, "AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, marker)") &&
        has(test, "only_injected_error(*messages.Get())") && has(test, "injected == 1") &&
        has(test, "for (const bool warm : {false, true})") &&
        has(test, "last_frame_pixels() != nullptr") && has(test, "diagnostic_same("),
        "cold/warm native late-error regression must preserve diagnostics and inspect every native error");
    require(has(read("tests/renderer-dx12/mcpp.toml"), "\"dx12_capture_publication_test.cpp\"") &&
        has(read("scripts/ci/architecture-header-targets.txt"),
        "dx12_capture_publication_structure_test|tests/architecture/dx12/dx12_capture_publication_structure_test.cpp"),
        "register both native and structure regressions");
    limit("src/renderer/dx12/dx12_capture.cpp", 60);
    limit("src/renderer/dx12/dx12_presentation.cpp", 110);
    limit("src/renderer/dx12/dx12_renderer.cpp", 100);
    limit("tests/renderer-dx12/dx12_capture_publication_test.cpp", 140);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
