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
  std::ifstream source(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!source) throw std::runtime_error("missing DRED scalar owner: " + path.string());
  return {std::istreambuf_iterator<char>(source), {}};
}
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
bool has(std::string_view value, std::string_view token) { return value.find(token) != value.npos; }
void capped(const char* name, unsigned maximum) {
  const auto text = read(std::filesystem::path("src/renderer/dx12") / name);
  require(std::count(text.begin(), text.end(), '\n') <= maximum, "DRED scalar leaf exceeds focused cap");
}
std::size_t at(const std::string& source, const char* token) {
  const auto found = source.find(token);
  require(found != source.npos, token);
  return found;
}
}
int main() {
  try {
    const char* leaves[] = {"dx12_dred_status_internal.hpp", "dx12_dred_api_internal.hpp",
        "dx12_dred_site_internal.hpp", "dx12_dred_site.cpp", "dx12_dred_setup.cpp",
        "dx12_dred_native_status_internal.hpp", "dx12_dred_native_status.cpp",
        "dx12_dred_status_sink_internal.hpp", "dx12_dred_status_format.cpp", "dx12_dred_status_sink.cpp"};
    for (const auto* leaf : leaves) {
      const auto text = read(std::filesystem::path("src/renderer/dx12") / leaf);
      capped(leaf, 120);
      for (const auto* token : {"GetAutoBreadcrumbsOutput", "GetPageFaultAllocationOutput",
          "D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT", "D3D12_AUTO_BREADCRUMB_NODE", "pCommandHistory",
          "SetPrivateData", "GetPrivateData", "FreeLibrary", "GetModuleHandleEx", "native_epoch_",
          "adapter_luid", "device_generation", "std::vector", "std::string", "std::mutex"})
        require(!has(text, token), "scalar candidate crossed a borrowed-output/identity/storage boundary");
    }
    for (const auto* leaf : {"dx12_dred_status_internal.hpp", "dx12_dred_site_internal.hpp"}) {
      const auto text = read(std::filesystem::path("src/renderer/dx12") / leaf);
      for (const auto* token : {"windows.h", "d3d12.h", "Dx12Ptr", "IUnknown", "ID3D12",
          "std::unique_ptr", "std::shared_ptr", "api_;", "std::function", "std::uint64_t"})
        require(!has(text, token), "inline scalar holder/payload must not retain API/native/identity owners");
    }
    const auto state = read("src/renderer/dx12/dx12_state_internal.hpp");
    require(has(state, "Dx12DredSite dred_status;") && !has(state, "unique_ptr<Dx12Dred"),
        "diagnostic owner must be inline, not a constructor allocation");
    require(std::count(state.begin(), state.end(), '\n') <= 100, "state wiring exceeded fixed 100-line cap");
    const auto site = read("src/renderer/dx12/dx12_dred_site.cpp");
    require(at(site, "phase_ = Dx12DredSitePhase::checking;") < at(site, "api.removed_reason()"),
        "checking must precede the external reason call");
    require(at(site, "last_loss_ = report;") < at(site, "phase_ = Dx12DredSitePhase::complete;"),
        "complete may only expose a fully owned value");
    const auto native = read("src/renderer/dx12/dx12_dred_native_status.cpp");
    require(has(native, "device_.GetDeviceRemovedReason()") && has(native, "source.QueryInterface(") &&
        !has(native, "state.removal_reason") && !has(native, "state.submitted") &&
        !has(native, "state.failed") && !has(native, "state.recovery_pending") &&
        !has(native, "command_reuse"), "diagnostics must observe native reason, not mutate retirement proof");
    const auto setup = read("src/renderer/dx12/dx12_dred_setup.cpp");
    require(has(setup, "static const Dx12DredSetupObservation configured") &&
        has(setup, "SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON)"),
        "settings observation needs one-copy immutable debug-requested setup");
    const auto factory = read("src/renderer/dx12/dx12_device.cpp");
    require(at(factory, "if (!debug)") < at(factory, "dx12_dred_setup(*debug)") &&
        at(factory, "dx12_dred_setup(*debug)") < at(factory, "CreateDXGIFactory2("),
        "setup must follow immutable debug success and precede this backend's factory");
    const auto adapter = read("src/renderer/dx12/dx12_adapter_device.cpp");
    require(has(adapter, "dx12_observe_dred_status(state);\n  state.dred_status.close_local_lease();\n  state.device.Reset();") &&
        has(adapter, "dx12_observe_dred_status(state);\n    state.dred_status.close_local_lease();\n    state.device.Reset();") &&
        at(adapter, "if (FAILED(result)) return result;") < at(adapter, "state.dred_status.open_local_lease();") &&
        at(adapter, "state.dred_status.open_local_lease();") < at(adapter, "CheckFeatureSupport("),
        "every direct reset and successful native attachment needs the narrow lease seam");
    const auto retirement = read("src/renderer/dx12/dx12_native_retirement.cpp");
    require(at(retirement, "dx12_observe_dred_status(state);") < at(retirement, "state.command_reuse->removed(state)") &&
        at(retirement, "state.command_reuse->removed(state)") < at(retirement, "state.submitted = false;") &&
        at(retirement, "state.submitted = false;") < at(retirement, "state.commands.Reset();") &&
        at(retirement, "state.commands.Reset();") < at(retirement, "state.allocator.Reset();") &&
        at(retirement, "state.allocator.Reset();") < at(retirement, "state.command_reuse->parent_released(state)") &&
        at(retirement, "state.command_reuse->parent_released(state)") < at(retirement, "dx12_reset_texture_resources(state)") &&
        has(retirement, "state.dred_status.close_local_lease();\n  state.device.Reset();"),
        "scalar observation must preserve the existing G/submitted/actual-parent/P/map retirement sequence");
    const auto tests = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_dred_scalar_test_support.cpp", "dx12_dred_setup_test.cpp",
        "dx12_dred_site_test.cpp", "dx12_dred_lease_test.cpp", "dx12_dred_reentry_test.cpp",
        "dx12_dred_api_seam_test.cpp", "dx12_dred_format_test.cpp"})
      require(has(tests, std::string("\"") + name + "\""), "register every new Windows test/support source");
    require(has(read("scripts/ci/architecture-header-targets.txt"),
        "dx12_dred_status_structure_test|tests/architecture/dx12/dx12_dred_status_structure_test.cpp"),
        "architecture glob discovery also needs its exact CI inventory row");
    for (const auto* path : {"include/cgpui/renderer/renderer.hpp",
        "include/cgpui/renderer/renderer_backend_info.hpp",
        "include/cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp"})
      require(!has(read(path), "Dred") && !has(read(path), "DRED"), "no public scalar-DRED contract is introduced");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "DRED scalar structure: " << error.what() << '\n';
    return 1;
  }
}
