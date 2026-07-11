#include "cgpui/cgpui.hpp"

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct NativeWindow {
  cgpui::WindowState state;
  cgpui::PlatformEventCallback callback;
  std::vector<cgpui::PlatformAccessibilityTreeUpdate> accessibility_updates;
};

class AccessibilityWindow final : public cgpui::PlatformWindow {
 public:
  explicit AccessibilityWindow(NativeWindow& window) : window_(window) {}
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return window_.state; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}
  void update_accessibility_tree(
      cgpui::PlatformAccessibilityTreeUpdate update) override {
    window_.accessibility_updates.push_back(std::move(update));
  }

 private:
  NativeWindow& window_;
};

class AccessibilityApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root.state = {.framebuffer_size = descriptor.size,
                  .scale = cgpui::DpiScale{1.0F}};
    root.callback = std::move(callback);
    return std::make_unique<AccessibilityWindow>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    child.state = {.framebuffer_size = descriptor.size,
                   .scale = cgpui::DpiScale{1.0F}};
    child.callback = std::move(callback);
    return std::make_unique<AccessibilityWindow>(child);
  }
  int run() override {
    on_run();
    return 0;
  }
  void quit() override {}

  NativeWindow root;
  NativeWindow child;
  std::function<void()> on_run;
};

class AccessibilityFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class AccessibilityRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<AccessibilityFrame>();
  }
};

class StaticAccessibilityView final : public cgpui::View {
 public:
  StaticAccessibilityView(
      std::uint64_t root_id,
      std::uint64_t text_id,
      std::string text)
      : root_id_(root_id), text_id_(text_id), text_(std::move(text)) {}

  bool supports_static_render() const override { return true; }
  cgpui::StaticElementTreeView render_static(
      cgpui::ViewContext& context) override {
    context.focus(cgpui::ElementId{text_id_});
    nodes_ = {};
    nodes_[root_id_ - 1] = cgpui::StaticElementNode{
        .id = cgpui::ElementId{root_id_},
        .children = {.first = 0, .count = 1},
        .kind = cgpui::StaticElementKind::container,
        .bounds = {.size = context.viewport_size},
    };
    nodes_[text_id_ - 1] = cgpui::StaticElementNode{
        .id = cgpui::ElementId{text_id_},
        .parent_id = cgpui::ElementId{root_id_},
        .kind = cgpui::StaticElementKind::text,
        .bounds = {.origin = {10.0F, 10.0F}, .size = {120.0F, 24.0F}},
        .intrinsic_size = {120.0F, 24.0F},
        .text = text_,
        .focusable = true,
    };
    children_[0] = cgpui::ElementId{text_id_};
    return {nodes_, children_, cgpui::ElementId{root_id_}};
  }
  void paint(cgpui::PaintList&, cgpui::Size) override {}
  void set_text(std::string text) { text_ = std::move(text); }

 private:
  std::uint64_t root_id_;
  std::uint64_t text_id_;
  std::string text_;
  std::array<cgpui::StaticElementNode, 2> nodes_{};
  std::array<cgpui::ElementId, 1> children_{};
};

bool update_matches(
    const cgpui::PlatformAccessibilityTreeUpdate& update,
    std::uint64_t root_id,
    std::uint64_t text_id,
    std::string_view text) {
  return update.root_element_id == root_id && update.node_count == 2 &&
      update.focused_node_count == 1 && update.nodes.size() == 2 &&
      update.nodes[1].element_id == text_id && update.nodes[1].name == text &&
      update.nodes[1].focused;
}

} // namespace

int main() {
  AccessibilityApplication application;
  StaticAccessibilityView root_view{1, 2, "Root tree"};
  auto child_view =
      std::make_unique<StaticAccessibilityView>(2, 1, "Child tree");
  StaticAccessibilityView* child_view_ptr = child_view.get();
  std::vector<std::unique_ptr<AccessibilityRenderer>> renderers;
  cgpui::WindowRuntime runtime(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        auto renderer = std::make_unique<AccessibilityRenderer>();
        cgpui::Renderer* raw = renderer.get();
        renderers.push_back(std::move(renderer));
        return raw;
      });
  const cgpui::AppOpenedWindow opened = runtime.open_window(
      cgpui::WindowOptions{}.title("Child").size(320.0F, 200.0F),
      std::move(child_view));

  int failure = 1;
  application.on_run = [&] {
    application.root.callback(cgpui::WindowRedrawRequested{});
    const cgpui::WindowRuntimeRecord* root_record =
        runtime.window_runtime_record(runtime.root_window_runtime_id());
    if (application.root.accessibility_updates.size() != 1) {
      failure = 20;
      return;
    }
    if (!application.child.accessibility_updates.empty()) {
      failure = 21;
      return;
    }
    if (root_record == nullptr) {
      failure = 22;
      return;
    }
    if (!root_record->last_accessibility_update.has_value()) {
      failure = 23;
      return;
    }
    if (!update_matches(
            application.root.accessibility_updates.back(), 1, 2,
            "Root tree")) {
      failure = 24;
      return;
    }
    const cgpui::PlatformAccessibilityTreeUpdate root_history =
        *root_record->last_accessibility_update;

    application.child.callback(cgpui::WindowRedrawRequested{});
    const cgpui::WindowRuntimeRecord* child_record =
        runtime.window_runtime_record(opened.runtime_id);
    root_record = runtime.window_runtime_record(runtime.root_window_runtime_id());
    if (application.root.accessibility_updates.size() != 1 ||
        application.child.accessibility_updates.size() != 1 ||
        child_record == nullptr || root_record == nullptr ||
        !child_record->last_accessibility_update.has_value() ||
        !root_record->last_accessibility_update.has_value() ||
        !update_matches(
            application.child.accessibility_updates.back(), 2, 1,
            "Child tree") ||
        root_record->last_accessibility_update->root_element_id !=
            root_history.root_element_id) {
      failure = 3;
      return;
    }

    child_view_ptr->set_text("Child updated");
    application.child.callback(cgpui::WindowRedrawRequested{});
    child_record = runtime.window_runtime_record(opened.runtime_id);
    root_record = runtime.window_runtime_record(runtime.root_window_runtime_id());
    const auto& child_update = application.child.accessibility_updates.back();
    const bool isolated = application.root.accessibility_updates.size() == 1 &&
        application.child.accessibility_updates.size() == 2 &&
        child_record != nullptr && root_record != nullptr &&
        child_record->last_accessibility_update.has_value() &&
        root_record->last_accessibility_update.has_value() &&
        update_matches(child_update, 2, 1, "Child updated") &&
        child_update.live_updates.size() == 1 &&
        child_update.live_updates[0].kind ==
            cgpui::PlatformAccessibilityLiveUpdateKind::text_changed &&
        child_update.live_updates[0].element_id == 1 &&
        root_record->last_accessibility_update->root_element_id ==
            root_history.root_element_id &&
        root_record->last_accessibility_update->nodes[1].name == "Root tree";
    failure = isolated ? 0 : 4;
  };

  const int result = runtime.run(
      cgpui::WindowDescriptor{
          .title = "Root",
          .size = {.width = 640.0F, .height = 480.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  if (result != 0) return 5;
  if (renderers.size() != 2) return 6;
  return failure;
}
