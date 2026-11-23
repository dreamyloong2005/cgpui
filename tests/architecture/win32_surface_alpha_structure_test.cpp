#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!stream) throw std::runtime_error(std::string("missing alpha owner: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
bool has(const std::string& text, const char* value) { return text.find(value) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void capped(const char* path, unsigned cap) {
  const auto text = read(path);
  require(std::count(text.begin(), text.end(), '\n') <= cap, "ordinary alpha leaves exceeded focused line caps");
}
} // namespace

int main() {
  try {
    for (const auto* path : {
        "src/platform/win32/win32_window_surface_alpha.cpp",
        "src/platform/win32/win32_window_surface_alpha_protocol.cpp",
        "src/platform/win32/win32_window_proc_surface_alpha.cpp",
        "src/platform/win32/win32_native_menu_alpha.cpp",
        "src/renderer/dx12/dx12_window_alpha.cpp"}) {
      capped(path, 150);
      const auto text = read(path);
      for (const auto* banned : {"WM_UAH", "UpdateLayeredWindow", "WM_PRINT",
                                "DwmExtendFrameIntoClientArea", "DwmEnableBlurBehindWindow",
                                "SetLayeredWindowAttributes", "SetWindowSubclass"})
        require(!has(text, banned), "alpha must not replace native drawing, topology or material management");
    }
    for (const auto* path : {
        "src/platform/win32/win32_window_surface_alpha_internal.hpp",
        "src/platform/win32/win32_window_proc_surface_alpha_internal.hpp",
        "src/platform/win32/win32_native_menu_alpha_internal.hpp",
        "src/renderer/dx12/dx12_window_alpha_internal.hpp"}) capped(path, 90);
    capped("src/platform/win32_surface_alpha_protocol_internal.hpp", 60);
    // Keep the existing broad owners' caps, rather than paying for a leaf by
    // increasing their old allowances.
    capped("src/platform/win32/win32_internal.hpp", 80);
    capped("src/platform/win32/win32_window_internal.hpp", 134);
    capped("src/platform/win32/win32_application.cpp", 220);
    capped("src/platform/win32/win32_window.cpp", 130);
    capped("src/platform/win32/win32_window_chrome.cpp", 90);
    capped("src/platform/win32/win32_window_proc.cpp", 80);

    const auto protocol = read("src/platform/win32_surface_alpha_protocol_internal.hpp");
    require(has(protocol, "window_instance") && has(protocol, "owner_cookie") &&
                has(protocol, "win32_surface_alpha_instance_message") &&
                has(protocol, "win32_surface_alpha_mark_opaque_message") &&
                !has(protocol, "Win32SurfaceAlphaRequest") && !has(protocol, "std::function"),
            "the versioned private protocol carries only tokens, never request addresses or callbacks");
    const auto owner = read("src/platform/win32/win32_window_surface_alpha.cpp");
    const auto requests = read("src/platform/win32/win32_window_surface_alpha_protocol.cpp");
    require(has(owner, "minimum_build = 26100") && has(owner, "FAILED(enabled)") &&
                has(owner, "SetPropW") && has(owner, "RemovePropW") && has(owner, "opaque_history_") &&
                has(requests, "instance != instance_") &&
                has(requests, "owner_cookie_ == owner_cookie") &&
                has(requests, "opaque_history_ = true") && has(requests, "ERROR_BUSY"),
            "alpha leases need real OS success, per-window identity and one-way HWND flip history");
    const auto window = read("src/platform/win32/win32_window.cpp");
    require(has(window, "surface_alpha_.attach(hwnd)") && has(window, "surface_alpha_.detach()") &&
                window.find("surface_alpha_.detach()") < window.find("DestroyWindow(hwnd_)"),
            "window lifetime owns alpha retirement before native destruction");
    const auto proc = read("src/platform/win32/win32_window_proc.cpp");
    const auto after = read("src/platform/win32/win32_window_proc_surface_alpha.cpp");
    require(has(proc, "win32_window_proc_handle_surface_alpha(") &&
                has(proc, "win32_window_proc_after_surface_alpha(") &&
                proc.rfind("win32_window_proc_after_surface_alpha(") > proc.rfind("DefWindowProcW(") &&
                has(after, "alpha->instance() == instance") && has(after, "win32_native_menu_alpha_event(message)"),
            "menu repair must occur after native dispatch and revalidate HWND identity after callbacks");
    require(has(after, "alpha->request(message, static_cast<UINT_PTR>(wparam), static_cast<UINT_PTR>(lparam))") &&
                !has(after, "reinterpret_cast<Win32SurfaceAlpha") && !has(after, "InSendMessageEx") &&
                !has(requests, "reinterpret_cast"),
            "posted and synchronous protocol messages validate integer tokens without dereferencing caller data");
    const auto menu = read("src/platform/win32/win32_native_menu_alpha.cpp");
    require(has(menu, "GetMenuBarInfo(hwnd, OBJID_MENU") && has(menu, "BPPF_NONCLIENT") &&
                has(menu, "SRCCOPY") && has(menu, "BufferedPaintSetAlpha(paint, &rectangle, 255)") &&
                menu.find("BitBlt(") < menu.find("GdiFlush()") &&
                menu.find("GdiFlush()") < menu.find("BufferedPaintSetAlpha(") &&
                !has(menu, "FillRect(") && !has(menu, "DrawText") && !has(menu, "WM_COMMAND"),
            "native menu RGB stays untouched; flush its copied GDI pixels before setting alpha only");
    const auto backdrop = read("src/platform/win32/win32_window_backdrop.cpp") +
        read("src/platform/win32/win32_window_backdrop_plan.cpp");
    require(!has(backdrop, "surface_alpha") && !has(owner, "apply_window_backdrop") &&
                !has(owner, "win32_plan_window_backdrop"),
            "native alpha ownership is independent of material/backdrop ownership");
    const auto renderer = read("src/renderer/dx12/dx12_window_alpha.cpp");
    const auto entry = read("src/renderer/dx12/dx12_renderer.cpp");
    require(has(renderer, "win32_surface_alpha_protocol_internal.hpp") &&
                has(renderer, "PostMessageW") && has(renderer, "GetCurrentThreadId()") &&
                has(renderer, "GetCurrentProcessId()") && has(renderer, "win32_surface_alpha_mark_opaque_message") &&
                !has(renderer, "Win32SurfaceAlphaRequest") &&
                !has(renderer, "Win32Window") && !has(renderer, "DwmSetWindowAttribute") &&
                entry.find("dx12_mark_opaque_window(") < entry.find("dx12_create_swapchain("),
            "renderer lease uses only private HWND tokens and marks flip use before native swapchain creation");
    const auto package = read("packages/renderer-dx12/mcpp.toml");
    require(!has(package, "platform-win32") && !has(package, "uxtheme.lib"),
            "DX12 must not link concrete platform implementation or own native menu painting");
    const auto platform_tests = read("tests/platform/mcpp.toml");
    require(has(platform_tests, "win32_surface_alpha_lease_test.cpp") &&
                has(read("tests/platform/win32_surface_alpha_lease_test.cpp"), "reject_posted_bogus_tokens") &&
                has(read("packages/platform-win32/mcpp.toml"), "uxtheme.lib"),
            "platform owns alpha behavior tests and BufferedPaint dependency");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "ordinary alpha structure: " << error.what() << '\n';
    return 1;
  }
}
