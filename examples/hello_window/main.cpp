#include "cgpui/cgpui.hpp"

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

  [[nodiscard]] cgpui::TextModel& text_model() {
    return text_model_;
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    ensure_setup(context);

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .background(cgpui::rgb(20, 23, 28))
            .padding(cgpui::edges(40.0F))
            .child(cgpui::v_stack()
                       .size(520.0F, 260.0F)
                       .gap(cgpui::px(18.0F))
                       .padding(cgpui::edges(18.0F))
                       .background(cgpui::rgb(31, 38, 46))
                       .border_radius(cgpui::BorderRadii::all(8.0F))
                       .on_pointer_down([this](
                                            const cgpui::PointerButton& button,
                                            const cgpui::ElementEventContext&) {
                         if (button.pressed) {
                           wants_focus_ = true;
                           return cgpui::EventResult::consumed_event();
                         }
                         return cgpui::EventResult::unhandled();
                       })
                       .child(cgpui::div()
                                  .size(accent_width(), 16.0F)
                                  .background(cgpui::rgb(61, 163, 235))
                                  .border_radius(cgpui::BorderRadii::all(4.0F))
                                  .layer(1))
                       .child(cgpui::text(text_model_)
                                  .size(440.0F, 58.0F)
                                  .font_size(28.0F)
                                  .background(cgpui::rgb(230, 240, 247))
                                  .foreground(cgpui::rgb(26, 36, 47))
                                  .border_width(cgpui::edges(1.0F))
                                  .border_color(cgpui::rgb(51, 82, 107))
                                  .border_radius(cgpui::BorderRadii::all(6.0F))
                                  .layer(2))));
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::ViewContext& context) override {
    ensure_setup(context);
    if (const auto* focused = std::get_if<cgpui::WindowFocused>(&event);
        focused != nullptr && focused->focused) {
      wants_focus_ = true;
      context.request_render();
    }
    if (std::holds_alternative<cgpui::PointerButton>(event)) {
      wants_focus_ = true;
      context.request_render();
      return cgpui::EventResult::consumed_event();
    }
    if (std::holds_alternative<cgpui::TextInput>(event) ||
        std::holds_alternative<cgpui::ImeComposition>(event)) {
      mark_model_changed(context);
      context.request_render();
    }
    return cgpui::EventResult::unhandled();
  }

 private:
  [[nodiscard]] float accent_width() const {
    const float model_width =
        96.0F + static_cast<float>(text_model_.text().size()) * 8.0F;
    return model_width > 464.0F ? 464.0F : model_width;
  }

  void ensure_setup(const cgpui::ViewContext& context) {
    if (!state_id_.has_value()) {
      state_id_ = context.new_model<DemoState>();
      context.subscribe_view_to_entity(context.view_id, *state_id_);
    }
    context.set_element_cursor(panel_id_, cgpui::CursorShape::pointing_hand);
    context.bind_text_model(input_id_, &text_model_);
    context.set_element_cursor(input_id_, cgpui::CursorShape::text);
    if (!bindings_installed_) {
      context.register_action(
          "demo.clear_text",
          [this](const cgpui::ViewContext& action_context) {
            text_model_.set_selection(0, text_model_.text().size());
            (void)text_model_.delete_forward();
            action_context.request_render();
            return cgpui::EventResult::consumed_event();
          });
      context.bind_key(cgpui::KeyBinding{
          .key_code = static_cast<std::uint32_t>('L'),
          .action = cgpui::KeyAction::pressed,
          .modifiers = {.control = true},
          .action_name = "demo.clear_text",
      });
      bindings_installed_ = true;
    }
    if (wants_focus_) {
      context.request_keyboard_focus(input_id_);
      wants_focus_ = false;
    }
  }

  void mark_model_changed(const cgpui::ViewContext& context) {
    if (!state_id_.has_value()) {
      return;
    }
    (void)context.update_model(
        *state_id_,
        [](DemoState& state) {
          state.revision += 1;
        });
  }

  cgpui::TextModel text_model_;
  std::optional<cgpui::Model<DemoState>> state_id_;
  cgpui::ElementId input_id_{1};
  cgpui::ElementId panel_id_{1};
  bool wants_focus_ = true;
  bool bindings_installed_ = false;
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

  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI Hello Window")
                       .size(viewport_size)
                       .to_descriptor();
  options.setup_context = [&](cgpui::AppContext& app_context) {
    app_context.runtime.set_error_callback([](const cgpui::Error& error) {
      std::cerr << error.message << '\n';
    });
    app_context.runtime.set_close_requested_callback(
        [&](const cgpui::ViewContext&) {
          if (close_after_first_frame && first_frame_presented) {
            close_requested_after_first_frame = true;
          }
        });
    app_context.runtime.set_after_render_callback(
        [&](const cgpui::ViewContext& context,
            const cgpui::RenderRecord& record) {
          if (record.root_element_id.has_value()) {
            context.bind_text_model(*record.root_element_id, &view.text_model());
            context.set_element_cursor(
                *record.root_element_id,
                cgpui::CursorShape::text);
            context.request_keyboard_focus(*record.root_element_id);
          }
        });
    app_context.runtime.set_after_frame_callback(
        [&](const cgpui::ViewContext& context) {
          viewport_size = context.viewport_size;
          if (!first_frame_presented) {
            first_frame_presented = true;
            if (!injected_text.empty()) {
              view.text_model().insert_text(injected_text);
              context.request_render();
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
  };

  const int run_result = cgpui::run_app(
      **app,
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      options);
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
