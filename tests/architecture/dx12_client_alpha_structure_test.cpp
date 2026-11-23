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
  if (!input) throw std::runtime_error(std::string("missing client-alpha owner: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& source, const char* token) { return source.find(token) != std::string::npos; }
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
void cap(const char* path, unsigned maximum) {
  const auto source = read(path);
  if (std::count(source.begin(), source.end(), '\n') > maximum)
    throw std::runtime_error(std::string("existing module cap exceeded: ") + path);
}
} // namespace

int main() {
  try {
    // Preserve the tightest existing allowances rather than raise a broad
    // owner's threshold to accommodate a new capability.
    cap("src/platform/win32/win32_window_internal.hpp", 134);
    cap("src/platform/win32/win32_application.cpp", 170);
    cap("src/platform/win32/win32_window_title_bar.cpp", 125);
    cap("src/platform/win32/win32_window_title_bar_actions.cpp", 105);
    cap("src/platform/win32/win32_window_title_bar_hit_test.cpp", 90);
    cap("src/platform/win32/win32_window_surface_alpha.cpp", 150);
    cap("src/platform/win32/win32_window_surface_alpha_internal.hpp", 90);
    cap("src/platform/win32/win32_native_menu_policy.cpp", 100);
    cap("src/platform/win32/win32_native_menu_policy_internal.hpp", 60);

    const auto policy = read("src/platform/win32/win32_native_menu_policy.cpp");
    const auto policy_header = read("src/platform/win32/win32_native_menu_policy_internal.hpp");
    const auto app = read("src/platform/win32/win32_application.cpp");
    const auto alpha = read("src/platform/win32/win32_window_surface_alpha.cpp");
    const auto alpha_header = read("src/platform/win32/win32_window_surface_alpha_internal.hpp");
    require(has(policy_header, "class Win32NativeMenuInstallationGuard") &&
                has(policy_header, "~Win32NativeMenuInstallationGuard()") &&
                has(policy_header, "allowed() const") &&
                has(policy_header, "win32_native_menu_installation_active()"),
            "menu installation policy belongs to a private RAII leaf");
    require(has(policy, "thread_local") && has(policy, "Win32NativeMenuInstallationGuard::") &&
                has(policy, "accepts_native_menu()") && has(policy, ".visit(") &&
                !has(policy, "SetMenu(") && !has(policy, "DrawMenuBar("),
            "transaction policy preflights live windows without performing native menu mutation");
    require(has(policy, "if (installing_native_menu) return") &&
                has(policy, "if (owns_reservation_) installing_native_menu = false") &&
                policy.find("windows.visit(") < policy.find("installing_native_menu = true"),
            "menu policy rejects reentry and reserves only after the allocating preflight has completed");
    require(has(app, "win32_native_menu_policy_internal.hpp") &&
                has(app, "Win32NativeMenuInstallationGuard") && has(app, ".allowed()") &&
                has(app, "window_handle(window), nullptr") &&
                app.find("Win32NativeMenuInstallationGuard") < app.find("window_handle(window), nullptr") &&
                app.find("Win32NativeMenuInstallationGuard") < app.find("native_menu_state_.install_native_menu("),
            "application must reject before detaching any native menu or replacing its model");
    require(!has(app, "thread_local") && !has(app, "surface_alpha_") && !has(app, "GetMenu("),
            "the application entry delegates rather than duplicating alpha/window policy");
    require(has(alpha_header, "accepts_native_menu() const") &&
                has(alpha, "WindowTitleBarMode::client") && has(alpha, "GetMenu(hwnd_)") &&
                has(alpha, "win32_native_menu_installation_active()") &&
                has(alpha, "options.title_bar == title_bar_") && has(alpha, "WS_CAPTION"),
            "client alpha retains caption style, rejects menu conflicts and freezes its initial chrome mode");
    for (const auto* source : {&policy, &alpha}) {
      for (const auto* banned : {"SetWindowLong", "SetMenu(", "DrawMenuBar(", "WM_UAH",
                                "WM_NCCALCSIZE", "DrawText", "UpdateLayeredWindow"})
        require(!has(*source, banned), "alpha admission must not rewrite native chrome or hide/replace menus");
    }
    const auto tests = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_client_initial_alpha_test.cpp", "dx12_client_alpha_lifetime_test.cpp",
                             "dx12_client_alpha_recovery_test.cpp", "dx12_client_menu_refusal_test.cpp",
                             "dx12_client_menu_reentrancy_test.cpp", "dx12_client_controls_test.cpp"}) {
      require(has(tests, name), "client alpha behavior tests must be registered in the native suite");
      const auto body = read((std::string("tests/renderer-dx12/") + name).c_str());
      require(!has(body, "../../src/") && !has(body, "DwmSetWindowAttribute") &&
                  !has(body, "BufferedPaintSetAlpha"),
              "native acceptance must use public factories without manual alpha repair");
    }
    const auto renderer = read("packages/renderer-dx12/mcpp.toml");
    require(!has(renderer, "platform-win32") && !has(renderer, "uxtheme.lib"),
            "the renderer cannot acquire a concrete platform dependency for client-menu policy");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "client alpha structure: " << error.what() << '\n';
    return 1;
  }
}
