#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/ui.hpp"

#include <algorithm>
#include <cstdlib>
#include <expected>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

class HelloView final : public cgpui::View {
 public:
  struct DemoState {
    int revision = 0;
  };

  HelloView() {
    root_id_ = element_tree_.set_root(
        cgpui::ElementBuilder::box()
            .style(cgpui::Style{}
                       .with_background_color(cgpui::Color{
                           .r = 0.12F, .g = 0.15F, .b = 0.18F, .a = 1.0F})
                       .with_preferred_size(
                           cgpui::Size{.width = 520.0F, .height = 260.0F})
                       .with_padding(cgpui::EdgeSizes::all(18.0F))
                       .with_border_radius(cgpui::BorderRadii::all(8.0F)))
            .build());
    accent_id_ = element_tree_.append_child(
        root_id_,
        cgpui::ElementBuilder::box()
            .style(cgpui::Style{}
                       .with_background_color(cgpui::Color{
                           .r = 0.24F, .g = 0.64F, .b = 0.92F, .a = 1.0F})
                       .with_preferred_size(
                           cgpui::Size{.width = 180.0F, .height = 16.0F})
                       .with_z_index(1))
            .build());
    input_id_ = element_tree_.append_child(
        root_id_,
        cgpui::ElementBuilder::box()
            .style(cgpui::Style{}
                       .with_background_color(cgpui::Color{
                           .r = 0.90F, .g = 0.94F, .b = 0.97F, .a = 1.0F})
                       .with_preferred_size(
                           cgpui::Size{.width = 440.0F, .height = 58.0F})
                       .with_border_width(cgpui::EdgeSizes::all(1.0F))
                       .with_border_color(cgpui::Color{
                           .r = 0.20F, .g = 0.32F, .b = 0.42F, .a = 1.0F})
                       .with_border_radius(cgpui::BorderRadii::all(6.0F))
                       .with_z_index(2))
            .build());
  }

  [[nodiscard]] cgpui::ElementId root_id() const {
    return root_id_;
  }

  [[nodiscard]] cgpui::ElementId input_id() const {
    return input_id_;
  }

  [[nodiscard]] cgpui::Element* element_root() {
    return element_tree_.get(root_id_);
  }

  [[nodiscard]] cgpui::TextModel& text_model() {
    return text_model_;
  }

  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport_size) override {
    const float panel_width = std::min(viewport_size.width - 80.0F, 560.0F);
    const float panel_height = 260.0F;
    const cgpui::Point panel_origin{
        .x = (viewport_size.width - panel_width) * 0.5F,
        .y = (viewport_size.height - panel_height) * 0.5F,
    };
    set_element_bounds(root_id_, cgpui::Rect{
                                     .origin = panel_origin,
                                     .size = {.width = panel_width,
                                              .height = panel_height},
                                 });

    const float accent_width =
        96.0F + static_cast<float>(text_model_.text().size()) * 8.0F;
    set_element_bounds(accent_id_, cgpui::Rect{
                                       .origin = {.x = panel_origin.x + 28.0F,
                                                  .y = panel_origin.y + 34.0F},
                                       .size = {.width = std::min(accent_width,
                                                                  panel_width -
                                                                      56.0F),
                                                .height = 16.0F},
                                   });
    set_element_bounds(input_id_, cgpui::Rect{
                                      .origin = {.x = panel_origin.x + 28.0F,
                                                 .y = panel_origin.y + 118.0F},
                                      .size = {.width = panel_width - 56.0F,
                                               .height = 58.0F},
                                  });

    element_tree_.paint(paint_list);

    const float cursor_x =
        panel_origin.x + 48.0F +
        static_cast<float>(text_model_.cursor()) * 7.0F;
    paint_list.fill_rect(
        cgpui::Rect{
            .origin = {.x = cursor_x, .y = panel_origin.y + 130.0F},
            .size = {.width = 2.0F, .height = 34.0F},
        },
        cgpui::Color{.r = 0.10F, .g = 0.18F, .b = 0.24F, .a = 1.0F});
    if (text_model_.has_composition()) {
      paint_list.fill_rect(
          cgpui::Rect{
              .origin = {.x = panel_origin.x + 48.0F,
                         .y = panel_origin.y + 166.0F},
              .size = {.width = std::max(24.0F,
                                         static_cast<float>(
                                             text_model_.composition_text()
                                                 .size()) *
                                             7.0F),
                       .height = 3.0F},
          },
          cgpui::Color{.r = 0.86F, .g = 0.48F, .b = 0.20F, .a = 1.0F});
    }
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    ensure_subscription(context);
    if (const auto* focused = std::get_if<cgpui::WindowFocused>(&event);
        focused != nullptr && focused->focused) {
      context.runtime.request_keyboard_focus(input_id_);
      context.runtime.request_layout();
    }
    if (const auto* button = std::get_if<cgpui::PointerButton>(&event);
        button != nullptr && button->pressed) {
      context.runtime.request_keyboard_focus(input_id_);
      return cgpui::EventResult::consumed_event();
    }
    if (std::holds_alternative<cgpui::TextInput>(event) ||
        std::holds_alternative<cgpui::ImeComposition>(event)) {
      mark_model_changed(context);
      context.runtime.request_layout();
    }
    return cgpui::EventResult::unhandled();
  }

 private:
  void set_element_bounds(cgpui::ElementId id, cgpui::Rect rect) {
    if (cgpui::Element* element = element_tree_.get(id); element != nullptr) {
      element->set_layout_bounds(rect);
    }
  }

  void ensure_subscription(const cgpui::WindowRuntimeContext& context) {
    if (state_id_.has_value()) {
      return;
    }
    state_id_ = context.runtime.insert_entity(DemoState{});
    context.runtime.subscribe_view_to_entity(context.view_id, *state_id_);
  }

  void mark_model_changed(const cgpui::WindowRuntimeContext& context) {
    if (!state_id_.has_value()) {
      return;
    }
    if (DemoState* state = context.runtime.mutate_entity(*state_id_);
        state != nullptr) {
      state->revision += 1;
      context.runtime.notify_entity_changed(*state_id_);
    }
  }

  cgpui::ElementTree element_tree_;
  cgpui::TextModel text_model_;
  cgpui::ElementId root_id_;
  cgpui::ElementId accent_id_;
  cgpui::ElementId input_id_;
  std::optional<cgpui::EntityId<DemoState>> state_id_;
};

