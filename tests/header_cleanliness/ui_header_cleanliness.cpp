#include "cgpui/ui/ui.hpp"
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"

#include <memory>

struct TestModel {
  int value = 0;
};

class TestView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.fill_rect(
        cgpui::Rect{.origin = {10.0F, 10.0F}, .size = {20.0F, 20.0F}},
        cgpui::Color{.r = 1.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F});
  }

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    context.request_render();
    const cgpui::Model<TestModel> model = context.new_model<TestModel>(1);
    const cgpui::WeakEntity<TestModel> weak_model(model);
    (void)context.upgrade_entity(weak_model);
    const cgpui::WeakView weak_view(context.view_id);
    (void)context.upgrade_view(weak_view);
    (void)context.observe_model(
        model,
        [](const cgpui::ViewContext&, cgpui::Model<TestModel>) {});
    (void)context.read_model(model);
    (void)context.update_model(
        model,
        [](TestModel& state) {
          state.value = 2;
        });
    (void)context.remove_model(model);
    return cgpui::into_element(cgpui::div().size(3.0F, 4.0F));
  }
};

class ChildView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

int main() {
  cgpui::PaintList paint_list;
  TestView view;
  cgpui::TextModel text_model;
  cgpui::ScrollModel scroll_model;
  cgpui::RenderRecord render_record{
      .sequence = 1,
      .view_id = cgpui::ViewId{1},
      .viewport_size = cgpui::Size{3.0F, 4.0F},
      .root_element_id = cgpui::ElementId{2},
  };
  cgpui::EventRoute event_route{
      .target_view_id = cgpui::ViewId{1},
      .target_element_id = cgpui::ElementId{2},
      .element_ancestry = {cgpui::ElementId{2}},
      .view_ancestry = {cgpui::ViewId{1}},
      .event_kind = cgpui::EventKind::pointer_moved,
  };
  cgpui::StyleState style_state;
  style_state.base = cgpui::Style{}
                         .with_background_color(cgpui::rgb(0, 0, 0))
                         .with_align_items(cgpui::AlignItems::center)
                         .with_justify_content(cgpui::JustifyContent::end);
  style_state.hover =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(64, 64, 64))
          .with_justify_content(cgpui::JustifyContent::space_between);
  const cgpui::Style resolved = cgpui::resolved_style(
      style_state,
      cgpui::StyleStateFlags{.hovered = true});
  cgpui::AppContextSetupCallback app_setup =
      [](cgpui::AppContext& app_context) {
        (void)app_context.runtime.invalidation_state();
        const cgpui::AppOpenedWindow opened =
            app_context.open_window(
                cgpui::WindowOptions{}.title("Header Window").size(9.0F, 7.0F),
                std::make_unique<ChildView>());
        const cgpui::View* opened_root =
            app_context.runtime.app_opened_window_root_view(
                opened.root_view_id);
        ChildView registered_view;
        const cgpui::ViewId registered_view_id =
            app_context.runtime.register_view(registered_view);
        cgpui::AnyElement placeholder =
            cgpui::into_element(cgpui::child_view(registered_view_id)
                                    .size(cgpui::Size{5.0F, 6.0F}));
        const auto* child_view_placeholder =
            dynamic_cast<const cgpui::ChildViewElement*>(placeholder.get());
        const cgpui::View* found_view =
            app_context.runtime.find_view(registered_view_id);
        const bool removed_view =
            app_context.runtime.remove_view(registered_view_id);
        (void)app_context.runtime.root_view();
        (void)opened_root;
        (void)child_view_placeholder;
        (void)found_view;
        (void)removed_view;
      };
  const cgpui::WindowDescriptor window_descriptor =
      cgpui::WindowOptions{}
          .title("Header Window")
          .size(cgpui::Size{9.0F, 7.0F})
          .to_descriptor();
  cgpui::AppRunnerOptions app_options;
  app_options.setup_context = app_setup;
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .size(cgpui::px(1.0F), cgpui::px(2.0F))
                              .padding(cgpui::edges(cgpui::px(1.0F)))
                              .background(cgpui::rgb(255, 0, 0))
                              .foreground(cgpui::rgba(255, 255, 255, 0.5F))
                              .hover_style(cgpui::StyleOverlay{}
                                               .with_background_color(
                                                   cgpui::rgb(0, 255, 0)))
                              .focus_style(cgpui::StyleOverlay{}
                                               .with_border_width(
                                                   cgpui::edges(1.0F)))
                              .disabled_style(cgpui::StyleOverlay{}
                                                  .with_foreground_color(
                                                      cgpui::rgb(128, 128, 128)))
                              .on_pointer_down([](
                                                   const cgpui::PointerButton&,
                                                   const cgpui::
                                                       ElementEventContext&) {
                                return cgpui::EventResult::consumed_event();
                              })
                              .on_pointer_up([](
                                                 const cgpui::PointerButton&,
                                                 const cgpui::
                                                     ElementEventContext&) {
                                return cgpui::EventResult::unhandled();
                              })
                               .on_pointer_move([](
                                                   const cgpui::PointerMoved&,
                                                   const cgpui::
                                                       ElementEventContext&) {
                                 return cgpui::EventResult::unhandled();
                               }));
  cgpui::AnyElement flex_element =
      cgpui::into_element(cgpui::h_flex()
                              .align_items(cgpui::AlignItems::center)
                              .justify_content(cgpui::JustifyContent::end)
                              .child(cgpui::div().size(1.0F, 1.0F)));
  const auto* flex = dynamic_cast<const cgpui::FlexElement*>(flex_element.get());
  cgpui::ScrollState scroll_state;
  cgpui::AnyElement scroll_element = cgpui::scroll(
      scroll_state,
      cgpui::div().size(cgpui::Size{7.0F, 8.0F}));
  const auto* scroll =
      dynamic_cast<const cgpui::ScrollElement*>(scroll_element.get());
  const auto* pointer =
      dynamic_cast<const cgpui::PointerElement*>(element.get());
  const auto* styled = pointer == nullptr
      ? nullptr
      : dynamic_cast<const cgpui::StyledElement*>(pointer->child());
  scroll_model.set_viewport_size(cgpui::Size{10.0F, 10.0F});
  text_model.insert_text("x");
  view.paint(paint_list, cgpui::Size{100.0F, 100.0F});
  return paint_list.commands().size() == 1 && text_model.text() == "x" &&
                 render_record.sequence == 1 &&
                 render_record.root_element_id.has_value() &&
                 event_route.element_ancestry.size() == 1 &&
                 event_route.view_ancestry.size() == 1 &&
                 styled != nullptr && styled->style().padding.top == 1.0F &&
                 resolved.background_color.has_value() &&
                 resolved.justify_content ==
                     cgpui::JustifyContent::space_between &&
                 static_cast<bool>(app_options.setup_context) &&
                 window_descriptor.title == "Header Window" &&
                 window_descriptor.size.width == 9.0F &&
                 resolved.background_color->r == 64.0F / 255.0F &&
                 styled->style_state().hover.background_color.has_value() &&
                 styled->style().preferred_size.width == 1.0F &&
                 styled->style().background_color.has_value() &&
                 styled->style().background_color->r == 1.0F &&
                  styled->style().foreground_color.has_value() &&
                  styled->style().foreground_color->a == 0.5F &&
                  scroll != nullptr && scroll->state() == &scroll_state &&
                  flex != nullptr &&
                  flex->align_items() == cgpui::AlignItems::center &&
                  flex->justify_content() == cgpui::JustifyContent::end &&
                  scroll_model.offset().x == 0.0F
              ? 0
              : 1;
}
