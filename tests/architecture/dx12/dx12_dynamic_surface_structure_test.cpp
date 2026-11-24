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
  if (!file) throw std::runtime_error(std::string("missing dynamic surface owner: ") + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
void cap(const char* path, unsigned lines) {
  const auto text = read(path);
  require(std::count(text.begin(), text.end(), '\n') <= lines, "focused dynamic surface owner grew too broad");
}
}
int main() {
  try {
    cap("src/renderer/dx12/dx12_surface.cpp", 85);
    cap("src/renderer/dx12/dx12_surface_policy_internal.hpp", 35);
    cap("src/renderer/dx12/dx12_surface_policy.cpp", 35);
    cap("src/renderer/dx12/dx12_surface_transition.cpp", 90);
    cap("src/renderer/dx12/dx12_composition_api.cpp", 30);
    cap("src/renderer/dx12/dx12_swapchain_candidate.cpp", 85);
    cap("src/renderer/dx12/dx12_window_alpha_reservation.cpp", 65);
    cap("src/platform/win32/win32_window_surface_alpha_lease.cpp", 100);
    const auto surface = read("src/renderer/dx12/dx12_surface.cpp");
    require(has(surface, "dx12_reserve_ordinary_composition") && has(surface, "dx12_surface_alpha_change") &&
        has(surface, "state.composition_surface ? RendererCompositeAlpha::pre_multiplied") &&
        has(surface, "if (!state.composition_surface) return color"), "route must not redefine actual opaque/PREMULT clear/report");
    const auto reservation = read("src/renderer/dx12/dx12_window_alpha_reservation.cpp");
    require(has(reservation, "GetCurrentThreadId()") && has(reservation, "GetCurrentProcessId()") &&
        has(reservation, "unsupported_platform) return false"), "only fresh managed owner-thread eligible opaque windows opt into composition");
    const auto platform = read("src/platform/win32/win32_window_surface_alpha_lease.cpp");
    const auto reserve = platform.substr(platform.find("::reserve("), platform.find("::acquire(") - platform.find("::reserve("));
    require(has(reserve, "win32_set_redirection_alpha(hwnd_, false)") && !has(reserve, "active_ = true") &&
        !has(reserve, "win32_repair_native_menu_alpha(") && has(platform, "reserved_ ? suspend() : release_owner()"),
        "reservation does not enable alpha; failed promotion rolls back without losing ownership");
    const auto owner = read("src/platform/win32/win32_window_surface_alpha.cpp");
    require(has(owner, "return !active_ ||") && !has(owner, "return !reserved_ ||"),
        "opaque reservation must not impose transparent-window chrome/menu restrictions");
    const auto transition = read("src/renderer/dx12/dx12_surface_transition.cpp");
    require(has(transition, "if (state.frame_active)") &&
        transition.find("dx12_prepare_composition_chain") < transition.find("dx12_acquire_window_alpha") &&
        transition.find("dx12_bind_composition(host, candidate->swapchain.Get())") < transition.find("dx12_exchange_swapchain") &&
        has(transition, "if (!host.retired_chain) host.retired_chain = std::move(candidate)") &&
        has(transition, "host.failed_binding = std::move(candidate)") && has(transition, "state.surface_changing") &&
        !has(transition, "->Commit("),
        "fully prepare before binding, retain old owners and never commit outside successful frame gate");
    const auto composition = read("src/renderer/dx12/dx12_composition.cpp");
    const auto presentation = read("src/renderer/dx12/dx12_presentation.cpp");
    require(has(composition, "dx12_composition_route(state.surface_policy)") &&
        composition.find("dx12_apply_composition(host)") < composition.find("host.retired_chain.reset()") &&
        presentation.find("dx12_commit_composition(state)") < presentation.find("diagnostics.complete_successful_frame()") &&
        presentation.find("dx12_commit_composition(state)") < presentation.find("state.pixels ="),
        "Commit must precede public success publication for IGNORE as well as PREMULT composition");
    require(has(read("src/renderer/dx12/dx12_command_reuse_selection.cpp"), "dx12_composition_route(state.surface_policy)"),
        "new opaque composition route stays conservatively DIRECT, not unvalidated BUNDLE reuse");
    const auto tests = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_surface_policy_test.cpp", "dx12_dynamic_alpha_opaque_test.cpp",
        "dx12_dynamic_alpha_menu_test.cpp", "dx12_dynamic_alpha_cancel_test.cpp", "dx12_dynamic_alpha_lifetime_test.cpp",
        "dx12_dynamic_alpha_worker_test.cpp", "dx12_dynamic_alpha_failure_test.cpp", "dx12_dynamic_alpha_rollback_test.cpp"})
      require(has(tests, name), "dynamic ordinary contract needs explicit behavior registration");
    require(has(read("tests/platform/mcpp.toml"), "win32_surface_alpha_reservation_test.cpp"), "platform reservation test missing");
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
