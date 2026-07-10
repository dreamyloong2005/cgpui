#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/ui.hpp"

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override { clear_count += 1; }
  void draw_rect(const cgpui::SolidRect&) override { draw_count += 1; }
  cgpui::Result<void> present() override {
    present_count += 1;
    return {};
  }

  int clear_count = 0;
  int draw_count = 0;
  int present_count = 0;
};

class RecordingRenderer final : public cgpui::Renderer {
 public:
  RecordingRenderer(
      RecordingFrame& frame,
      int& begin_frame_count,
      int* resize_count_out = nullptr)
      : frame_(frame),
        begin_frame_count_(begin_frame_count),
        resize_count_out_(resize_count_out) {}

  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    resize_count += 1;
    if (resize_count_out_ != nullptr) {
      *resize_count_out_ += 1;
    }
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    begin_frame_count_ += 1;
    return std::unique_ptr<cgpui::RenderFrame>(new BorrowedFrame(frame_));
  }

  int resize_count = 0;

 private:
  class BorrowedFrame final : public cgpui::RenderFrame {
   public:
    explicit BorrowedFrame(RecordingFrame& frame) : frame_(frame) {}

    void clear(cgpui::Color color) override { frame_.clear(color); }
    void draw_rect(const cgpui::SolidRect& rect) override {
      frame_.draw_rect(rect);
    }
    cgpui::Result<void> present() override { return frame_.present(); }

   private:
    RecordingFrame& frame_;
  };

  RecordingFrame& frame_;
  int& begin_frame_count_;
  int* resize_count_out_ = nullptr;
};

class TestView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    paint_list.fill_rect(
        cgpui::Rect{.origin = {}, .size = {.width = 8.0F, .height = 6.0F}},
        cgpui::Color{.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F});
  }

  int paint_count = 0;
};

class EventCountingView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    paint_list.fill_rect(
        cgpui::Rect{.origin = {}, .size = {.width = 6.0F, .height = 4.0F}},
        cgpui::Color{.r = 0.2F, .g = 0.3F, .b = 0.4F, .a = 1.0F});
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    event_count += 1;
    last_context_view_id = context.view_id;
    if (std::holds_alternative<cgpui::WindowFocused>(event)) {
      focus_count += 1;
    } else if (std::holds_alternative<cgpui::PointerMoved>(event)) {
      pointer_move_count += 1;
    } else if (std::holds_alternative<cgpui::KeyboardKey>(event)) {
      key_count += 1;
    }
    return cgpui::EventResult::consumed_event();
  }

  int paint_count = 0;
  int event_count = 0;
  int focus_count = 0;
  int pointer_move_count = 0;
  int key_count = 0;
  cgpui::ViewId last_context_view_id{};
};

struct AppSettings {
  int launch_count = 0;
};

struct MissingAppSettings {
  int value = 0;
};

class LifetimeView final : public cgpui::View {
 public:
  explicit LifetimeView(bool& destroyed) : destroyed_(destroyed) {}
  ~LifetimeView() override { destroyed_ = true; }

  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    paint_list.fill_rect(
        cgpui::Rect{.origin = {}, .size = {.width = 4.0F, .height = 4.0F}},
        cgpui::Color{.r = 0.4F, .g = 0.5F, .b = 0.6F, .a = 1.0F});
  }

  int paint_count = 0;

 private:
  bool& destroyed_;
};

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  explicit FakeWindow(cgpui::WindowState state) : state_(state) {}

  [[nodiscard]] cgpui::NativeSurfaceHandle native_surface() const override {
    return {};
  }

  [[nodiscard]] cgpui::WindowState state() const override {
    return state_;
  }

  [[nodiscard]] cgpui::PlatformWindowCloseState close_request_state()
      const override {
    return close_state_;
  }

  bool resolve_close_request(
      cgpui::PlatformWindowCloseResolution resolution) override {
    if (!close_state_.pending) {
      return false;
    }
    close_state_.pending = false;
    close_state_.accepted =
        resolution == cgpui::PlatformWindowCloseResolution::accept;
    if (close_state_.accepted) {
      close_state_.accepted_count += 1;
    } else {
      close_state_.cancelled_count += 1;
      state_.close_requested = false;
    }
    return true;
  }

  void request_redraw() override {
    request_redraw_count += 1;
    if (callback) {
      callback(cgpui::WindowRedrawRequested{});
    }
  }

  void request_close() override {
    request_close_count += 1;
    emit_close(cgpui::WindowCloseRequestSource::application);
  }

  void emit(const cgpui::PlatformEvent& event) {
    if (const auto* close = std::get_if<cgpui::WindowCloseRequested>(&event);
        close != nullptr) {
      emit_close(close->source);
      return;
    }
    if (callback) {
      callback(event);
    }
  }

  void set_title(std::string_view title) override {
    last_title = title;
  }

  void set_cursor(cgpui::CursorShape cursor_shape) override {
    last_cursor_shape = cursor_shape;
  }

  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

  cgpui::PlatformEventCallback callback;
  int request_redraw_count = 0;
  int request_close_count = 0;
  cgpui::CursorShape last_cursor_shape = cgpui::CursorShape::default_arrow;
  std::string_view last_title;

 private:
  void emit_close(cgpui::WindowCloseRequestSource source) {
    if (close_state_.pending || close_state_.accepted) {
      close_state_.coalesced_count += 1;
      return;
    }
    close_state_.pending = true;
    close_state_.source = source;
    close_state_.sequence += 1;
    state_.close_requested = true;
    if (callback) {
      callback(cgpui::WindowCloseRequested{
          .source = source,
          .sequence = close_state_.sequence});
    }
  }

  cgpui::WindowState state_;
  cgpui::PlatformWindowCloseState close_state_;
};

