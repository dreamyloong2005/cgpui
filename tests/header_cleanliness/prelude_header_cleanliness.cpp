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
  cgpui::EntityStore<cgpui::TextModel> models;
  const cgpui::Model<cgpui::TextModel> model_id =
      models.insert(cgpui::TextModel("model"));
  const cgpui::Entity<cgpui::TextModel> entity_id = model_id;
  const cgpui::WeakEntity<cgpui::TextModel> weak_model(model_id);
  const cgpui::WeakView weak_view(cgpui::ViewId{1});
  cgpui::ModelObserver<cgpui::TextModel> observer =
      [](const cgpui::ViewContext&, cgpui::Model<cgpui::TextModel>) {};
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
                        .build());
  PreludeView view;
  cgpui::AppRunnerOptions options;
  options.setup_context = app_setup;
  (void)view;
  (void)options;
  return root_id.value != 0 && tree.root_id() == root_id &&
                 window_descriptor.title == "Prelude Window" &&
                 window_descriptor.size.height == 13.0F
             ? 0
             : 1;
}
