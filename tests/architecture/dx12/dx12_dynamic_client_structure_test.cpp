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
  if (!file) throw std::runtime_error(std::string("missing dynamic-client owner: ") + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& value, const char* token) { return value.find(token) != std::string::npos; }
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
void cap(const char* path, unsigned maximum) {
  const auto text = read(path);
  require(std::count(text.begin(), text.end(), '\n') <= maximum, "dynamic-client focused owner exceeds cap");
}
}
int main() {
  try {
    const auto lease = read("src/platform/win32/win32_window_surface_alpha_lease.cpp");
    const auto reserve_at = lease.find("::reserve("), acquire_at = lease.find("::acquire("), suspend_at = lease.find("::suspend(");
    require(reserve_at != std::string::npos && acquire_at > reserve_at && suspend_at > acquire_at,
        "reservation/promotion ownership order is explicit");
    const auto reservation = lease.substr(reserve_at, acquire_at - reserve_at);
    const auto promotion = lease.substr(acquire_at, suspend_at - acquire_at);
    require(has(reservation, "if (!supported()) return E_NOTIMPL;") &&
        has(promotion, "if (!supported()) return E_NOTIMPL;") && !has(lease, "title_bar_ != WindowTitleBarMode::system"),
        "fresh reservation and promotion reuse the same existing current capability, not a system-only special case");
    const auto owner = read("src/platform/win32/win32_window_surface_alpha.cpp");
    require(has(owner, "minimum_build = 26100") && has(owner, "opaque_history_") &&
        has(owner, "title_bar_ == WindowTitleBarMode::client") && has(owner, "GetMenu(hwnd_)") &&
        has(owner, "win32_native_menu_installation_active()") && has(owner, "WS_CAPTION") &&
        has(owner, "WS_CHILD | WS_POPUP") && has(owner, "WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP"),
        "client+HMENU, in-flight install, old OS, raw history and native topology restrictions remain");
    require(has(owner, "return !active_ ||") && !has(owner, "return !reserved_ ||") &&
        has(lease, "reserved_ ? suspend() : release_owner()"),
        "reservation leaves opaque chrome/menu permissions; failed promotion rollback is separate");
    cap("src/platform/win32/win32_window_surface_alpha_lease.cpp", 100);
    cap("tests/renderer-dx12/dx12_dynamic_client_test_support.hpp", 20);
    cap("tests/renderer-dx12/dx12_dynamic_client_test_support.cpp", 65);
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_dynamic_client_pixels_test.cpp", "dx12_dynamic_client_lifetime_test.cpp",
                            "dx12_dynamic_client_menu_test.cpp", "dx12_dynamic_client_menu_reentry_test.cpp"}) {
      require(has(manifest, name), "dynamic client native case must be registered");
      const auto path = std::string("tests/renderer-dx12/") + name;
      const auto test = read(path.c_str());
      cap(path.c_str(), 100);
      require(!has(test, "../../src/") && !has(test, "DwmSetWindowAttribute") &&
          !has(test, "BufferedPaintSetAlpha") && !has(test, "CreateSwapChainFor"),
          "dynamic client acceptance uses public renderer/platform APIs without manual alpha or routing repair");
    }
    require(has(manifest, "dx12_dynamic_client_test_support.cpp") && has(manifest, "dx12_client_menu_refusal_test.cpp") &&
        has(read("tests/platform/win32_surface_alpha_reservation_test.cpp"), "WindowTitleBarMode::client"),
        "focused support, existing client+HMENU refusal and platform reservation controls remain owned");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