class FakeApplication final : public cgpui::PlatformApplication {
 public:
  explicit FakeApplication(FakeWindow& window) : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    last_descriptor = descriptor;
    created_descriptors.push_back(descriptor);
    if (!failing_window_title.empty() &&
        descriptor.title == failing_window_title) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::window_creation_failed,
          .message = "native child window unsupported"});
    }
    window_.callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(new BorrowedWindow(window_));
  }

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    last_descriptor = descriptor;
    created_descriptors.push_back(descriptor);
    if (!failing_window_title.empty() &&
        descriptor.title == failing_window_title) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::window_creation_failed,
          .message = "native child window unsupported"});
    }
    auto child = std::make_unique<FakeWindow>(cgpui::WindowState{
        .framebuffer_size = descriptor.size,
        .scale = cgpui::DpiScale{1.0F},
        .close_requested = false});
    child->callback = std::move(callback);
    FakeWindow* child_ptr = child.get();
    child_windows_.push_back(std::move(child));
    return std::unique_ptr<cgpui::PlatformWindow>(
        new BorrowedWindow(*child_ptr));
  }

  int run() override {
    run_count += 1;
    if (window_.callback) {
      window_.callback(cgpui::WindowRedrawRequested{});
    }
    if (on_run) {
      on_run(*this);
    }
    return run_result;
  }

  void quit() override { quit_count += 1; }

  void request_wakeup() override { request_wakeup_count += 1; }

  cgpui::PlatformMenuInstallationResult install_native_menu(
      cgpui::NativeMenuModel menu) override {
    install_native_menu_count += 1;
    last_menu_model = std::move(menu);
    return cgpui::PlatformMenuInstallationResult{
        .supported = true,
        .backend = "fake",
        .menu_count = last_menu_model.items.size(),
        .item_count = cgpui::native_menu_item_count(last_menu_model),
        .accelerator_count =
            cgpui::native_menu_accelerator_count(last_menu_model),
    };
  }

  cgpui::NativeFileDialogResult show_native_file_dialog(
      cgpui::NativeFileDialogOptions options) override {
    show_native_file_dialog_count += 1;
    last_file_dialog_options = std::move(options);
    return cgpui::NativeFileDialogResult{
        .supported = true,
        .accepted = true,
        .backend = "fake",
        .kind = last_file_dialog_options.kind,
        .paths = {last_file_dialog_options.default_directory +
                  "/project.cgpui"},
        .filter_count = last_file_dialog_options.filters.size(),
    };
  }

  int create_window_count = 0;
  int run_count = 0;
  int quit_count = 0;
  int request_wakeup_count = 0;
  int install_native_menu_count = 0;
  int show_native_file_dialog_count = 0;
  int run_result = 0;
  cgpui::WindowDescriptor last_descriptor{};
  std::vector<cgpui::WindowDescriptor> created_descriptors;
  cgpui::NativeMenuModel last_menu_model;
  cgpui::NativeFileDialogOptions last_file_dialog_options;
  std::string failing_window_title;
  std::function<void(FakeApplication&)> on_run;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(FakeWindow& window) : window_(window) {}

    [[nodiscard]] cgpui::NativeSurfaceHandle native_surface()
        const override {
      return window_.native_surface();
    }

    [[nodiscard]] cgpui::WindowState state() const override {
      return window_.state();
    }

    [[nodiscard]] cgpui::PlatformWindowCloseState close_request_state()
        const override {
      return window_.close_request_state();
    }

    bool resolve_close_request(
        cgpui::PlatformWindowCloseResolution resolution) override {
      return window_.resolve_close_request(resolution);
    }

    void request_redraw() override { window_.request_redraw(); }
    void request_close() override { window_.request_close(); }
    void set_title(std::string_view title) override {
      window_.set_title(title);
    }
    void set_cursor(cgpui::CursorShape cursor_shape) override {
      window_.set_cursor(cursor_shape);
    }
    void set_ime_text_input_placement(
        std::optional<cgpui::ImeTextInputPlacement> placement) override {
      window_.set_ime_text_input_placement(placement);
    }

   private:
    FakeWindow& window_;
  };

  FakeWindow& window_;
  std::vector<std::unique_ptr<FakeWindow>> child_windows_;
};

class MultiWindowFakeApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    created_descriptors.push_back(descriptor);
    auto window = std::make_unique<FakeWindow>(cgpui::WindowState{
        .framebuffer_size = descriptor.size,
        .scale = cgpui::DpiScale{1.0F},
        .close_requested = false});
    window->callback = std::move(callback);
    FakeWindow* window_ptr = window.get();
    created_windows.push_back(std::move(window));
    return std::unique_ptr<cgpui::PlatformWindow>(
        new BorrowedWindow(*window_ptr));
  }

  int run() override {
    run_count += 1;
    if (!created_windows.empty()) {
      created_windows.front()->emit(cgpui::WindowRedrawRequested{});
    }
    if (on_run) {
      on_run(*this);
    }
    return run_result;
  }

  void quit() override { quit_count += 1; }

  FakeWindow* window_for_title(std::string_view title) {
    for (std::size_t index = 0; index < created_descriptors.size(); ++index) {
      if (created_descriptors[index].title == title) {
        return created_windows[index].get();
      }
    }
    return nullptr;
  }

  int create_window_count = 0;
  int run_count = 0;
  int quit_count = 0;
  int run_result = 0;
  std::vector<cgpui::WindowDescriptor> created_descriptors;
  std::vector<std::unique_ptr<FakeWindow>> created_windows;
  std::function<void(MultiWindowFakeApplication&)> on_run;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(FakeWindow& window) : window_(window) {}

    [[nodiscard]] cgpui::NativeSurfaceHandle native_surface()
        const override {
      return window_.native_surface();
    }

    [[nodiscard]] cgpui::WindowState state() const override {
      return window_.state();
    }

    [[nodiscard]] cgpui::PlatformWindowCloseState close_request_state()
        const override {
      return window_.close_request_state();
    }

    bool resolve_close_request(
        cgpui::PlatformWindowCloseResolution resolution) override {
      return window_.resolve_close_request(resolution);
    }

    void request_redraw() override { window_.request_redraw(); }
    void request_close() override { window_.request_close(); }
    void set_title(std::string_view title) override {
      window_.set_title(title);
    }
    void set_cursor(cgpui::CursorShape cursor_shape) override {
      window_.set_cursor(cursor_shape);
    }
    void set_ime_text_input_placement(
        std::optional<cgpui::ImeTextInputPlacement> placement) override {
      window_.set_ime_text_input_placement(placement);
    }

   private:
    FakeWindow& window_;
  };
};

