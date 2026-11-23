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
  if (!input) throw std::runtime_error("missing experimental composition owner: " + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& value, const char* token) { return value.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
std::string probe(const char* name) { return read(std::string("tests/renderer-dx12/") + name); }
void limit(const char* name, unsigned maximum) {
  const auto text = probe(name);
  require(std::count(text.begin(), text.end(), '\n') <= maximum, "experimental owner exceeded focused budget");
}
}
int main() {
  try {
    const auto header = probe("dx12_composition_probe_test_support.hpp");
    const auto setup = probe("dx12_composition_probe_setup_test_support.cpp");
    const auto wait = probe("dx12_composition_probe_wait_test_support.cpp");
    const auto frame = probe("dx12_composition_probe_frame_test_support.cpp");
    const auto resize = probe("dx12_composition_probe_resize_test_support.cpp");
    const auto api = probe("dx12_composition_probe_trace_test_support.cpp");
    require(!has(header, "try_admit()") && !has(header, "CreateSwapChainForComposition(") &&
        !has(header, "WaitForSingleObject") && !has(header, "CloseHandle("),
        "declaration header cannot own native implementation");
    require(has(setup, "std::make_shared<Dx12State>(&api)") &&
        has(setup, "CreateSwapChainForComposition(") &&
        has(setup, "DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL") &&
        has(setup, "DXGI_ALPHA_MODE_PREMULTIPLIED") && !has(setup, "->Commit(") &&
        !has(setup, "dx12_create_swapchain("), "probe must bind a real flagged composition chain without early Commit");
    require(has(wait, "state->frame_pacing->try_admit()") &&
        has(wait, "!state->composition_host->committed") && has(wait, "std::chrono::seconds(3)") &&
        has(wait, "composition_probe_dispatch()") && !has(wait, "Present("),
        "bounded first admission must use actual owner before commit/presentation");
    const auto present = frame.find("state->swapchain->Present(1, 0)");
    const auto consume = frame.find("state->frame_pacing->presentation_attempted()");
    require(present != std::string::npos && consume > present &&
        frame.find("Commit_after_first_Present") > consume &&
        frame.find("post_present_retirement") > consume,
        "actual Present must consume grant before result processing; retirement/Commit remain separate");
    require(has(resize, "DXGI_FORMAT_R8G8B8A8_UNORM, creation_flags_") &&
        has(resize, "api.sets == sets && api.gets == gets && api.closes == closes") &&
        !has(resize, "dx12_resize(*"), "same-chain resize must preserve its actual flags and owner");
    require(has(api, "native.set_maximum_latency(chain, latency)") &&
        has(api, "native.latency_object(chain)") && has(api, "native.poll(handle)") &&
        has(api, "native.close(handle)") && !has(api, "GetLastError("),
        "experiment observer must forward every native operation without manufacturing status");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    require(has(manifest, "[targets.dx12_composition_pacing_probe]") &&
        has(manifest, "main = \"dx12_composition_pacing_probe.cpp\"") &&
        !has(manifest, "\"dx12_composition_pacing_probe.cpp\","),
        "capability experiment is an explicit executable, not a required-pass discovered test");
    for (const auto* part : {"setup", "trace", "wait", "frame", "pixels", "resize", "case", "windows"}) {
      const auto name = std::string("dx12_composition_probe_") + part + "_test_support.cpp";
      limit(name.c_str(), 140);
      require(has(manifest, name.c_str()), "register each focused support implementation");
    }
    limit("dx12_composition_probe_test_support.hpp", 70);
    limit("dx12_composition_probe_windows_test_support.hpp", 40);
    limit("dx12_composition_pacing_probe.cpp", 35);
    require(has(read("scripts/ci/architecture-header-targets.txt"),
        "dx12_composition_probe_structure_test|tests/architecture/dx12/dx12_composition_probe_structure_test.cpp"),
        "register the experimental boundary guard in exact CI inventory");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
