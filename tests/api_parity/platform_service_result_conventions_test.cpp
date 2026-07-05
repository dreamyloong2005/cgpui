#include "cgpui/cgpui.hpp"

#include <expected>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>()
                           .try_install_native_menu(
                               std::declval<cgpui::NativeMenuModel>())),
              cgpui::Result<cgpui::NativeMenuInstallation>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::AppContext&>()
                           .try_install_native_menu(
                               std::declval<cgpui::NativeMenuModel>())),
              cgpui::Result<cgpui::NativeMenuInstallation>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntimeContext&>()
                           .try_install_native_menu(
                               std::declval<cgpui::NativeMenuModel>())),
              cgpui::Result<cgpui::NativeMenuInstallation>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntime&>()
                           .try_show_native_file_dialog(
                               std::declval<cgpui::NativeFileDialogOptions>())),
              cgpui::Result<cgpui::NativeFileDialogResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::AppContext&>()
                           .try_show_native_file_dialog(
                               std::declval<cgpui::NativeFileDialogOptions>())),
              cgpui::Result<cgpui::NativeFileDialogResult>>);
static_assert(std::is_same_v<
              decltype(std::declval<cgpui::WindowRuntimeContext&>()
                           .try_show_native_file_dialog(
                               std::declval<cgpui::NativeFileDialogOptions>())),
              cgpui::Result<cgpui::NativeFileDialogResult>>);

class ServiceResultFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class ServiceResultRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<ServiceResultFrame>();
  }
};

class ServiceResultView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class ServiceResultWindow final : public cgpui::PlatformWindow {
 public:
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }

  cgpui::WindowState state() const override {
    return cgpui::WindowState{
        .framebuffer_size = {.width = 320.0F, .height = 240.0F},
        .scale = cgpui::DpiScale{1.0F},
        .close_requested = false};
  }

  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}
};

class ServiceResultApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformEventCallback) override {
    return std::unexpected(cgpui::Error{
        .code = cgpui::ErrorCode::window_creation_failed,
        .message = "window creation is outside this test"});
  }

  cgpui::PlatformMenuInstallationResult install_native_menu(
      cgpui::NativeMenuModel menu) override {
    install_native_menu_count += 1;
    last_menu = std::move(menu);
    cgpui::PlatformMenuInstallationResult result = menu_result;
    result.menu_count = last_menu.items.size();
    result.item_count = cgpui::native_menu_item_count(last_menu);
    result.accelerator_count =
        cgpui::native_menu_accelerator_count(last_menu);
    return result;
  }

  cgpui::NativeFileDialogResult show_native_file_dialog(
      cgpui::NativeFileDialogOptions options) override {
    show_native_file_dialog_count += 1;
    last_dialog_options = std::move(options);
    cgpui::NativeFileDialogResult result = dialog_result;
    result.kind = last_dialog_options.kind;
    result.filter_count = last_dialog_options.filters.size();
    return result;
  }

  int run() override { return 0; }
  void quit() override {}

  cgpui::PlatformMenuInstallationResult menu_result{
      .supported = true,
      .backend = "service-supported",
  };
  cgpui::NativeFileDialogResult dialog_result{
      .supported = true,
      .accepted = true,
      .backend = "dialog-supported",
  };
  int install_native_menu_count = 0;
  int show_native_file_dialog_count = 0;
  cgpui::NativeMenuModel last_menu;
  cgpui::NativeFileDialogOptions last_dialog_options;
};

