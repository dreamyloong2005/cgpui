#include "cgpui/cgpui.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <thread>
#include <variant>

namespace {
class ServiceFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class ServiceRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<ServiceFrame>();
  }
};

class ServiceView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

bool wait_for(const std::atomic_bool& value) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!value.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return value.load();
}
}  // namespace

int main() {
  if (std::getenv("DISPLAY") == nullptr) return 0;
  setenv("CGPUI_LINUX_BACKEND", "x11", 1);
  unsetenv("WAYLAND_DISPLAY");
  auto app = cgpui::create_platform_application();
  if (!app) return 1;

  cgpui::NativeMenuModel menu{
      .items = {{.kind = cgpui::NativeMenuItemKind::submenu,
                 .title = "File",
                 .children = {{.title = "Open",
                               .action_name = "file.open",
                               .accelerator = cgpui::NativeMenuAccelerator{
                                   .key_code = 30}}}}}};
  const auto installed = (*app)->install_native_menu(std::move(menu));
  if (installed.supported || installed.backend != "x11" ||
      installed.menu_count != 1 || installed.item_count != 2 ||
      installed.accelerator_count != 1 ||
      installed.registered_accelerator_count != 0 ||
      installed.skipped_accelerator_count != 1) {
    return 2;
  }
  const auto files = (*app)->show_native_file_dialog(
      {.kind = cgpui::NativeFileDialogKind::open_files,
       .title = "Open",
       .filters = {{.name = "Code", .extensions = {"cpp", "hpp"}}}});
  if (files.supported || files.backend != "x11" ||
      files.kind != cgpui::NativeFileDialogKind::open_files ||
      files.filter_count != 1 || files.error_message.empty()) {
    return 3;
  }
  const auto message = (*app)->show_native_message_dialog({});
  const auto url = (*app)->open_url("https://example.com");
  if (message.supported || message.backend != "x11" ||
      message.error_message.empty() || url.supported ||
      url.backend != "x11" || url.error_message.empty()) {
    return 4;
  }
  ServiceView view;
  ServiceRenderer renderer;
  cgpui::WindowRuntime runtime{
      **app, view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &renderer; }};
  if (runtime.try_show_native_message_dialog({}).has_value() ||
      runtime.try_open_url("https://example.com").has_value()) {
    return 5;
  }
  const auto diagnostics = runtime.platform_diagnostics();
  if (diagnostics.size() != 2 ||
      diagnostics[0].kind != cgpui::PlatformDiagnosticKind::message_dialog ||
      diagnostics[0].backend != "x11" || diagnostics[0].supported ||
      diagnostics[0].succeeded ||
      diagnostics[1].kind != cgpui::PlatformDiagnosticKind::open_url ||
      diagnostics[1].backend != "x11" || diagnostics[1].supported ||
      diagnostics[1].succeeded) {
    return 6;
  }

  std::atomic_bool wakeup{false};
  std::atomic_bool close{false};
  cgpui::PlatformWindow* observed = nullptr;
  auto window = (*app)->create_window(
      {.title = "CGPUI X11 Services", .size = {240.0F, 140.0F}},
      [&](const cgpui::PlatformEvent& event) {
        wakeup.store(wakeup.load() ||
                     std::holds_alternative<cgpui::WindowWakeupRequested>(event));
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          close.store(true);
          if (observed != nullptr) {
            observed->resolve_close_request(
                cgpui::PlatformWindowCloseResolution::accept);
          }
        }
      });
  if (!window) return 7;
  observed = window->get();
  (*window)->update_accessibility_tree({
      .root_element_id = 1,
      .node_count = 2,
      .focused_node_count = 1,
      .nodes = {{.element_id = 1,
                 .role = cgpui::PlatformAccessibilityRole::generic,
                 .name = "Root",
                 .child_count = 1},
                {.element_id = 2,
                 .parent_element_id = 1,
                 .role = cgpui::PlatformAccessibilityRole::button,
                 .name = "Open",
                 .focusable = true,
                 .focused = true}},
  });

  std::thread loop([&] { (*app)->run(); });
  (*app)->request_wakeup();
  if (!wait_for(wakeup)) { (*app)->quit(); loop.join(); return 8; }
  (*window)->request_close();
  if (!wait_for(close)) { (*app)->quit(); loop.join(); return 9; }
  (*app)->quit();
  loop.join();
  return (*window)->close_request_state().accepted ? 0 : 10;
}
