#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/ui.hpp"

#include <expected>
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
  RecordingRenderer(RecordingFrame& frame, int& begin_frame_count)
      : frame_(frame), begin_frame_count_(begin_frame_count) {}

  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    resize_count += 1;
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

  void request_redraw() override {
    request_redraw_count += 1;
    if (callback) {
      callback(cgpui::WindowRedrawRequested{});
    }
  }

  void request_close() override {
    request_close_count += 1;
    if (callback) {
      callback(cgpui::WindowCloseRequested{});
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
  cgpui::WindowState state_;
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

  int run() override {
    run_count += 1;
    if (window_.callback) {
      window_.callback(cgpui::WindowRedrawRequested{});
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

  int create_window_count = 0;
  int run_count = 0;
  int quit_count = 0;
  int request_wakeup_count = 0;
  int install_native_menu_count = 0;
  int run_result = 0;
  cgpui::WindowDescriptor last_descriptor{};
  std::vector<cgpui::WindowDescriptor> created_descriptors;
  cgpui::NativeMenuModel last_menu_model;
  std::string failing_window_title;

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
                opened_descriptor = opened.descriptor;
                const auto opened_windows =
                    context.runtime.app_opened_windows();
                recorded_count = opened_windows.size();
                if (!opened_windows.empty()) {
                  recorded_descriptor = opened_windows.front().descriptor;
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
  if (recorded_count != 1) {
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
  if (application.create_window_count != 2 ||
      application.created_descriptors.size() != 2 ||
      application.created_descriptors[0].title != "Secondary Window" ||
      application.created_descriptors[1].title != "CGPUI") {
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
                    first_record->window != nullptr &&
                    second_record->window != nullptr &&
                    first_record->renderer == nullptr &&
                    second_record->renderer == nullptr &&
                    first_record->owns_window &&
                    second_record->owns_window &&
                    first_record->owns_renderer &&
                    second_record->owns_renderer &&
                    first_record->active &&
                    second_record->active &&
                    !first_record->native_window_error.has_value() &&
                    !second_record->native_window_error.has_value() &&
                    context.runtime.app_opened_window_root_view(
                        first_opened.root_view_id) == first_root_ptr &&
                    context.runtime.app_opened_window_root_view(
                        second_opened.root_view_id) == second_root_ptr;

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
                          root_record->window == &frame_context.window &&
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
                          first_frame_record->renderer == nullptr &&
                          second_frame_record->renderer == nullptr &&
                          first_frame_record->owns_window &&
                          second_frame_record->owns_window &&
                          first_frame_record->owns_renderer &&
                          second_frame_record->owns_renderer &&
                          first_frame_record->active &&
                          second_frame_record->active &&
                          !first_frame_record->native_window_error.has_value() &&
                          !second_frame_record->native_window_error.has_value();
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
  if (renderer_factory_count != 1 || renderer_begin_frame_count != 1 ||
      frame.present_count != 1) {
    return 35;
  }
  if (!first_destroyed || !second_destroyed) {
    return 36;
  }
  if (application.create_window_count != 3 ||
      application.created_descriptors.size() != 3 ||
      application.created_descriptors[0].title != "First Child" ||
      application.created_descriptors[1].title != "Second Child" ||
      application.created_descriptors[2].title != "CGPUI") {
    return 37;
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
                    record->native_window_error.has_value() &&
                    record->native_window_error->code ==
                        cgpui::ErrorCode::window_creation_failed;
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
      application.created_descriptors[0].title != "Unsupported Child" ||
      application.created_descriptors[1].title != "CGPUI") {
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
          test_app_opened_window_records_native_creation_error();
      result != 0) {
    return result;
  }
  if (const int result =
          test_app_context_installs_native_menu_and_accelerators();
      result != 0) {
    return result;
  }
  return 0;
}