int test_run_app_builds_runtime_and_runs_window() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{2.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  int renderer_factory_count = 0;
  bool setup_called = false;
  bool after_frame_called = false;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        renderer_factory_count += 1;
        if (descriptor.framebuffer_size.width != 320.0F ||
            descriptor.framebuffer_size.height != 240.0F ||
            descriptor.scale.value != 2.0F) {
          return std::unexpected(cgpui::Error{
              .code = cgpui::ErrorCode::renderer_initialization_failed,
              .message = "bad runner descriptor"});
        }
        auto owned =
            std::make_unique<RecordingRenderer>(frame, renderer_begin_frame_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .window =
              {
                  .title = "Runner Test",
                  .size = {.width = 800.0F, .height = 600.0F},
              },
          .runtime = {.request_initial_redraw = false},
          .setup =
              [&](cgpui::WindowRuntime& runtime) {
                setup_called = true;
                runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext&) {
                      after_frame_called = true;
                    });
              },
      });

  if (result != 0) {
    return 1;
  }
  if (application.create_window_count != 1 || application.run_count != 1) {
    return 2;
  }
  if (application.last_descriptor.title != "Runner Test" ||
      application.last_descriptor.size.width != 800.0F ||
      application.last_descriptor.size.height != 600.0F) {
    return 3;
  }
  if (renderer_factory_count != 1) {
    return 4;
  }
  if (!setup_called || !after_frame_called) {
    return 5;
  }
  if (window.request_redraw_count != 0) {
    return 6;
  }
  if (renderer_begin_frame_count != 1) {
    return 7;
  }
  if (frame.present_count != 1) {
    return 8;
  }
  if (view.paint_count != 1) {
    return 9;
  }
  return 0;
}

int test_run_app_accepts_app_context_setup_callback() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  bool setup_called = false;
  bool after_frame_called = false;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        auto owned =
            std::make_unique<RecordingRenderer>(frame, renderer_begin_frame_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext&) {
                      after_frame_called = true;
                    });
              },
      });

  if (result != 0) {
    return 10;
  }
  if (!setup_called || !after_frame_called) {
    return 11;
  }
  if (renderer_begin_frame_count != 1 || frame.present_count != 1) {
    return 12;
  }
  return 0;
}

int test_window_options_and_app_context_open_window_skeleton() {
  const cgpui::WindowDescriptor descriptor =
      cgpui::WindowOptions{}
          .title("Secondary Window")
          .size(640.0F, 480.0F)
          .to_descriptor();
  if (descriptor.title != "Secondary Window" ||
      descriptor.size.width != 640.0F ||
      descriptor.size.height != 480.0F) {
    return 13;
  }
  const cgpui::WindowDescriptor chrome_descriptor =
      cgpui::WindowOptions{}
          .title("Palette")
          .size(400.0F, 320.0F)
          .titlebar_visible(false)
          .decorations(false)
          .resizable(false)
          .transparent(true)
          .to_descriptor();
  if (chrome_descriptor.chrome.titlebar_visible ||
      chrome_descriptor.chrome.decorations ||
      chrome_descriptor.chrome.resizable ||
      !chrome_descriptor.chrome.transparent_background) {
    return 20;
  }

  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  bool setup_called = false;
  bool after_frame_called = false;
  cgpui::WindowDescriptor opened_descriptor;
  cgpui::WindowDescriptor recorded_descriptor;
  cgpui::WindowDescriptor chrome_opened_descriptor;
  cgpui::WindowDescriptor chrome_recorded_descriptor;
  std::size_t recorded_count = 0;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        auto owned =
            std::make_unique<RecordingRenderer>(frame, renderer_begin_frame_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                const cgpui::AppOpenedWindow opened =
                    context.open_window(cgpui::WindowOptions{}
                                            .title("Secondary Window")
                                            .size(cgpui::Size{
                                                .width = 640.0F,
                                                .height = 480.0F}));
                const cgpui::AppOpenedWindow chrome_opened =
                    context.open_window(cgpui::WindowOptions{}
                                            .title("Palette")
                                            .size(400.0F, 320.0F)
                                            .titlebar_visible(false)
                                            .decorations(false)
                                            .resizable(false)
                                            .transparent(true));
                opened_descriptor = opened.descriptor;
                chrome_opened_descriptor = chrome_opened.descriptor;
                const auto opened_windows =
                    context.runtime.app_opened_windows();
                recorded_count = opened_windows.size();
                if (!opened_windows.empty()) {
                  recorded_descriptor = opened_windows.front().descriptor;
                }
                if (opened_windows.size() > 1) {
                  chrome_recorded_descriptor = opened_windows[1].descriptor;
                }
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext&) {
                      after_frame_called = true;
                    });
              },
      });

  if (result != 0) {
    return 14;
  }
  if (!setup_called || !after_frame_called) {
    return 15;
  }
  if (recorded_count != 2) {
    return 16;
  }
  if (opened_descriptor.title != "Secondary Window" ||
      opened_descriptor.size.width != 640.0F ||
      opened_descriptor.size.height != 480.0F) {
    return 17;
  }
  if (recorded_descriptor.title != opened_descriptor.title ||
      recorded_descriptor.size.width != opened_descriptor.size.width ||
      recorded_descriptor.size.height != opened_descriptor.size.height) {
    return 18;
  }
  if (chrome_opened_descriptor.chrome.titlebar_visible ||
      chrome_opened_descriptor.chrome.decorations ||
      chrome_opened_descriptor.chrome.resizable ||
      !chrome_opened_descriptor.chrome.transparent_background ||
      chrome_recorded_descriptor.chrome.titlebar_visible ||
      chrome_recorded_descriptor.chrome.decorations ||
      chrome_recorded_descriptor.chrome.resizable ||
      !chrome_recorded_descriptor.chrome.transparent_background) {
    return 21;
  }
  if (application.create_window_count != 3 ||
      application.created_descriptors.size() != 3 ||
      application.created_descriptors[0].title != "CGPUI" ||
      application.created_descriptors[1].title != "Secondary Window" ||
      application.created_descriptors[2].title != "Palette") {
    return 19;
  }
  return 0;
}

