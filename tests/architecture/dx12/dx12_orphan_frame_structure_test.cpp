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
  if (!input) throw std::runtime_error(std::string("missing orphan owner: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& source, const char* text) { return source.find(text) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void limit(const char* path, unsigned maximum) {
  const auto source = read(path);
  require(std::count(source.begin(), source.end(), '\n') <= maximum, "orphan leaf exceeded its focused cap");
}
}
int main() {
  try {
    const auto orphan = read("src/renderer/dx12/dx12_orphan_frame.cpp");
    const auto frame = read("src/renderer/dx12/dx12_frame.cpp");
    const auto routing = read("src/renderer/dx12/dx12_frame_recovery.cpp");
    const auto recovery = read("src/renderer/dx12/dx12_recovery.cpp");
    require(has(orphan, "dx12_prepare_orphan_present") && has(orphan, "dx12_fail_orphan_frame") &&
        has(orphan, "IsWindow(state.hwnd)") && has(orphan, "dx12_validate_window_alpha(state)") &&
        has(orphan, "generation != state.info.device_generation") &&
        has(orphan, "state.device->GetDeviceRemovedReason()") &&
        has(orphan, "state.operation_depth <= 1") && !has(orphan, "dx12_create_") &&
        !has(orphan, "dx12_recover_removed_device("),
        "orphan policy validates native epoch/surface and can retire but never reconstruct");
    require(has(routing, "if (!state.owner_alive) return dx12_prepare_orphan_present") &&
        has(routing, "if (!state.owner_alive) return dx12_fail_orphan_frame") &&
        recovery.find("if (!state.owner_alive)") < recovery.find("dx12_release_native_objects(state);"),
        "preflight and late failure must both deny ownerless reconstruction");
    require(!has(frame, "!state_->owner_alive") &&
        has(frame, "if (released_)") && has(frame, "dx12_prepare_present(*state_, generation_)") &&
        has(read("src/renderer/dx12/dx12_renderer.cpp"), "state_->owner_alive = false;") &&
        has(read("src/renderer/dx12/dx12_frame_internal.hpp"), "std::shared_ptr<Dx12State> state_;"),
        "facade marks reconstruction capability while existing frame retains state and one-shot routing");
    const auto state = read("src/renderer/dx12/dx12_state.cpp");
    require(state.find("dx12_retire_submitted_work(*this)") < state.find("dx12_release_native_objects(*this)") &&
        state.find("dx12_release_native_objects(*this)") < state.find("window_alpha.reset()"),
        "alpha lease outlives native retirement and is not revoked with facade deletion");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    require(has(manifest, "\"dx12_orphan_present_test.cpp\"") && has(manifest, "\"dx12_orphan_removed_test.cpp\"") &&
        has(manifest, "\"dx12_orphan_late_error_test.cpp\"") &&
        has(manifest, "\"dx12_orphan_cold_popup_test.cpp\"") &&
        has(manifest, "\"dx12_orphan_late_removal_test.cpp\"") &&
        has(read("scripts/ci/architecture-header-targets.txt"),
            "dx12_orphan_frame_structure_test|tests/architecture/dx12/dx12_orphan_frame_structure_test.cpp"),
        "register orphan native and structure regressions");
    const auto factory = read("src/renderer/dx12/dx12_renderer.cpp");
    const auto native = read("tests/renderer-dx12/dx12_orphan_native_removal_test_support.cpp");
    require(has(factory, "std::make_shared<Dx12State>(&pacing_api, synchronization_api)") &&
        !has(factory, "state->synchronization_api =") && has(state, "synchronization_api(synchronization)") &&
        has(factory, "std::make_unique<Dx12Renderer>(std::move(state))") &&
        native.find("native_.signal(queue, fence, value)") < native.find("device->RemoveDevice()") &&
        has(native, "return result;") && has(native, "native_.completed(fence)"),
        "late removal uses the actual renderer and native synchronization results, not a fake frame/result");
    const auto late = read("tests/renderer-dx12/dx12_orphan_late_removal_test.cpp");
    require(has(late, "removal.completed_result != std::numeric_limits<UINT64>::max()") &&
        has(late, "removal.removal_calls != 1") && has(late, "!removal.native_same_device") &&
        has(late, "!removal.post_remove_checks") &&
        late.find("(*peer)->begin_frame()") < late.find("orphan->reset();") &&
        !has(native, "ClearStoredMessages"),
        "late removal must prove the native sentinel/failure branch and retirement before orphan teardown");
    const auto cold = read("tests/renderer-dx12/dx12_orphan_cold_popup_test.cpp");
    require(cold.find("renderer.reset();") < cold.find("(*orphan)->present()") &&
        has(cold, "windows.expect({128, 127, 0}") && !has(cold, "->Commit("),
        "cold orphan validates actual desktop pixels with production first Commit, not manual warming");
    require(has(read("tests/renderer/mcpp.toml"), "\"renderer_orphan_frame_contract_test.cpp\"") &&
        has(manifest, "\"dx12_orphan_native_removal_test_support.cpp\""),
        "register shared exact lifetime contract and focused native-removal support");
    limit("tests/renderer-dx12/dx12_orphan_native_removal_test_support.hpp", 35);
    limit("tests/renderer-dx12/dx12_orphan_native_removal_test_support.cpp", 65);
    limit("tests/renderer-dx12/dx12_orphan_cold_popup_test.cpp", 100);
    limit("tests/renderer-dx12/dx12_orphan_late_removal_test.cpp", 100);
    limit("src/renderer/dx12/dx12_orphan_frame_internal.hpp", 25);
    limit("src/renderer/dx12/dx12_orphan_frame.cpp", 80);
    limit("src/renderer/dx12/dx12_renderer.cpp", 100);
    limit("tests/renderer-dx12/dx12_orphan_present_test.cpp", 140);
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
