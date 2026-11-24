#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const std::filesystem::path& path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error("missing DRED native control: " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
bool has(std::string_view text, std::string_view token) { return text.find(token) != text.npos; }
}
int main() {
  try {
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_dred_native_identity_test_support.cpp", "dx12_dred_native_readout_test_support.cpp", "dx12_dred_native_loss_test_support.cpp",
        "dx12_dred_native_cycles_test_support.cpp", "dx12_dred_native_debug_test.cpp",
        "dx12_dred_native_nodebug_test.cpp", "dx12_dred_native_partial_test.cpp",
        "dx12_dred_native_live_failure_test.cpp", "dx12_dred_native_final_frame_test.cpp"}) {
      const auto text = read(std::filesystem::path("tests/renderer-dx12") / name);
      require(std::count(text.begin(), text.end(), '\n') <= 100, "native controls require focused leaves");
      require(has(manifest, std::string("\"") + name + "\""), "native control not registered");
      for (const auto* token : {"GetAutoBreadcrumbsOutput", "GetPageFaultAllocationOutput",
          "dx12_observe_dred_status(", "state.owner_alive =", "state->owner_alive ="})
        require(!has(text, token), "native oracle must not request output/directly invoke diagnostic hook/fake owner death");
    }
    const auto identity = read("tests/renderer-dx12/dx12_dred_native_identity_test_support.cpp");
    require(has(identity, "device->GetAdapterLuid()") && has(identity, "native_luid != *info.adapter_luid") &&
        has(identity, "actual_software=") && has(identity, "incoming_gpu_request=") &&
        has(identity, "configured_gpu_request=") && has(identity, "actual_infoqueue_received=") &&
        has(identity, "gpu_validation_native_state=not_queried") && has(identity, "root_mapping_oracle_required=1") &&
        has(identity, "std::cout.flush();") && has(identity, "return std::cout.good();") &&
        has(identity, "(enabled || _putenv_s(\"CGPUI_DX12_GPU_VALIDATION\", \"0\") == 0)"),
        "actual adapter/config output must be flush-checked; debug-on preserves GPU request, never claims a native getter");
    for (const auto* file : {"dx12_dred_native_cycles_test_support.cpp", "dx12_dred_native_partial_test.cpp",
        "dx12_dred_native_live_failure_test.cpp", "dx12_dred_native_final_frame_test.cpp",
        "dx12_coordinated_recovery_test.cpp", "dx12_orphan_removed_test.cpp"}) {
      const auto source = read(std::filesystem::path("tests/renderer-dx12") / file);
      require(has(source, "dred_record_identity(") && has(source, "dred_flush_output()"),
          "every new native executable/shared case and both reused cases require identity+output gates");
    }
    const auto cycles = read("tests/renderer-dx12/dx12_dred_native_cycles_test_support.cpp");
    require(has(cycles, "create_dx12_renderer(window.descriptor())") && has(cycles, "cycle != 2") &&
        has(cycles, "const auto retry = renderer->begin_frame();") && has(cycles, "ErrorCode::frame_retry_required") &&
        has(cycles, "readout.calls != cycle + 1") && has(cycles, "dred_current_readout(readout, loss") &&
        has(cycles, "diagnostic_same(") && has(cycles, "view.last_is_current") && has(cycles, "dx12_pixel("),
        "both native cycles need public retry/pixels/history and current-vs-prior scalar assertions");
    const auto readout = read("tests/renderer-dx12/dx12_dred_native_readout_test_support.cpp");
    require(has(readout, "reuse_public_state(info, hwnd)") && has(readout, "void retired(bool)") &&
        has(readout, "state_.dred_status.snapshot_locked()") && has(readout, "return nullptr;") &&
        !has(readout, "std::shared_ptr") && !has(readout, "RemoveDevice"),
        "test readout reuses existing weak lookup and retirement seam without retaining state/native work");
    const auto loss = read("tests/renderer-dx12/dx12_dred_native_loss_test_support.cpp");
    require(has(loss, "device->RemoveDevice();") && has(loss, "observed.reason = device->GetDeviceRemovedReason();") &&
        has(loss, "device.As(&dred)") && has(loss, "dx12_recovery_debug_messages_safe(messages.Get(), true)") &&
        has(loss, "reference_qi=not_attempted(debug_off)"), "stimulus/reference status must be actual and explicit");
    const auto partial = read("tests/renderer-dx12/dx12_dred_native_partial_test.cpp");
    require(has(partial, "dx12_open_adapter_device(state)") && has(partial, "state.queue || state.commands") &&
        has(partial, "state.info.device_generation != generation") &&
        has(partial, "!historical.last_loss") && has(partial, "dred_current_readout(readout, second_loss"),
        "partial direct-reset and second common cleanup require real childless unchanged-generation observations");
    const auto live = read("tests/renderer-dx12/dx12_dred_native_live_failure_test.cpp");
    require(has(live, "dx12_poll_submission(state, script)") && has(live, "state.device->GetDeviceRemovedReason() != S_OK") &&
        has(live, "script.signal_calls") && has(live, "readout.snapshot.last_loss"),
        "injected synchronization error must be distinguished from a live native device without submission");
    const auto final_frame = read("tests/renderer-dx12/dx12_dred_native_final_frame_test.cpp");
    require(has(final_frame, "renderer.reset();") && has(final_frame, "weak.use_count() != 1") &&
        has(final_frame, "frame->reset();") && has(final_frame, "!weak.expired()") &&
        has(final_frame, "dred_current_readout(readout, loss") && has(final_frame, "auto fresh = create_dx12_renderer("),
        "final-frame control must drop the actual sole owner and read only copied scalars afterward");
    require(has(read("tests/renderer-dx12/dx12_coordinated_recovery_test.cpp"), "dred_current_readout(second_scalar, loss") &&
        has(read("tests/renderer-dx12/dx12_orphan_removed_test.cpp"), "dred_current_readout(scalar, loss"),
        "reuse existing peer/orphan behavior with scalar assertions instead of silent noncoverage");
    require(has(read("scripts/ci/architecture-header-targets.txt"),
        "dx12_dred_native_controls_structure_test|tests/architecture/dx12/dx12_dred_native_controls_structure_test.cpp"),
        "native-controls structure entry must be additive and explicitly registered");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
