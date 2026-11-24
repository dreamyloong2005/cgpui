#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* relative) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / relative);
  if (!file) throw std::runtime_error(std::string("missing cohort owner: ") + relative);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
} // namespace

int main() {
  try {
    for (const auto* path : {"src/renderer/dx12/dx12_device_cohort_internal.hpp",
                             "src/renderer/dx12/dx12_device_cohort.cpp",
                             "src/renderer/dx12/dx12_operation.cpp",
                             "src/renderer/dx12/dx12_operation_internal.hpp",
                             "src/renderer/dx12/dx12_cohort_retirement.cpp"}) {
      const auto text = read(path);
      require(std::count(text.begin(), text.end(), '\n') <= 150,
              "cohort metadata and native retirement must keep focused owners");
      require(!has(text, "RemoveDevice(") && !has(text, "getenv("),
              "coordinator cannot force removal or expose environment fault switches");
    }
    const auto cohort = read("src/renderer/dx12/dx12_device_cohort.cpp");
    const auto retirement = read("src/renderer/dx12/dx12_cohort_retirement.cpp");
    require(has(cohort, "weak_ptr") && has(retirement, "try_to_lock") &&
                has(cohort, "GetCurrentThreadId(") && has(retirement, "GetDeviceRemovedReason("),
            "cohort uses weak ownership and nonblocking owner-thread/native-removal eligibility");
    require(!has(cohort, "D3D12CreateDevice(") && !has(cohort, "CreateSwapChain") &&
                !has(cohort, "dx12_create_device_on_adapter(") &&
                !has(retirement, "dx12_create_device_on_adapter("),
            "cohort retirement cannot eagerly reconstruct peer windows");
    const auto state = read("src/renderer/dx12/dx12_state_internal.hpp");
    require(has(state, "std::recursive_mutex operation_mutex") && has(state, "window_thread_id") &&
                has(state, "operation_depth") && has(retirement, "peer->operation_depth != 0"),
            "operation gates and UI-thread eligibility must remain private state");
    const auto operation = read("src/renderer/dx12/dx12_operation.cpp");
    require(has(operation, "++state_.operation_depth") && has(operation, "--state_.operation_depth"),
            "same-thread recursive entry must remain active until its outer guard exits");
    require(has(retirement, "Dx12OperationGuard retiring(*peer)") &&
                has(read("src/renderer/dx12/dx12_recovery.cpp"), "state.operation_depth > 1"),
            "retirement and nested origin recovery must respect active outer operations");
    const auto renderer = read("src/renderer/dx12/dx12_renderer.cpp");
    require(has(read("src/renderer/dx12/dx12_initialization.cpp"), "dx12_register_device_cohort(state)") &&
                has(renderer, "!state_->recovery_pending && state_->pixels"),
            "successful renderers register privately and pending peers hide stale captures");
    const auto purge = read("src/renderer/dx12/dx12_native_retirement.cpp");
    require(!has(purge, "pixels.reset(") && !has(purge, "frame_active") &&
                !has(purge, "support.supported = false"),
            "peer native retirement preserves borrowed CPU capture, active frames and capability");
    const auto frame = read("src/renderer/dx12/dx12_frame.cpp");
    require(has(frame, "dx12_prepare_present(*state_, generation_)") &&
                frame.find("dx12_prepare_present(") < frame.find("dx12_submit_frame("),
            "old active frames must preflight before touching possibly retired native state");
    for (const auto* path : {"src/renderer/dx12/dx12_frame.cpp",
                             "src/renderer/dx12/dx12_image_frame.cpp",
                             "src/renderer/dx12/dx12_text_frame.cpp",
                             "src/renderer/dx12/dx12_shadow_frame.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_frame.cpp"}) {
      require(has(read(path), "Dx12OperationGuard operation(*state_)"),
              "frame modules must protect state access against peer retirement");
    }
    require(!has(read("include/cgpui/renderer/renderer_backend.hpp"), "cohort") &&
                !has(read("include/cgpui/renderer/renderer_reports.hpp"), "operation_mutex"),
            "cohort coordination must not leak native modes or locks into public contracts");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