int test_app_context_global_state_helpers() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  bool setup_called = false;
  bool after_frame_called = false;
  bool set_global = false;
  bool updated_global = false;
  bool missing_global_update = true;
  int setup_read_value = -1;
  int after_frame_read_value = -1;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        auto owned =
            std::make_unique<RecordingRenderer>(frame, renderer_begin_frame_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                context.set_global(AppSettings{.launch_count = 1});
                set_global = context.global<AppSettings>() != nullptr;
                updated_global =
                    context.update_global<AppSettings>(
                        [](AppSettings& settings) {
                          settings.launch_count += 4;
                        });
                const AppSettings* settings =
                    context.global<AppSettings>();
                setup_read_value =
                    settings == nullptr ? -1 : settings->launch_count;
                missing_global_update =
                    context.update_global<MissingAppSettings>(
                        [](MissingAppSettings& settings) {
                          settings.value = 42;
                        });
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext& frame_context) {
                      after_frame_called = true;
                      const AppSettings* after_frame_settings =
                          frame_context.global<AppSettings>();
                      after_frame_read_value =
                          after_frame_settings == nullptr
                              ? -1
                              : after_frame_settings->launch_count;
                    });
              },
      });

  if (result != 0) {
    return 27;
  }
  if (!setup_called || !after_frame_called || !set_global) {
    return 28;
  }
  if (!updated_global || missing_global_update) {
    return 29;
  }
  if (setup_read_value != 5 || after_frame_read_value != 5) {
    return 30;
  }
  if (application.create_window_count != 1 ||
      renderer_begin_frame_count != 1 ||
      frame.present_count != 1) {
    return 31;
  }
  return 0;
}

int test_app_opened_window_keeps_root_view_alive_until_run_app_returns() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  bool setup_called = false;
  bool after_frame_called = false;
  bool root_destroyed = false;
  bool root_destroyed_during_setup = false;
  bool root_destroyed_during_frame = false;
  bool root_view_stored_during_setup = false;
  bool root_view_stored_during_frame = false;
  bool root_view_id_allocated = false;
  cgpui::ViewId opened_root_view_id;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        auto owned =
            std::make_unique<RecordingRenderer>(frame, renderer_begin_frame_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                auto root_view = std::make_unique<LifetimeView>(root_destroyed);
                const cgpui::View* root_view_ptr = root_view.get();
                const cgpui::AppOpenedWindow opened =
                    context.open_window(
                        cgpui::WindowOptions{}
                            .title("Owned Root")
                            .size(320.0F, 200.0F),
                        std::move(root_view));
                opened_root_view_id = opened.root_view_id;
                root_view_id_allocated =
                    context.runtime.is_view_id_allocated(opened.root_view_id);
                root_view_stored_during_setup =
                    context.runtime.app_opened_window_root_view(
                        opened.root_view_id) == root_view_ptr;
                root_destroyed_during_setup = root_destroyed;
                context.runtime.set_after_frame_callback(
                    [&, root_view_ptr](
                        const cgpui::ViewContext& frame_context) {
                      after_frame_called = true;
                      root_view_stored_during_frame =
                          frame_context.runtime.app_opened_window_root_view(
                              opened_root_view_id) == root_view_ptr;
                      root_destroyed_during_frame = root_destroyed;
                    });
              },
      });

  if (result != 0) {
    return 20;
  }
  if (!setup_called || !after_frame_called) {
    return 21;
  }
  if (opened_root_view_id.value == 0 || !root_view_id_allocated) {
    return 22;
  }
  if (!root_view_stored_during_setup || !root_view_stored_during_frame) {
    return 23;
  }
  if (root_destroyed_during_setup || root_destroyed_during_frame) {
    return 24;
  }
  if (!root_destroyed) {
    return 25;
  }
  if (application.create_window_count != 2) {
    return 26;
  }
  return 0;
}