struct ServiceResultFixture {
  ServiceResultApplication app;
  ServiceResultView view;
  ServiceResultRenderer renderer;
  ServiceResultWindow window;
  cgpui::WindowRuntime runtime{
      app,
      view,
      [this](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> { return &renderer; }};

  [[nodiscard]] cgpui::WindowRuntimeContext context() {
    return cgpui::WindowRuntimeContext{
        .runtime = runtime,
        .application = app,
        .platform_window = window,
        .renderer = renderer,
        .window_runtime_id = runtime.root_window_runtime_id(),
        .view_id = cgpui::ViewId{1},
        .viewport_size = {.width = 320.0F, .height = 240.0F},
        .scale = cgpui::DpiScale{1.0F},
        .input = {},
        .event_route = {},
        .last_event_result = {},
        .last_event_dispatch = {},
        .frame_index = 0};
  }
};

cgpui::NativeMenuModel menu_with_item(std::string title) {
  return cgpui::NativeMenuModel{
      .items = {cgpui::NativeMenuItem{
          .kind = cgpui::NativeMenuItemKind::command,
          .title = std::move(title),
          .action_name = "test.action",
      }}};
}

int test_unsupported_menu_result_does_not_overwrite_last_success() {
  ServiceResultFixture fixture;

  cgpui::Result<cgpui::NativeMenuInstallation> installed =
      fixture.runtime.try_install_native_menu(menu_with_item("Supported"));
  if (!installed.has_value() ||
      installed->platform.backend != "service-supported" ||
      fixture.runtime.native_menu_installation().model.items[0].title !=
          "Supported") {
    return 1;
  }

  fixture.app.menu_result = cgpui::PlatformMenuInstallationResult{
      .supported = false,
      .backend = "service-unsupported",
  };
  cgpui::Result<cgpui::NativeMenuInstallation> unsupported =
      fixture.runtime.try_install_native_menu(menu_with_item("Unsupported"));

  if (unsupported.has_value()) {
    return 2;
  }
  if (unsupported.error().code != cgpui::ErrorCode::unsupported_platform ||
      unsupported.error().message !=
          "native menu unsupported by service-unsupported") {
    return 3;
  }
  if (fixture.runtime.native_menu_installation().platform.backend !=
          "service-supported" ||
      fixture.runtime.native_menu_installation().model.items[0].title !=
          "Supported") {
    return 4;
  }
  if (fixture.app.install_native_menu_count != 2) {
    return 5;
  }
  return 0;
}

int test_contexts_forward_platform_service_results() {
  ServiceResultFixture fixture;
  cgpui::AppContext app_context{.runtime = fixture.runtime};
  cgpui::WindowRuntimeContext window_context = fixture.context();

  cgpui::Result<cgpui::NativeMenuInstallation> from_app_context =
      app_context.try_install_native_menu(menu_with_item("App Context"));
  fixture.app.dialog_result = cgpui::NativeFileDialogResult{
      .supported = true,
      .accepted = false,
      .backend = "dialog-supported",
  };
  cgpui::Result<cgpui::NativeFileDialogResult> from_window_context =
      window_context.try_show_native_file_dialog(cgpui::NativeFileDialogOptions{
          .kind = cgpui::NativeFileDialogKind::save_file,
          .title = "Save",
          .filters = {cgpui::NativeFileDialogFilter{
              .name = "Projects",
              .extensions = {"cgpui"},
          }}});

  if (!from_app_context.has_value() ||
      from_app_context->platform.backend != "service-supported" ||
      from_app_context->model.items[0].title != "App Context") {
    return 6;
  }
  if (!from_window_context.has_value() || from_window_context->accepted ||
      from_window_context->backend != "dialog-supported" ||
      from_window_context->kind != cgpui::NativeFileDialogKind::save_file ||
      from_window_context->filter_count != 1) {
    return 7;
  }
  if (!fixture.runtime.native_file_dialog_result().supported ||
      fixture.runtime.native_file_dialog_result().accepted ||
      fixture.runtime.native_file_dialog_result().backend !=
          "dialog-supported") {
    return 8;
  }
  return 0;
}

int test_unsupported_dialog_result_does_not_overwrite_last_success() {
  ServiceResultFixture fixture;

  fixture.app.dialog_result = cgpui::NativeFileDialogResult{
      .supported = true,
      .accepted = true,
      .backend = "dialog-supported",
      .paths = {"D:/Projects/project.cgpui"},
  };
  cgpui::Result<cgpui::NativeFileDialogResult> accepted =
      fixture.runtime.try_show_native_file_dialog(cgpui::NativeFileDialogOptions{
          .kind = cgpui::NativeFileDialogKind::open_file,
          .title = "Open",
      });
  if (!accepted.has_value() || !accepted->accepted ||
      fixture.runtime.native_file_dialog_result().paths.size() != 1) {
    return 9;
  }

  fixture.app.dialog_result = cgpui::NativeFileDialogResult{
      .supported = false,
      .accepted = false,
      .backend = "dialog-unsupported",
      .error_message = "dialog missing",
  };
  cgpui::Result<cgpui::NativeFileDialogResult> unsupported =
      fixture.runtime.try_show_native_file_dialog(cgpui::NativeFileDialogOptions{
          .kind = cgpui::NativeFileDialogKind::open_files,
          .title = "Open Many",
      });

  if (unsupported.has_value()) {
    return 10;
  }
  if (unsupported.error().code != cgpui::ErrorCode::unsupported_platform ||
      unsupported.error().message != "dialog missing") {
    return 11;
  }
  if (fixture.runtime.native_file_dialog_result().backend !=
          "dialog-supported" ||
      !fixture.runtime.native_file_dialog_result().accepted ||
      fixture.runtime.native_file_dialog_result().paths[0] !=
          "D:/Projects/project.cgpui") {
    return 12;
  }
  if (fixture.app.show_native_file_dialog_count != 2) {
    return 13;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result =
          test_unsupported_menu_result_does_not_overwrite_last_success();
      result != 0) {
    return result;
  }
  if (const int result = test_contexts_forward_platform_service_results();
      result != 0) {
    return result;
  }
  if (const int result =
          test_unsupported_dialog_result_does_not_overwrite_last_success();
      result != 0) {
    return result;
  }
  return 0;
}
