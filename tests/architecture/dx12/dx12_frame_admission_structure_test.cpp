#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream input(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!input) throw std::runtime_error(std::string("missing frame admission owner: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& value, const char* token) { return value.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void once(const std::string& value, const char* token) {
  const auto first = value.find(token);
  require(first != std::string::npos && value.find(token, first + 1) == std::string::npos,
          "frame admission must retain a single observation/consumption/value-transfer hook");
}
void limit(const char* path, std::size_t maximum) {
  const auto value = read(path);
  require(std::count(value.begin(), value.end(), '\n') <= maximum, "frame admission exceeded its focused cap");
}
} // namespace

int main() {
  try {
    const auto renderer = read("src/renderer/dx12/dx12_renderer.cpp");
    require(has(renderer,
        "dx12_prepare_frame(*state_); !ready) return std::unexpected(ready.error());\n"
        "    auto admitted = dx12_admit_frame(*state_);\n"
        "    if (!admitted) return std::unexpected(admitted.error());\n"
        "    auto frame = std::make_unique<Dx12Frame>(state_, admitted->admission_call_ns);\n"
        "    state_->frame_active = true;"),
        "alpha/overlap/recovery precede one admission and frame-owned timing construction");
    require(!has(renderer, "try_admit()") && !has(renderer, "ErrorCode::frame_not_ready") &&
                !has(renderer, "steady_clock"), "renderer entry must delegate admission and measurement");
    const auto header = read("src/renderer/dx12/dx12_frame_admission_internal.hpp");
    require(has(header, "Result<Dx12FrameAdmissionTiming> dx12_admit_frame(Dx12State& state);") &&
                has(header, "std::uint64_t admission_call_ns = 0;") &&
                !has(header, "try_admit()") && !has(header, "return "),
            "private admission header declares a value result, never native implementation");
    const auto admission = read("src/renderer/dx12/dx12_frame_admission.cpp");
    once(admission, "state.frame_pacing->try_admit()");
    require(has(admission,
        "const auto started = std::chrono::steady_clock::now();\n"
        "  auto admission = state.frame_pacing->try_admit();\n"
        "  const auto stopped = std::chrono::steady_clock::now();\n"
        "  if (!admission) {") &&
        has(admission, "dx12_handle_frame_failure(state, std::move(admission.error()))") &&
        has(admission, "return std::unexpected(std::move(failure.error()));") &&
        has(admission, "if (*admission == Dx12FrameAdmission::pending)\n"
            "    return std::unexpected(Error{.code = ErrorCode::frame_not_ready,") &&
        has(admission, "if (*admission == Dx12FrameAdmission::disabled) return Dx12FrameAdmissionTiming{};") &&
        has(admission, "std::chrono::nanoseconds>(stopped - started)") &&
        has(admission, "elapsed > 0 ? static_cast<std::uint64_t>(elapsed) : 0"),
        "only one successful/cached call is measured; pending/errors return, disabled yields zero, recovery excluded");
    for (const auto* token : {"state.failed", "state.pixels", "state.diagnostics", "state.info",
                             "state.frame_active", "state.submitted", "state.pending_fence_value",
                             "presentation_attempted", "Sleep(", "DispatchMessage", "MsgWait", "INFINITE",
                             "while (", "for (", "GetCurrentThreadId", "GetWindowThreadProcessId"})
      require(!has(admission, token), "admission cannot mutate published/GPU state, consume grants, block or pump");
    const auto frame_header = read("src/renderer/dx12/dx12_frame_internal.hpp");
    const auto frame = read("src/renderer/dx12/dx12_frame.cpp");
    require(has(frame_header, "const std::uint64_t admission_call_ns_;") &&
                has(frame_header, "std::uint64_t admission_call_ns = 0") &&
                has(frame, "admission_call_ns_(admission_call_ns)") &&
                has(frame, "command_count_, admission_call_ns_);") &&
                !has(frame, "steady_clock") && !has(read("src/renderer/dx12/dx12_state_internal.hpp"), "admission_call") &&
                !has(read("src/renderer/dx12/dx12_frame_pacing_internal.hpp"), "admission_call"),
            "the immutable sample travels by frame value, not a clock spanning the frame or shared grant/state");
    const auto present = read("src/renderer/dx12/dx12_presentation.cpp");
    once(present, "Dx12FrameDiagnostics diagnostics(state, draws, command_count, admission_call_ns)");
    once(present, "diagnostics.finish_stage(RendererFrameTimingStage::pacing_wait)");
    once(present, "state.frame_pacing->presentation_attempted()");
    require(has(present, "const HRESULT presented = state.swapchain->Present(1, 0);\n"
                         "  state.frame_pacing->presentation_attempted();\n") &&
                !has(frame, "presentation_attempted") && !has(present, "ErrorCode::frame_not_ready"),
            "actual Present always consumes; abandonment/pre-Present errors do not, and Present cannot emit pending");
    const auto diagnostics = read("src/renderer/dx12/dx12_frame_diagnostics.cpp");
    once(diagnostics, "renderer_add_frame_timing(snapshot_.timings, RendererFrameTimingStage::pacing_wait, admission_call_ns)");
    require(!has(diagnostics, "timings.total_nanoseconds =") &&
                has(diagnostics, "finish_stage(RendererFrameTimingStage::resource_preparation)"),
            "seed uses shared saturation without replacing the original presentation-stage clock");
    for (const auto* path : {"include/cgpui/renderer/renderer.hpp", "include/cgpui/renderer/renderer_frame.hpp"})
      require(!has(read(path), "dx12_admit_frame"), "native admission policy cannot leak into public APIs");
    limit("src/renderer/dx12/dx12_renderer.cpp", 100);
    limit("src/renderer/dx12/dx12_frame_admission_internal.hpp", 25);
    limit("src/renderer/dx12/dx12_frame_admission.cpp", 40);
    require(has(read("packages/renderer-dx12/mcpp.toml"), "../../src/renderer/dx12/*.cpp") &&
                has(read("tests/renderer-dx12/mcpp.toml"), "\"dx12_diagnostic_timing_test.cpp\"") &&
                has(read("tests/architecture/mcpp.toml"), "\"dx12/*_structure_test.cpp\"") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "dx12_frame_admission_structure_test|tests/architecture/dx12/dx12_frame_admission_structure_test.cpp"),
            "focused source and moved structure guard must retain owning mcpp/CI registration");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