int test_multi_window_registry_owns_independent_runtime_records() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  int renderer_factory_count = 0;
  bool setup_called = false;
  bool after_frame_called = false;
  bool first_destroyed = false;
  bool second_destroyed = false;
  bool setup_records_ok = false;
  bool frame_root_record_ok = false;
  bool frame_secondary_records_ok = false;
  bool setup_children_pending = false;
  bool frame_child_renderers_distinct = false;
  const cgpui::View* first_root_ptr = nullptr;
  const cgpui::View* second_root_ptr = nullptr;
  cgpui::AppOpenedWindow first_opened;
  cgpui::AppOpenedWindow second_opened;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        renderer_factory_count += 1;
        auto owned =
            std::make_unique<RecordingRenderer>(frame, renderer_begin_frame_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                auto first_root =
                    std::make_unique<LifetimeView>(first_destroyed);
                auto second_root =
                    std::make_unique<LifetimeView>(second_destroyed);
                first_root_ptr = first_root.get();
                second_root_ptr = second_root.get();

                first_opened = context.open_window(
                    cgpui::WindowOptions{}
                        .title("First Child")
                        .size(320.0F, 200.0F),
                    std::move(first_root));
                second_opened = context.open_window(
                    cgpui::WindowOptions{}
                        .title("Second Child")
                        .size(420.0F, 260.0F),
                    std::move(second_root));

                const cgpui::WindowRuntimeRecord* first_record =
                    context.runtime.window_runtime_record(
                        first_opened.runtime_id);
                const cgpui::WindowRuntimeRecord* second_record =
                    context.runtime.window_runtime_record(
                        second_opened.runtime_id);
                const auto records = context.runtime.window_runtime_records();
                setup_records_ok =
                    records.size() == 3 &&
                    first_opened.runtime_id.value != 0 &&
                    second_opened.runtime_id.value != 0 &&
                    first_opened.runtime_id != second_opened.runtime_id &&
                    first_opened.root_view_id.value != 0 &&
                    second_opened.root_view_id.value != 0 &&
                    first_opened.root_view_id != second_opened.root_view_id &&
                    first_record != nullptr && second_record != nullptr &&
                    first_record->descriptor.title == "First Child" &&
                    second_record->descriptor.title == "Second Child" &&
                    first_record->root_view_id == first_opened.root_view_id &&
                    second_record->root_view_id == second_opened.root_view_id &&
                    first_record->owns_root_view &&
                    second_record->owns_root_view &&
                    first_record->window == nullptr &&
                    second_record->window == nullptr &&
                    first_record->renderer == nullptr &&
                    second_record->renderer == nullptr &&
                    first_record->owns_window &&
                    second_record->owns_window &&
                    first_record->owns_renderer &&
                    second_record->owns_renderer &&
                    !first_record->active &&
                    !second_record->active &&
                    first_record->parent_runtime_id ==
                        context.runtime.root_window_runtime_id() &&
                    second_record->parent_runtime_id ==
                        context.runtime.root_window_runtime_id() &&
                    !first_record->native_window_error.has_value() &&
                    !second_record->native_window_error.has_value() &&
                    context.runtime.app_opened_window_root_view(
                        first_opened.root_view_id) == first_root_ptr &&
                    context.runtime.app_opened_window_root_view(
                        second_opened.root_view_id) == second_root_ptr;
                setup_children_pending =
                    first_record != nullptr && second_record != nullptr &&
                    first_record->renderer == nullptr &&
                    second_record->renderer == nullptr;

                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext& frame_context) {
                      after_frame_called = true;
                      const cgpui::WindowRuntimeRecord* root_record =
                          frame_context.runtime.window_runtime_record(
                              frame_context.runtime.root_window_runtime_id());
                      const cgpui::WindowRuntimeRecord* first_frame_record =
                          frame_context.runtime.window_runtime_record(
                              first_opened.runtime_id);
                      const cgpui::WindowRuntimeRecord* second_frame_record =
                          frame_context.runtime.window_runtime_record(
                              second_opened.runtime_id);
                      frame_root_record_ok =
                          root_record != nullptr &&
                          root_record->active &&
                          root_record->root_view_id == frame_context.view_id &&
                          root_record->window ==
                              &frame_context.platform_window &&
                          root_record->renderer == &frame_context.renderer &&
                          root_record->owns_window &&
                          !root_record->owns_renderer &&
                          !root_record->owns_root_view;
                      frame_secondary_records_ok =
                          first_frame_record != nullptr &&
                          second_frame_record != nullptr &&
                          first_frame_record->root_view_id ==
                              first_opened.root_view_id &&
                          second_frame_record->root_view_id ==
                              second_opened.root_view_id &&
                          first_frame_record->window != nullptr &&
                          second_frame_record->window != nullptr &&
                          first_frame_record->renderer != nullptr &&
                          second_frame_record->renderer != nullptr &&
                          first_frame_record->owns_window &&
                          second_frame_record->owns_window &&
                          first_frame_record->owns_renderer &&
                          second_frame_record->owns_renderer &&
                          first_frame_record->active &&
                          second_frame_record->active &&
                          !first_frame_record->native_window_error.has_value() &&
                          !second_frame_record->native_window_error.has_value();
                      frame_child_renderers_distinct =
                          root_record != nullptr && first_frame_record != nullptr &&
                          second_frame_record != nullptr &&
                          first_frame_record->renderer != nullptr &&
                          second_frame_record->renderer != nullptr &&
                          first_frame_record->renderer !=
                              second_frame_record->renderer &&
                          first_frame_record->renderer != root_record->renderer &&
                          second_frame_record->renderer != root_record->renderer;
                    });
              },
      });

  if (result != 0) {
    return 32;
  }
  if (!setup_called || !after_frame_called) {
    return 33;
  }
  if (!setup_records_ok || !frame_root_record_ok ||
      !frame_secondary_records_ok) {
    return 34;
  }
  if (!setup_children_pending || !frame_child_renderers_distinct) {
    return 43;
  }
  if (renderer_factory_count != 3 || renderer_begin_frame_count != 1 ||
      frame.present_count != 1) {
    return 35;
  }
  if (!first_destroyed || !second_destroyed) {
    return 36;
  }
  if (application.create_window_count != 3 ||
      application.created_descriptors.size() != 3 ||
      application.created_descriptors[0].title != "CGPUI" ||
      application.created_descriptors[1].title != "First Child" ||
      application.created_descriptors[2].title != "Second Child") {
    return 37;
  }
  return 0;
}

