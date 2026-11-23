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
  if (!input) throw std::runtime_error(std::string("missing composition first-begin leaf: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void limit(const char* path, std::size_t maximum) {
  const auto text = read(path);
  require(std::count(text.begin(), text.end(), '\n') <= maximum, "composition first-begin leaf exceeds cap");
}
} // namespace

int main() {
  try {
    constexpr auto base = "tests/renderer-dx12/";
    const auto source = [&](const char* name) { return read((std::string(base) + name).c_str()); };
    const auto header = source("dx12_composition_first_begin_test_support.hpp");
    const auto window = source("dx12_composition_first_begin_window_test_support.cpp");
    const auto visibility = source("dx12_composition_first_begin_visibility_test_support.cpp");
    const auto branch = source("dx12_composition_first_begin_case_test_support.cpp");
    require(has(header, "class Dx12CompositionFirstBeginWindows") &&
                has(header, "bool matches(bool visible) const;") && has(header, "bool show_explicitly();"),
            "declare focused native-window and explicit-visibility seam");
    for (const auto* token : {"return ", "CreateWindowExW(", "ShowWindow(", "begin_frame(", "std::cout"})
      require(!has(header, token), "first-begin header must remain declaration-only");
    require(has(window, "CreateWindowExW(") && has(window, "WS_EX_NOREDIRECTIONBITMAP") &&
                has(window, ".composition = true"), "construct the actual matching composition HWND topology");
    for (const auto* token : {"ShowWindow(", "SetWindowPos(", "WS_VISIBLE", "CompositionTestWindows"})
      require(!has(window, token), "construction must never show then hide the alleged initially-hidden popup");
    require(has(visibility, "ShowWindow(owner_, SW_SHOWNOACTIVATE)") &&
                has(visibility, "ShowWindow(popup_, SW_SHOWNOACTIVATE)") &&
                has(visibility, "IsWindowVisible(popup_)") && has(visibility, "GetWindow(popup_, GW_OWNER)"),
            "explicit show must preserve native ownership/nonactivation and independently check visibility");
    require(has(branch, "if (visible && !windows.show_explicitly())") &&
                has(source("dx12_composition_first_begin_hidden_test.cpp"), "composition_first_begin_case(false)") &&
                has(source("dx12_composition_first_begin_visible_test.cpp"), "composition_first_begin_case(true)"),
            "hidden entry never takes the only explicit-show branch; visible entry does");
    require(has(branch, "Dx12PacingForwardApi api;") &&
                has(branch, "dx12_create_renderer(windows.descriptor(), api)") &&
                has(branch, "pacing_begin_bounded(*renderer, api)"),
            "observe the actual renderer with the strict native forwarder, not a mock renderer");
    const auto begin = branch.find("auto frame = pacing_begin_bounded");
    const auto red = branch.find("FAIL_UNPACED_FIRST_FRAME");
    require(begin != std::string::npos && red != std::string::npos && begin < red &&
                !has(branch.substr(0, begin), "api.sets !=") && !has(branch.substr(0, begin), "api.gets !=") &&
                has(branch, "api.polls == 0 || api.ready != 1"),
            "meaningful RED must observe actual returned first frame, not fail early on setup counters");
    require(branch.find("frame->reset();") < branch.find("renderer.reset();") &&
                branch.find("Dx12PacingForwardApi api;") < branch.find("auto created ="),
            "all shared native state must die before its borrowed API and native HWND fixture");
    for (const auto* token : {"->present(", "ExecuteCommandLists(", "->Commit(", "ShowWindow(",
                             "Dx12PacingControlApi", "Dx12PacingScriptApi"})
      require(!has(branch, token), "first-begin oracle cannot prime presentation or manufacture readiness");
    const auto wait = source("dx12_pacing_begin_wait_test_support.cpp");
    require(has(wait, "renderer.begin_frame()") && has(wait, "std::chrono::seconds(3)") &&
                has(wait, "count != 32") && has(wait, "PostQuitMessage") && has(wait, "Sleep(1)"),
            "reuse real virtual begin and finite bounded-message admission helper");
    const auto forward = source("dx12_pacing_native_api_test_support.cpp");
    for (const auto* token : {"native.set_maximum_latency", "native.latency_object", "native.poll(handle)",
                             "native.close(handle)", "return result;"})
      require(has(forward, token), "native observer forwards without substituted wait status");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    const auto build = manifest.find("[target.'cfg(os = \"windows\")'.build]");
    const auto test = manifest.find("[target.'cfg(os = \"windows\")'.test]");
    require(build != std::string::npos && test != std::string::npos, "native entries remain Windows-only");
    for (const auto* name : {"dx12_composition_first_begin_window_test_support.cpp",
                             "dx12_composition_first_begin_visibility_test_support.cpp",
                             "dx12_composition_first_begin_case_test_support.cpp"}) {
      require(manifest.find(name, build) < test, "register focused native support in the Windows build");
      limit((std::string(base) + name).c_str(), 120);
    }
    for (const auto* name : {"dx12_composition_first_begin_hidden_test.cpp",
                             "dx12_composition_first_begin_visible_test.cpp"}) {
      require(manifest.find(name, test) != std::string::npos, "register both native first-begin conditions");
      limit((std::string(base) + name).c_str(), 10);
    }
    limit("tests/renderer-dx12/dx12_composition_first_begin_test_support.hpp", 60);
    require(has(read("tests/architecture/mcpp.toml"), "\"dx12/*_structure_test.cpp\"") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "dx12_composition_first_begin_structure_test|tests/architecture/dx12/dx12_composition_first_begin_structure_test.cpp"),
            "register structure guard for local and CI discovery");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
