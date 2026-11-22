#include "cgpui/cgpui.hpp"
#include "cgpui/ui/workbench_widgets.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// PopoverMenu on a platform with popup windows: the open menu lives in a
// popup child window placed by the platform, the root paints none of it,
// pointer input in the popup activates items through the root view, and
// closing removes the popup window. Without popups the overlay is used.
namespace {

struct NativeWindow {
  cgpui::WindowState state;
  cgpui::WindowDescriptor descriptor;
  cgpui::PlatformEventCallback callback;
  bool open = false;
};

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  explicit FakeWindow(NativeWindow& window) : window_(window) { window_.open = true; }
  ~FakeWindow() override { window_.open = false; }
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return window_.state; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(std::optional<cgpui::ImeTextInputPlacement>) override {}
  // Below the anchor, left edges aligned, no screen edges.
  cgpui::PlatformPopupPlacementResult place_popup(
      cgpui::WindowPopupPlacement placement) override {
    const auto& anchor = placement.anchor;
    return {.supported = true,
            .placed = true,
            .bounds = {{anchor.origin.x, anchor.origin.y + anchor.size.height + placement.gap},
                       window_.descriptor.size}};
  }

 private:
  NativeWindow& window_;
};

class FakeApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root = {.state = {.framebuffer_size = descriptor.size, .scale = {1.0F}},
            .descriptor = descriptor, .callback = std::move(callback)};
    return std::make_unique<FakeWindow>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor, cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    popups.push_back(std::make_unique<NativeWindow>(NativeWindow{
        .state = {.framebuffer_size = descriptor.size, .scale = {1.0F}},
        .descriptor = descriptor, .callback = std::move(callback)}));
    return std::make_unique<FakeWindow>(*popups.back());
  }
  bool popup_windows_supported() const override { return true; }
  int run() override {
    on_run();
    return 0;
  }
  void quit() override {}

  NativeWindow root;
  std::vector<std::unique_ptr<NativeWindow>> popups;
  std::function<void()> on_run;
};

class FakeFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class FakeRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<FakeFrame>();
  }
};

std::vector<cgpui::MenuItem> items() {
  return {cgpui::MenuItem::action("file.new", "New File"),
          cgpui::MenuItem::action("edit.rename", "Rename")};
}

class MenuView final : public cgpui::View {
 public:
  explicit MenuView(bool popups) : menu_(cgpui::PopoverMenuOptions{.popup_window = popups}) {
    menu_.on_activate([this](std::string_view id) { activated = std::string(id); });
  }
  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    menu_.layout(context.viewport_size, context.scale, theme_);
    cgpui::WidgetTreeBuilder builder(context);
    menu_.append_elements(builder, builder.root());
    builder.install(context);
    return cgpui::AnyElement{};
  }
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    menu_.paint(paint_list, theme_);
    root_paint_commands = paint_list.commands().size();
  }
  cgpui::EventResult handle_event(const cgpui::PlatformEvent& event,
                                  const cgpui::WindowRuntimeContext& context) override {
    if (const auto result = menu_.handle_event(event, context); result.consumed) return result;
    if (const auto* button = std::get_if<cgpui::PointerButton>(&event);
        button != nullptr && button->button == cgpui::MouseButton::right && button->pressed) {
      menu_.open_at(button->position, items(), context);
      context.request_render();
    }
    return cgpui::EventResult::unhandled();
  }

  cgpui::PopoverMenu menu_;
  cgpui::WorkbenchTheme theme_ = cgpui::workbench_theme(cgpui::SystemAppearance{});
  std::string activated;
  std::size_t root_paint_commands = 0;
};

cgpui::PointerButton button(cgpui::MouseButton which, bool pressed, cgpui::Point at) {
  return {.button = which, .pressed = pressed, .click_count = 1, .position = at};
}

}  // namespace

int main() {
  FakeApplication application;
  MenuView view(true);
  std::vector<std::unique_ptr<FakeRenderer>> renderers;
  cgpui::WindowRuntime runtime(
      application, view,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        renderers.push_back(std::make_unique<FakeRenderer>());
        return renderers.back().get();
      });
  int failure = 0;
  const auto frame = [&] { application.root.callback(cgpui::WindowRedrawRequested{}); };
  const auto drain = [&] { application.root.callback(cgpui::WindowWakeupRequested{}); };
  application.on_run = [&] {
    application.root.callback(cgpui::WindowFocused{.focused = true});
    frame();
    application.root.callback(button(cgpui::MouseButton::right, true, {50, 60}));
    frame();
    drain();
    // One popup window for the open level, placed by the platform; the
    // level keeps root-window coordinates and the root paints nothing.
    if (application.popups.size() != 1 || !application.popups[0]->open ||
        application.popups[0]->descriptor.kind != cgpui::WindowKind::popup) {
      failure = 1;
      return;
    }
    const cgpui::Rect level = view.menu_.level_bounds(0);
    const auto& popup = *application.popups[0];
    if (level.origin.x != 50.0F || level.origin.y != 60.0F ||
        popup.descriptor.size.width != level.size.width + 80.0F) {
      failure = 2;
      return;
    }
    frame();
    if (view.root_paint_commands != 0) {
      failure = 3;
      return;
    }
    // A click on "Rename" in the popup (popup-local coordinates) reaches the
    // menu through the root view and activates the item.
    const cgpui::Rect rename = view.menu_.item_rect(0, 1);
    const cgpui::Point local{rename.origin.x + 10.0F - (level.origin.x - 40.0F),
                             rename.origin.y + 5.0F - (level.origin.y - 40.0F)};
    popup.callback(cgpui::PointerMoved{.position = local});
    popup.callback(button(cgpui::MouseButton::left, true, local));
    popup.callback(button(cgpui::MouseButton::left, false, local));
    drain();
    drain();
    if (view.activated != "edit.rename" || view.menu_.is_open() ||
        application.popups[0]->open) {
      failure = 4;
    }
  };
  const int result = runtime.run(
      cgpui::WindowDescriptor{.title = "Root", .size = {640.0F, 480.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  return result == 0 ? failure : 10;
}