int test_app_opened_window_routes_native_events_by_runtime_id() {
  MultiWindowFakeApplication application;
  EventCountingView root_view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  int renderer_factory_count = 0;
  int child_renderer_resize_count = 0;
  bool setup_called = false;
  bool on_run_called = false;
  bool child_window_found = false;
  bool child_resize_routed = false;
  bool child_close_routed = false;
  bool child_dispatch_records_routed = false;
  cgpui::WindowRuntime* runtime = nullptr;
  cgpui::AppOpenedWindow opened;
  EventCountingView* child_view = nullptr;
  int child_event_dispatch_count = 0;
  int observed_root_event_count = -1;
  int observed_child_event_count = -1;
  int observed_child_focus_count = -1;
  int observed_child_pointer_move_count = -1;
  int observed_child_key_count = -1;
  cgpui::ViewId observed_child_context_view_id{};
  int observed_child_paint_count = -1;

  const int result = cgpui::run_app(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        int* resize_counter =
            renderer_factory_count == 1 ? &child_renderer_resize_count
                                        : nullptr;
        renderer_factory_count += 1;
        auto owned = std::make_unique<RecordingRenderer>(
            frame,
            renderer_begin_frame_count,
            resize_counter);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                runtime = &context.runtime;
                auto child = std::make_unique<EventCountingView>();
                child_view = child.get();
                opened = context.open_window(
                    cgpui::WindowOptions{}
                        .title("Routed Child")
                        .size(320.0F, 200.0F),
                    std::move(child));
                context.runtime.set_after_event_callback(
                    [&](const cgpui::WindowRuntimeContext& event_context,
                        const cgpui::EventDispatchRecord& record) {
                      if (record.view_id == opened.root_view_id &&
                          event_context.view_id == opened.root_view_id &&
                          (record.event_kind ==
                               cgpui::EventKind::window_focused ||
                           record.event_kind ==
                               cgpui::EventKind::pointer_moved ||
                           record.event_kind ==
                               cgpui::EventKind::keyboard_key)) {
                        child_event_dispatch_count += 1;
                      }
                    });
                application.on_run =
                    [&](MultiWindowFakeApplication& running_application) {
                      on_run_called = true;
                      FakeWindow* routed_child =
                          running_application.window_for_title("Routed Child");
                      child_window_found = routed_child != nullptr;
                      if (routed_child == nullptr || runtime == nullptr) {
                        return;
                      }

                      routed_child->emit(cgpui::WindowFocused{.focused = true});
                      routed_child->emit(cgpui::PointerMoved{
                          .position = {.x = 24.0F, .y = 12.0F}});
                      routed_child->emit(cgpui::KeyboardKey{
                          .key_code = 65,
                          .action = cgpui::KeyAction::pressed});
                      routed_child->emit(cgpui::WindowResized{
                          .size = {.width = 640.0F, .height = 480.0F},
                          .scale = cgpui::DpiScale{2.0F}});
                      child_resize_routed = child_renderer_resize_count == 1;
                      routed_child->emit(cgpui::WindowRedrawRequested{});
                      observed_root_event_count = root_view.event_count;
                      if (child_view != nullptr) {
                        observed_child_event_count = child_view->event_count;
                        observed_child_focus_count = child_view->focus_count;
                        observed_child_pointer_move_count =
                            child_view->pointer_move_count;
                        observed_child_key_count = child_view->key_count;
                        observed_child_context_view_id =
                            child_view->last_context_view_id;
                        observed_child_paint_count = child_view->paint_count;
                      }
                      routed_child->emit(cgpui::WindowCloseRequested{});
                      const cgpui::WindowRuntimeRecord* record =
                          runtime->window_runtime_record(opened.runtime_id);
                      child_close_routed =
                          record != nullptr && !record->active &&
                          running_application.quit_count == 0;
                    };
              },
      });

  child_dispatch_records_routed = child_event_dispatch_count == 3;

  if (result != 0) {
    return 44;
  }
  if (!setup_called || !on_run_called || !child_window_found ||
      runtime == nullptr || child_view == nullptr) {
    return 45;
  }
  if (observed_root_event_count != 0) {
    return 46;
  }
  if (observed_child_event_count != 3) {
    return 49;
  }
  if (observed_child_focus_count != 1 ||
      observed_child_pointer_move_count != 1 || observed_child_key_count != 1) {
    return 50;
  }
  if (observed_child_context_view_id != opened.root_view_id) {
    return 51;
  }
  if (!child_dispatch_records_routed || !child_resize_routed ||
      !child_close_routed) {
    return 47;
  }
  if (renderer_factory_count != 2 || renderer_begin_frame_count != 2 ||
      observed_child_paint_count != 1 || frame.present_count != 2) {
    return 48;
  }
  return 0;
}

int test_app_opened_window_close_cleans_runtime_owned_state() {
  MultiWindowFakeApplication application;
  TestView root_view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  int renderer_factory_count = 0;
  bool child_destroyed = false;
  bool setup_called = false;
  bool on_run_called = false;
  bool child_window_found = false;
  bool subscribed_before_close = false;
  bool destroyed_after_close = false;
  bool view_removed_after_close = false;
  bool subscriptions_removed_after_close = false;
  bool record_released_after_close = false;
  bool child_close_did_not_quit_app = false;
  cgpui::AppOpenedWindow opened;

  const int result = cgpui::run_app(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        renderer_factory_count += 1;
        auto owned =
            std::make_unique<RecordingRenderer>(frame, renderer_begin_frame_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                auto child =
                    std::make_unique<LifetimeView>(child_destroyed);
                opened = context.open_window(
                    cgpui::WindowOptions{}
                        .title("Lifecycle Child")
                        .size(320.0F, 200.0F),
                    std::move(child));
                const cgpui::Model<AppSettings> settings =
                    context.runtime.emplace_entity<AppSettings>();
                context.runtime.subscribe_view_to_entity(
                    opened.root_view_id,
                    settings);
                subscribed_before_close =
                    context.runtime.subscriptions_for_view(opened.root_view_id)
                        .size() == 1;

                application.on_run =
                    [&](MultiWindowFakeApplication& running_application) {
                      on_run_called = true;
                      FakeWindow* lifecycle_child =
                          running_application.window_for_title(
                              "Lifecycle Child");
                      child_window_found = lifecycle_child != nullptr;
                      if (lifecycle_child == nullptr) {
                        return;
                      }

                      lifecycle_child->emit(cgpui::WindowCloseRequested{});

                      const cgpui::WindowRuntimeRecord* record =
                          context.runtime.window_runtime_record(
                              opened.runtime_id);
                      destroyed_after_close = child_destroyed;
                      view_removed_after_close =
                          context.runtime.find_view(opened.root_view_id) ==
                              nullptr &&
                          context.runtime.app_opened_window_root_view(
                              opened.root_view_id) == nullptr;
                      subscriptions_removed_after_close =
                          context.runtime
                              .subscriptions_for_view(opened.root_view_id)
                              .empty();
                      record_released_after_close =
                          record != nullptr && record->window == nullptr &&
                          record->renderer == nullptr && !record->active &&
                          !record->owns_window && !record->owns_renderer &&
                          !record->owns_root_view;
                      child_close_did_not_quit_app =
                          running_application.quit_count == 0;
                    };
              },
      });

  if (result != 0) {
    return 52;
  }
  if (!setup_called || !on_run_called || !child_window_found) {
    return 53;
  }
  if (!subscribed_before_close) {
    return 54;
  }
  if (!destroyed_after_close || !view_removed_after_close) {
    return 55;
  }
  if (!subscriptions_removed_after_close || !record_released_after_close ||
      !child_close_did_not_quit_app) {
    return 56;
  }
  if (!child_destroyed) {
    return 57;
  }
  if (renderer_factory_count != 2) {
    return 58;
  }
  return 0;
}

