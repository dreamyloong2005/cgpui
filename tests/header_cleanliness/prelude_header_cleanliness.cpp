#include "cgpui/cgpui.hpp"

class PreludeView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.fill_rect(
        cgpui::Rect{.origin = {0.0F, 0.0F}, .size = {1.0F, 1.0F}},
        cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F});
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent&,
      const cgpui::ViewContext& context) override {
    context.request_paint();
    return cgpui::EventResult::unhandled();
  }

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    context.request_layout();
    return cgpui::into_element(cgpui::div().size(2.0F, 3.0F));
  }
};

int main() {
  cgpui::ElementTree tree;
  cgpui::TextModel model("x");
  cgpui::ScrollState scroll_state;
  cgpui::EntityStore<cgpui::TextModel> models;
  const cgpui::Model<cgpui::TextModel> model_id =
      models.insert(cgpui::TextModel("model"));
  const cgpui::Entity<cgpui::TextModel> entity_id = model_id;
  const cgpui::WeakEntity<cgpui::TextModel> weak_model(model_id);
  const cgpui::WeakView weak_view(cgpui::ViewId{1});
  cgpui::ModelObserver<cgpui::TextModel> observer =
      [](const cgpui::ViewContext&, cgpui::Model<cgpui::TextModel>) {};
  cgpui::ModelObserver<cgpui::TextModel> context_observer =
      [](const cgpui::Context<PreludeView>&,
         cgpui::Model<cgpui::TextModel>) {};
  cgpui::AppContextSetupCallback app_setup =
      [](cgpui::AppContext& app_context) {
        (void)app_context.runtime.invalidation_state();
        const cgpui::AppOpenedWindow opened =
            app_context.open_window(cgpui::WindowOptions{}
                                        .title("Prelude Window")
                                        .size(11.0F, 13.0F));
        (void)opened;
      };
  const cgpui::WindowDescriptor window_descriptor =
      cgpui::WindowOptions{}
          .title("Prelude Window")
          .size(cgpui::Size{11.0F, 13.0F})
          .to_descriptor();
  (void)entity_id;
  (void)weak_model;
  (void)weak_view;
  (void)observer;
  (void)context_observer;
  const cgpui::ElementId root_id =
      tree.set_root(cgpui::div()
                        .size(cgpui::px(12.0F), cgpui::px(8.0F))
                        .gap(cgpui::px(1.0F))
                        .margin(cgpui::edges(2.0F, 1.0F))
                        .background(cgpui::rgb(32, 64, 128))
                        .foreground(cgpui::rgba(255, 255, 255, 0.75F))
                        .border_width(cgpui::edges(1.0F))
                        .border_color(cgpui::rgba(255, 255, 255, 0.75F))
                        .border_radius(cgpui::BorderRadii::all(2.0F))
                        .child(cgpui::text(model))
                        .child(cgpui::child_view(cgpui::ViewId{2})
                                   .size(4.0F, 5.0F))
                        .build());
  cgpui::AnyElement scroll_element =
      cgpui::scroll(scroll_state, cgpui::div().size(6.0F, 7.0F));
  const auto* scroll =
      dynamic_cast<const cgpui::ScrollElement*>(scroll_element.get());
  cgpui::AnyElement button_element =
      cgpui::button("prelude.accept")
          .style(cgpui::Style{}.with_preferred_size(
              cgpui::Size{14.0F, 6.0F}))
          .focus_style(cgpui::StyleOverlay{}.with_border_width(
              cgpui::edges(1.0F)))
          .disabled_style(cgpui::StyleOverlay{}.with_foreground_color(
              cgpui::rgb(128, 128, 128)))
          .child(cgpui::div().size(2.0F, 2.0F))
          .build();
  const auto* button =
      dynamic_cast<const cgpui::ButtonElement*>(button_element.get());
  const cgpui::StyleClassId class_id = cgpui::style_class("prelude.card");
  cgpui::StyleClasses classes;
  classes.add(class_id);
  cgpui::Theme theme;
  const cgpui::ThemeTokenId accent_token = cgpui::theme_token("color.accent");
  const cgpui::ThemeTokenId gap_token = cgpui::theme_token("space.gap");
  theme.set_color(accent_token, cgpui::rgb(1, 2, 3))
      .set_spacing(gap_token, cgpui::px(4.0F));
  const std::optional<cgpui::Color> accent_color = theme.color(accent_token);
  const std::optional<float> gap_spacing = theme.spacing(gap_token);
  PreludeView view;
  cgpui::AppRunnerOptions options;
  options.setup_context = app_setup;
  (void)view;
  (void)options;
  return root_id.value != 0 && tree.root_id() == root_id &&
                 window_descriptor.title == "Prelude Window" &&
                 window_descriptor.size.height == 13.0F &&
                 scroll != nullptr && scroll->state() == &scroll_state &&
                 button != nullptr &&
                 button->action_name() == "prelude.accept" &&
                 button->focusable() && button->child() != nullptr &&
                 classes.contains(class_id) && accent_color.has_value() &&
                 accent_color->b == 3.0F / 255.0F &&
                 gap_spacing.has_value() && *gap_spacing == 4.0F
             ? 0
             : 1;
}