int main() {
  const char* exit_after_first_frame_env =
      std::getenv("CGPUI_EXIT_AFTER_FIRST_FRAME");
  const bool exit_after_first_frame =
      exit_after_first_frame_env != nullptr &&
      std::string_view(exit_after_first_frame_env) != "0";
  const char* resize_after_first_frame_env =
      std::getenv("CGPUI_RESIZE_AFTER_FIRST_FRAME");
  const bool resize_after_first_frame =
      resize_after_first_frame_env != nullptr &&
      std::string_view(resize_after_first_frame_env) != "0";
  const char* close_after_first_frame_env =
      std::getenv("CGPUI_CLOSE_AFTER_FIRST_FRAME");
  const bool close_after_first_frame =
      close_after_first_frame_env != nullptr &&
      std::string_view(close_after_first_frame_env) != "0";
  const char* injected_text_env = std::getenv("CGPUI_DEMO_INJECT_TEXT");
  const std::string injected_text =
      injected_text_env == nullptr ? std::string{} : std::string{injected_text_env};

  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  HelloView view;
  cgpui::Size viewport_size{960.0F, 640.0F};
  bool render_failed = false;
  bool first_frame_presented = false;
  bool resize_requested_after_first_frame = false;
  bool second_frame_presented = false;
  bool close_requested_after_first_frame = false;
  std::unique_ptr<cgpui::Renderer> renderer;

  cgpui::WindowRuntime runtime(
      **app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor) {
        auto renderer_result = cgpui::create_renderer(descriptor);
        if (!renderer_result) {
          return cgpui::Result<cgpui::Renderer*>{
              std::unexpected(renderer_result.error())};
        }
        renderer = std::move(*renderer_result);
        return cgpui::Result<cgpui::Renderer*>{renderer.get()};
      });
  runtime.set_element_root(view.element_root());
  runtime.bind_text_model(view.input_id(), &view.text_model());
  runtime.set_element_cursor(view.root_id(), cgpui::CursorShape::pointing_hand);
  runtime.set_element_cursor(view.input_id(), cgpui::CursorShape::text);
  runtime.register_action(
      "demo.clear_text",
      [&](const cgpui::WindowRuntimeContext& context) {
        view.text_model().set_selection(0, view.text_model().text().size());
        (void)view.text_model().delete_forward();
        context.runtime.request_layout();
        return cgpui::EventResult::consumed_event();
      });
  runtime.bind_key(cgpui::KeyBinding{
      .key_code = static_cast<std::uint32_t>('L'),
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true},
      .action_name = "demo.clear_text",
  });
  runtime.set_error_callback([](const cgpui::Error& error) {
    std::cerr << error.message << '\n';
  });
  runtime.set_close_requested_callback(
      [&](const cgpui::WindowRuntimeContext&) {
        if (close_after_first_frame && first_frame_presented) {
          close_requested_after_first_frame = true;
        }
      });
  runtime.set_after_frame_callback(
      [&](const cgpui::WindowRuntimeContext& context) {
        viewport_size = context.viewport_size;
        if (!first_frame_presented) {
          first_frame_presented = true;
          if (!injected_text.empty()) {
            view.text_model().insert_text(injected_text);
            context.runtime.request_layout();
          }
          if (close_after_first_frame) {
            context.window.request_close();
            return;
          }
          if (resize_after_first_frame) {
            viewport_size =
                cgpui::Size{viewport_size.width * 0.75F,
                            viewport_size.height * 0.75F};
            auto resized_result = context.runtime.resize_surface(
                viewport_size,
                cgpui::DpiScale{1.0F});
            if (!resized_result) {
              std::cerr << resized_result.error().message << '\n';
              render_failed = true;
              context.application.quit();
              return;
            }
            resize_requested_after_first_frame = true;
            context.window.request_redraw();
            return;
          }
        } else {
          second_frame_presented = true;
        }
        if (exit_after_first_frame ||
            (resize_after_first_frame && second_frame_presented)) {
          context.application.quit();
        }
      });

  const int run_result = runtime.run(cgpui::WindowDescriptor{
      .title = "CGPUI Hello Window",
      .size = viewport_size});
  if (render_failed) {
    return 1;
  }
  if (exit_after_first_frame && !first_frame_presented) {
    std::cerr << "first frame was not presented\n";
    return 1;
  }
  if (resize_after_first_frame &&
      (!resize_requested_after_first_frame || !second_frame_presented)) {
    std::cerr << "resize smoke did not present a second frame\n";
    return 1;
  }
  if (close_after_first_frame && !close_requested_after_first_frame) {
    std::cerr << "close smoke did not receive a close request\n";
    return 1;
  }
  return run_result;
}