int test_app_opened_window_records_native_creation_error() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  application.failing_window_title = "Unsupported Child";
  TestView view;
  RecordingFrame frame;
  int renderer_begin_frame_count = 0;
  bool setup_called = false;
  bool after_frame_called = false;
  bool setup_error_record_ok = false;
  bool frame_error_record_ok = false;
  cgpui::AppOpenedWindow opened;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        auto owned =
            std::make_unique<RecordingRenderer>(frame, renderer_begin_frame_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                opened = context.open_window(
                    cgpui::WindowOptions{}
                        .title("Unsupported Child")
                        .size(200.0F, 120.0F));
                const cgpui::WindowRuntimeRecord* record =
                    context.runtime.window_runtime_record(opened.runtime_id);
                setup_error_record_ok =
                    record != nullptr && record->window == nullptr &&
                    !record->active &&
                    !record->native_window_error.has_value() &&
                    record->parent_runtime_id ==
                        context.runtime.root_window_runtime_id();
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext& frame_context) {
                      after_frame_called = true;
                      const cgpui::WindowRuntimeRecord* record =
                          frame_context.runtime.window_runtime_record(
                              opened.runtime_id);
                      frame_error_record_ok =
                          record != nullptr && record->window == nullptr &&
                          !record->active &&
                          record->native_window_error.has_value() &&
                          record->native_window_error->message ==
                              "native child window unsupported";
                    });
              },
      });

  if (result != 0) {
    return 38;
  }
  if (!setup_called || !after_frame_called) {
    return 39;
  }
  if (!setup_error_record_ok || !frame_error_record_ok) {
    return 40;
  }
  if (application.create_window_count != 2 ||
      application.created_descriptors.size() != 2 ||
      application.created_descriptors[0].title != "CGPUI" ||
      application.created_descriptors[1].title != "Unsupported Child") {
    return 41;
  }
  if (renderer_begin_frame_count != 1 || frame.present_count != 1) {
    return 42;
  }
  return 0;
}

int test_app_context_installs_native_menu_and_accelerators() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  bool setup_called = false;
  bool setup_install_ok = false;
  bool frame_install_ok = false;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        auto owned = std::make_unique<RecordingRenderer>(
            frame,
            application.run_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                cgpui::NativeMenuModel menu{
                    .items =
                        {
                            cgpui::NativeMenuItem{
                                .kind = cgpui::NativeMenuItemKind::submenu,
                                .title = "File",
                                .children =
                                    {
                                        cgpui::NativeMenuItem{
                                            .kind =
                                                cgpui::NativeMenuItemKind::
                                                    command,
                                            .title = "Open",
                                            .action_name = "file.open",
                                            .accelerator =
                                                cgpui::NativeMenuAccelerator{
                                                    .key_code = 'O',
                                                    .modifiers =
                                                        cgpui::
                                                            KeyboardModifiers{
                                                                .control =
                                                                    true},
                                                },
                                        },
                                        cgpui::NativeMenuItem{
                                            .kind =
                                                cgpui::NativeMenuItemKind::
                                                    separator,
                                        },
                                        cgpui::NativeMenuItem{
                                            .kind =
                                                cgpui::NativeMenuItemKind::
                                                    command,
                                            .title = "Quit",
                                            .action_name = "app.quit",
                                            .enabled = false,
                                        },
                                    },
                            },
                        },
                };

                const cgpui::NativeMenuInstallation installation =
                    context.install_native_menu(std::move(menu));
                setup_install_ok =
                    installation.platform.supported &&
                    installation.platform.backend == "fake" &&
                    installation.platform.menu_count == 1 &&
                    installation.platform.item_count == 4 &&
                    installation.platform.accelerator_count == 1 &&
                    context.runtime.native_menu_installation()
                        .platform.accelerator_count == 1 &&
                    application.install_native_menu_count == 1 &&
                    application.last_menu_model.items.size() == 1 &&
                    application.last_menu_model.items[0].children.size() == 3;
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext& frame_context) {
                      const cgpui::NativeMenuInstallation& frame_installation =
                          frame_context.native_menu_installation();
                      frame_install_ok =
                          frame_installation.model.items.size() == 1 &&
                          frame_installation.platform.item_count == 4 &&
                          frame_installation.platform.accelerator_count == 1;
                    });
              },
      });

  if (result != 0) {
    return 900;
  }
  if (!setup_called || !setup_install_ok || !frame_install_ok) {
    return 901;
  }

  return 0;
}

int test_app_context_shows_native_file_dialog_skeleton() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  bool setup_called = false;
  bool setup_dialog_ok = false;
  bool frame_dialog_ok = false;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        auto owned = std::make_unique<RecordingRenderer>(
            frame,
            application.run_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                cgpui::NativeFileDialogOptions options{
                    .kind = cgpui::NativeFileDialogKind::open_file,
                    .title = "Open Project",
                    .default_directory = "D:/Projects",
                    .suggested_name = "project.cgpui",
                    .filters =
                        {
                            cgpui::NativeFileDialogFilter{
                                .name = "CGPUI Project",
                                .extensions = {"cgpui"},
                            },
                            cgpui::NativeFileDialogFilter{
                                .name = "Text",
                                .extensions = {"txt", "md"},
                            },
                        },
                };

                const cgpui::NativeFileDialogResult dialog_result =
                    context.show_native_file_dialog(std::move(options));
                setup_dialog_ok =
                    dialog_result.supported && dialog_result.accepted &&
                    dialog_result.backend == "fake" &&
                    dialog_result.kind ==
                        cgpui::NativeFileDialogKind::open_file &&
                    dialog_result.filter_count == 2 &&
                    dialog_result.paths.size() == 1 &&
                    dialog_result.paths[0] == "D:/Projects/project.cgpui" &&
                    context.runtime.native_file_dialog_result().paths.size() ==
                        1 &&
                    application.show_native_file_dialog_count == 1 &&
                    application.last_file_dialog_options.filters.size() == 2 &&
                    application.last_file_dialog_options.suggested_name ==
                        "project.cgpui";
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext& frame_context) {
                      const cgpui::NativeFileDialogResult& frame_dialog =
                          frame_context.native_file_dialog_result();
                      frame_dialog_ok =
                          frame_dialog.backend == "fake" &&
                          frame_dialog.filter_count == 2 &&
                          frame_dialog.paths.size() == 1;
                    });
              },
      });

  if (result != 0) {
    return 910;
  }
  if (!setup_called || !setup_dialog_ok || !frame_dialog_ok) {
    return 911;
  }

  return 0;
}

int test_app_context_registers_command_palette_entries() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  bool setup_called = false;
  bool setup_palette_ok = false;
  bool frame_palette_ok = false;
  int dispatch_count = 0;

  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        auto owned = std::make_unique<RecordingRenderer>(
            frame,
            application.run_count);
        return owned;
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup_context =
              [&](cgpui::AppContext& context) {
                setup_called = true;
                context.runtime.register_app_action(
                    "app.palette.open",
                    [&](const cgpui::ViewContext&) {
                      dispatch_count += 1;
                      return cgpui::EventResult::consumed_event();
                    });
                context.register_command_palette_entry(
                    cgpui::CommandPaletteEntry{
                        .action_name = "app.palette.open",
                        .title = "Open Project",
                        .group = "File",
                        .scope = cgpui::ActionScope::app,
                    });
                context.register_command_palette_entry(
                    cgpui::CommandPaletteEntry{
                        .action_name = "app.palette.disabled",
                        .title = "Disabled",
                        .group = "File",
                        .scope = cgpui::ActionScope::app,
                        .enabled = false,
                    });

                const std::vector<cgpui::CommandPaletteEntry> file_entries =
                    context.command_palette_entries_for_group("File");
                setup_palette_ok =
                    context.command_palette_entries().size() == 2 &&
                    file_entries.size() == 2 &&
                    file_entries[0].action_name == "app.palette.open";
                context.runtime.set_after_frame_callback(
                    [&](const cgpui::ViewContext& frame_context) {
                      const std::vector<cgpui::CommandPaletteEntry>
                          frame_file_entries =
                              frame_context.command_palette_entries_for_group(
                                  "File");
                      const cgpui::ActionDispatchResult dispatch =
                          frame_context.dispatch_command_palette_action(
                              "app.palette.open");
                      const cgpui::ActionDispatchResult disabled_dispatch =
                          frame_context.dispatch_command_palette_action(
                              "app.palette.disabled");
                      frame_palette_ok =
                          frame_context.command_palette_entries().size() == 2 &&
                          frame_file_entries.size() == 2 && dispatch.handled &&
                          dispatch.result.consumed &&
                          !disabled_dispatch.handled &&
                          disabled_dispatch.name == "app.palette.disabled";
                    });
              },
      });

  if (result != 0) {
    return 920;
  }
  if (!setup_called || !setup_palette_ok || !frame_palette_ok ||
      dispatch_count != 1) {
    return 921;
  }

  return 0;
}

int test_runtime_close_callback_can_cancel_then_accept() {
  FakeWindow window(cgpui::WindowState{
      .framebuffer_size = {.width = 320.0F, .height = 240.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false});
  FakeApplication application(window);
  TestView view;
  RecordingFrame frame;
  int close_callback_count = 0;
  bool pending_state_matched = true;

  application.on_run = [&](FakeApplication&) {
    window.request_close();
    window.request_close();
  };
  const int result = cgpui::run_app(
      application,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        return std::make_unique<RecordingRenderer>(frame, application.run_count);
      },
      cgpui::AppRunnerOptions{
          .runtime = {.request_initial_redraw = false},
          .setup =
              [&](cgpui::WindowRuntime& runtime) {
                runtime.set_close_requested_callback(
                    [&](const cgpui::WindowRuntimeContext& context) {
                      close_callback_count += 1;
                      const auto close =
                          context.platform_window.close_request_state();
                      pending_state_matched = pending_state_matched &&
                          close.pending && !close.accepted &&
                          close.source ==
                              cgpui::WindowCloseRequestSource::application &&
                          close.sequence ==
                              static_cast<std::uint64_t>(close_callback_count);
                      if (close_callback_count == 1) {
                        (void)context.cancel_window_close();
                      } else {
                        (void)context.accept_window_close();
                      }
                    });
              },
      });

  const auto close = window.close_request_state();
  if (result != 0 || close_callback_count != 2 ||
      application.quit_count != 1 || !pending_state_matched) {
    return 930;
  }
  if (close.pending || !close.accepted || close.sequence != 2 ||
      close.cancelled_count != 1 || close.accepted_count != 1 ||
      !window.state().close_requested) {
    return 931;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_run_app_builds_runtime_and_runs_window();
      result != 0) {
    return result;
  }
  if (const int result = test_run_app_accepts_app_context_setup_callback();
      result != 0) {
    return result;
  }
  if (const int result =
          test_window_options_and_app_context_open_window_skeleton();
      result != 0) {
    return result;
  }
  if (const int result = test_app_context_global_state_helpers();
      result != 0) {
    return result;
  }
  if (const int result =
          test_app_opened_window_keeps_root_view_alive_until_run_app_returns();
      result != 0) {
    return result;
  }
  if (const int result =
          test_multi_window_registry_owns_independent_runtime_records();
      result != 0) {
    return result;
  }
  if (const int result =
          test_app_opened_window_routes_native_events_by_runtime_id();
      result != 0) {
    return result;
  }
  if (const int result =
          test_app_opened_window_close_cleans_runtime_owned_state();
      result != 0) {
    return result;
  }
  if (const int result =
          test_app_opened_window_records_native_creation_error();
      result != 0) {
    return result;
  }
  if (const int result =
          test_app_context_installs_native_menu_and_accelerators();
      result != 0) {
    return result;
  }
  if (const int result =
          test_app_context_shows_native_file_dialog_skeleton();
      result != 0) {
    return result;
  }
  if (const int result = test_app_context_registers_command_palette_entries();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_close_callback_can_cancel_then_accept();
      result != 0) {
    return result;
  }
  return 0;
}
