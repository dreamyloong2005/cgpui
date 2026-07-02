#include "cgpui/ui/ui.hpp"
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/style.hpp"
#include "cgpui/ui/text.hpp"

#include <memory>
#include <vector>

struct TestModel {
  int value = 0;
};

class TestView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.push_metadata(cgpui::PaintMetadata{
        .opacity = 0.75F,
        .transform = cgpui::AffineTransform::translation(1.0F, 2.0F),
    });
    paint_list.fill_rect(
        cgpui::Rect{.origin = {10.0F, 10.0F}, .size = {20.0F, 20.0F}},
        cgpui::Color{.r = 1.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F});
    paint_list.fill_rounded_rect(
        cgpui::Rect{.origin = {1.0F, 2.0F}, .size = {3.0F, 4.0F}},
        cgpui::Color{.r = 0.0F, .g = 1.0F, .b = 0.0F, .a = 1.0F},
        cgpui::BorderRadii::all(2.0F));
    paint_list.fill_text(
        cgpui::Rect{.origin = {2.0F, 3.0F}, .size = {12.0F, 16.0F}},
        cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
        "header",
        cgpui::FontDescriptor{.family = "Header"},
        18.0F);
    paint_list.fill_text_selection(
        cgpui::Rect{.origin = {3.0F, 4.0F}, .size = {5.0F, 18.0F}},
        cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.8F, .a = 0.5F},
        cgpui::TextSelectionRange{.start = 1, .end = 3, .collapsed = false},
        18.0F);
    paint_list.fill_text_caret(
        cgpui::Rect{.origin = {9.0F, 4.0F}, .size = {1.0F, 18.0F}},
        cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
        3,
        18.0F);
    paint_list.pop_metadata();
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
    cgpui::Subscription subscription =
        context.observe_model_subscription(
            model,
            [](const cgpui::ViewContext&, cgpui::Model<TestModel>) {});
    const cgpui::SubscriptionId subscription_id = subscription.id();
    (void)subscription.connected();
    (void)context.runtime.remove_subscription(subscription_id);
    (void)subscription.release();
    context.defer([](const cgpui::ViewContext& deferred_context) {
      deferred_context.request_paint();
    });
    const cgpui::TimerId timer_id = context.schedule_timer(
        10,
        [](const cgpui::ViewContext& timer_context) {
          timer_context.request_layout();
        });
    const cgpui::TimerId repeating_timer_id = context.schedule_repeating_timer(
        5,
        [](const cgpui::ViewContext& timer_context) {
          timer_context.request_paint();
        });
    (void)context.runtime.cancel_timer(timer_id);
    (void)context.runtime.cancel_timer(repeating_timer_id);
    cgpui::TaskHandle task = context.spawn_task(
        [](const cgpui::ViewContext& completion_context) {
          completion_context.request_render();
        });
    const cgpui::TaskId task_id = task.id();
    (void)task.active();
    (void)task.complete();
    (void)context.runtime.complete_task(task_id);
    context.runtime.drain_task_completions();
    context.batch_updates([](const cgpui::ViewContext& batch_context) {
      batch_context.request_render();
    });
    context.runtime.batch_updates([](const cgpui::ViewContext& batch_context) {
      batch_context.request_layout();
    });
    const cgpui::RuntimeDiagnosticsSnapshot context_diagnostics =
        context.diagnostics_snapshot();
    const cgpui::RuntimeDiagnosticsSnapshot runtime_diagnostics =
        context.runtime.diagnostics_snapshot();
    (void)context_diagnostics.entity_count;
    (void)runtime_diagnostics.last_render_record;
    (void)runtime_diagnostics.last_frame_statistics;
    const cgpui::FocusHandle focus_handle =
        context.focus_handle(cgpui::ElementId{4});
    focus_handle.request(context);
    (void)focus_handle.id();
    (void)focus_handle.empty();
    (void)focus_handle.contains(context);
    (void)focus_handle.focused(context.input_state());
    focus_handle.release(context.runtime);
    context.register_app_action(
        "header.context.app",
        [](const cgpui::ViewContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "header.context.window",
        [](const cgpui::ViewContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.register_view_action(
        "header.context.view",
        [](const cgpui::ViewContext&) {
          return cgpui::EventResult::consumed_event();
        });
    context.register_focused_element_action(
        cgpui::ElementId{4},
        "header.context.focused",
        [](const cgpui::ViewContext&) {
          return cgpui::EventResult::consumed_event();
        });
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
  const cgpui::TextShapeRun shape_run = cgpui::shape_text(
      "H\xE4\xB8\xAD",
      cgpui::FontDescriptor{.family = "Header"},
      18.0F);
  const std::vector<cgpui::TextGlyphPaint> header_glyphs =
      cgpui::text_glyph_paint_metadata(shape_run);
  const cgpui::RasterizedGlyph header_rasterized =
      cgpui::rasterize_fallback_glyph(header_glyphs[0]);
  cgpui::FontDatabase font_database;
  font_database.add_face(cgpui::FontFaceDescriptor{
      .font = cgpui::FontDescriptor{.family = "Header"},
      .postscript_name = "Header-Regular",
      .source = cgpui::FontSource::test,
      .path = "header.ttf",
  });
  font_database.add_face(cgpui::FontFaceDescriptor{
      .font = cgpui::FontDescriptor{.family = "Fallback"},
      .postscript_name = "Fallback-Regular",
      .source = cgpui::FontSource::test,
      .path = "fallback.ttf",
  });
  font_database.add_generic_fallback_family("Fallback");
  const cgpui::FontFallbackChain header_font_chain =
      font_database.resolve_chain(cgpui::FontDescriptor{.family = "Header"});
  cgpui::ScrollModel scroll_model;
  cgpui::ElementTree element_tree;
  const cgpui::ElementId header_root_id =
      element_tree.reconcile_root(cgpui::into_element(cgpui::div()));
  std::vector<cgpui::AnyElement> keyed_children;
  keyed_children.push_back(cgpui::into_element(cgpui::div().key("first")));
  keyed_children.push_back(cgpui::into_element(cgpui::div().key("second")));
  const std::vector<cgpui::ElementId> keyed_child_ids =
      element_tree.reconcile_children(header_root_id, std::move(keyed_children));
  const cgpui::AccessibilityTreeSnapshot accessibility_snapshot =
      element_tree.accessibility_snapshot(cgpui::AccessibilitySnapshotOptions{
          .focused_element_id = header_root_id,
      });
  const cgpui::AccessibilityNode* accessibility_root =
      accessibility_snapshot.node(header_root_id);
  cgpui::ElementLifecycleContext lifecycle_context{
      .element_id = header_root_id,
      .parent_element_id = std::nullopt,
  };
  cgpui::FrameStatistics frame_statistics{
      .frame_index = 1,
      .render_pass_count = 1,
      .layout_pass_count = 1,
      .paint_pass_count = 1,
      .paint_command_count = 5,
      .submitted_command_count = 3,
      .skipped_command_count = 2,
      .solid_rect_command_count = 1,
      .rounded_rect_command_count = 1,
      .text_command_count = 1,
      .begin_frame_count = 1,
      .clear_count = 1,
      .present_count = 1,
  };
  cgpui::RenderRecord render_record{
      .sequence = 1,
      .view_id = cgpui::ViewId{1},
      .viewport_size = cgpui::Size{3.0F, 4.0F},
      .root_element_id = cgpui::ElementId{2},
      .statistics = frame_statistics,
  };
  cgpui::EventRoute event_route{
      .target_view_id = cgpui::ViewId{1},
      .target_element_id = cgpui::ElementId{2},
      .element_ancestry = {cgpui::ElementId{2}},
      .view_ancestry = {cgpui::ViewId{1}},
      .event_kind = cgpui::EventKind::pointer_moved,
  };
  cgpui::ImeCandidateRect ime_rect{
      .element_id = cgpui::ElementId{2},
      .rect = cgpui::Rect{.origin = {4.0F, 5.0F}, .size = {1.0F, 18.0F}},
      .byte_offset = 3,
  };
  cgpui::NativeMenuModel menu_model{
      .items =
          {
              cgpui::NativeMenuItem{
                  .kind = cgpui::NativeMenuItemKind::command,
                  .title = "Open",
                  .action_name = "file.open",
                  .accelerator =
                      cgpui::NativeMenuAccelerator{
                          .key_code = 'O',
                          .modifiers =
                              cgpui::KeyboardModifiers{.control = true},
                      },
              },
          },
  };
  const cgpui::PlatformMenuInstallationResult menu_result{
      .supported = false,
      .backend = "header",
      .menu_count = menu_model.items.size(),
      .item_count = cgpui::native_menu_item_count(menu_model),
      .accelerator_count =
          cgpui::native_menu_accelerator_count(menu_model),
  };
  const cgpui::NativeMenuInstallation menu_installation{
      .model = menu_model,
      .platform = menu_result,
  };
  const cgpui::NativeFileDialogOptions file_dialog_options{
      .kind = cgpui::NativeFileDialogKind::open_file,
      .title = "Open",
      .default_directory = "D:/Projects",
      .filters =
          {
              cgpui::NativeFileDialogFilter{
                  .name = "Images",
                  .extensions = {"png", "jpg"},
              },
          },
  };
  const cgpui::NativeFileDialogResult file_dialog_result{
      .supported = false,
      .accepted = false,
      .backend = "header",
      .kind = file_dialog_options.kind,
      .filter_count = file_dialog_options.filters.size(),
  };
  cgpui::StyleState style_state;
  cgpui::ElementKey element_key{.value = "header-key"};
  style_state.base = cgpui::Style{}
                         .with_background_color(cgpui::rgb(0, 0, 0))
                         .with_align_items(cgpui::AlignItems::center)
                         .with_justify_content(cgpui::JustifyContent::end)
                         .with_layer(2)
                         .with_font(cgpui::FontDescriptor{.family = "Inter"})
                         .with_font_size(15.0F)
                         .with_opacity(0.8F)
                         .with_transform(
                             cgpui::AffineTransform::translation(2.0F, 3.0F));
  style_state.hover =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(64, 64, 64))
          .with_justify_content(cgpui::JustifyContent::space_between)
          .with_font(cgpui::FontDescriptor{.family = "Hover"})
          .with_font_size(17.0F)
          .with_opacity(0.6F)
          .with_transform(
              cgpui::AffineTransform::translation(4.0F, 5.0F));
  const cgpui::Style resolved = cgpui::resolved_style(
      style_state,
      cgpui::StyleStateFlags{.hovered = true});
  cgpui::AppContextSetupCallback app_setup =
      [](cgpui::AppContext& app_context) {
        (void)app_context.runtime.invalidation_state();
        app_context.runtime.register_app_action(
            "header.app",
            [](const cgpui::WindowRuntimeContext&) {
              return cgpui::EventResult::consumed_event();
            });
        app_context.runtime.register_window_action(
            "header.window",
            [](const cgpui::WindowRuntimeContext&) {
              return cgpui::EventResult::consumed_event();
            });
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
        app_context.runtime.register_view_action(
            registered_view_id,
            "header.view",
            [](const cgpui::WindowRuntimeContext&) {
              return cgpui::EventResult::consumed_event();
            });
        cgpui::AnyElement placeholder =
            cgpui::into_element(cgpui::child_view(registered_view_id)
                                    .size(cgpui::Size{5.0F, 6.0F}));
        const auto* child_view_placeholder =
            dynamic_cast<const cgpui::ChildViewElement*>(placeholder.get());
        app_context.runtime.register_focused_element_action(
            cgpui::ElementId{3},
            "header.focused",
            [](const cgpui::WindowRuntimeContext&) {
              return cgpui::EventResult::consumed_event();
            });
        const cgpui::ActionDispatchResult dispatch =
            app_context.runtime.dispatch_action("header.view");
        const cgpui::View* found_view =
            app_context.runtime.find_view(registered_view_id);
        const bool removed_view =
            app_context.runtime.remove_view(registered_view_id);
        (void)app_context.runtime.root_view();
        (void)opened_root;
        (void)child_view_placeholder;
        (void)found_view;
        (void)removed_view;
        (void)dispatch.scope;
        (void)dispatch.view_id;
        (void)dispatch.element_id;
        (void)cgpui::ActionScope::app;
        (void)cgpui::ActionScope::window;
        (void)cgpui::ActionScope::view;
        (void)cgpui::ActionScope::focused_element;
      };
  const cgpui::WindowDescriptor window_descriptor =
      cgpui::WindowOptions{}
          .title("Header Window")
          .size(cgpui::Size{9.0F, 7.0F})
          .titlebar_visible(false)
          .decorations(false)
          .resizable(false)
          .transparent(true)
          .to_descriptor();
  cgpui::AppRunnerOptions app_options;
  app_options.setup_context = app_setup;
  cgpui::AnyElement element =
      cgpui::into_element(cgpui::div()
                              .key(element_key)
                              .size(cgpui::px(1.0F), cgpui::px(2.0F))
                              .padding(cgpui::edges(cgpui::px(1.0F)))
                              .background(cgpui::rgb(255, 0, 0))
                              .foreground(cgpui::rgba(255, 255, 255, 0.5F))
                              .font(cgpui::FontDescriptor{.family = "UI"})
                              .font_size(16.0F)
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
                              .key("header-flex")
                              .align_items(cgpui::AlignItems::center)
                              .justify_content(cgpui::JustifyContent::end)
                              .child(cgpui::div()
                                        .size(1.0F, 1.0F)
                                        .flex_grow(1.0F)
                                        .flex_shrink(2.0F)
                                        .layer(4)
                                        .absolute()
                                        .inset(cgpui::edges(3.0F))));
  const auto* flex = dynamic_cast<const cgpui::FlexElement*>(flex_element.get());
  cgpui::AnyElement text_element =
      cgpui::into_element(cgpui::text(text_model)
                              .font(cgpui::FontDescriptor{.family = "Text"})
                              .font_size(19.0F));
  const auto* built_text =
      dynamic_cast<const cgpui::TextElement*>(text_element.get());
  cgpui::AnyElement label_element =
      cgpui::label("Header Label")
          .foreground(cgpui::rgb(210, 220, 230))
          .font(cgpui::FontDescriptor{.family = "Label"})
          .font_size(13.0F)
          .key("header-label")
          .build();
  const auto* label =
      dynamic_cast<const cgpui::LabelElement*>(label_element.get());
  cgpui::AnyElement text_input_element =
      cgpui::text_input(text_model)
          .foreground(cgpui::rgb(230, 235, 240))
          .font(cgpui::FontDescriptor{.family = "Input"})
          .font_size(16.0F)
          .key("header-input")
          .build();
  const auto* text_input =
      dynamic_cast<const cgpui::TextInputElement*>(text_input_element.get());
  cgpui::ScrollState scroll_state;
  cgpui::AnyElement scroll_element = cgpui::scroll(
      scroll_state,
      cgpui::div().size(cgpui::Size{7.0F, 8.0F}));
  const auto* scroll =
      dynamic_cast<const cgpui::ScrollElement*>(scroll_element.get());
  cgpui::ScrollState list_scroll_state;
  cgpui::AnyElement list_element =
      cgpui::scrollable_list(list_scroll_state)
          .size(12.0F, 9.0F)
          .item("first", cgpui::div().size(12.0F, 4.0F))
          .item("second", cgpui::div().size(12.0F, 4.0F))
          .build();
  const auto* list =
      dynamic_cast<const cgpui::ScrollableListElement*>(list_element.get());
  cgpui::AnyElement button_element =
      cgpui::button("header.accept")
          .style(cgpui::Style{}.with_preferred_size(
              cgpui::Size{12.0F, 5.0F}))
          .on_click([](const cgpui::ElementEventContext& event_context) {
            return event_context.dispatch_action
                       ? event_context.dispatch_action("header.click")
                       : cgpui::EventResult::unhandled();
          })
          .build();
  const auto* button =
      dynamic_cast<const cgpui::ButtonElement*>(button_element.get());
  const auto* pointer =
      dynamic_cast<const cgpui::PointerElement*>(element.get());
  const auto* styled = pointer == nullptr
      ? nullptr
      : dynamic_cast<const cgpui::StyledElement*>(pointer->child());
  scroll_model.set_viewport_size(cgpui::Size{10.0F, 10.0F});
  text_model.insert_text("x");
  view.paint(paint_list, cgpui::Size{100.0F, 100.0F});
  const std::span<const cgpui::PaintCommand> commands = paint_list.commands();
  if (commands.size() != 5) {
    return 1;
  }
  const cgpui::RoundedRect& rounded = commands[1].rounded_rect;
  const cgpui::TextPaint& text = commands[2].text;
  const cgpui::TextSelectionPaint& selection = commands[3].text_selection;
  const cgpui::TextCaretPaint& caret = commands[4].text_caret;
  return commands[0].kind == cgpui::PaintCommandKind::solid_rect &&
                 commands[1].kind == cgpui::PaintCommandKind::rounded_rect &&
                 commands[2].kind == cgpui::PaintCommandKind::text &&
                 commands[3].kind ==
                     cgpui::PaintCommandKind::text_selection &&
                 commands[4].kind == cgpui::PaintCommandKind::text_caret &&
                 rounded.radius.top_left == 2.0F && text_model.text() == "x" &&
                 text.content == "header" && text.byte_length == 6 &&
                 text.font.family == "Header" && text.font_size == 18.0F &&
                 text.glyphs.size() == 6 &&
                 text.glyphs[0].key.font_family == "Header" &&
                 text.glyphs[0].advance == 9.0F &&
                 shape_run.glyph_count() == 2 &&
                 shape_run.total_advance == 18.0F &&
                 !header_rasterized.bitmap.empty() &&
                 header_rasterized.bitmap.width == 9 &&
                 header_rasterized.key.font_family == "Header" &&
                 !header_font_chain.empty() &&
                 header_font_chain.primary()->font.family == "Header" &&
                 header_font_chain.size() == 2 &&
                 header_font_chain.faces()[1]->font.family == "Fallback" &&
                 selection.range.start == 1 && selection.range.end == 3 &&
                 selection.rect.size.width == 5.0F &&
                 caret.byte_offset == 3 && caret.rect.size.width == 1.0F &&
                 ime_rect.byte_offset == 3 &&
                 keyed_child_ids.size() == 2 &&
                 keyed_child_ids[0].value != 0 &&
                 accessibility_snapshot.root_element_id == header_root_id &&
                 accessibility_root != nullptr &&
                 accessibility_root->role ==
                     cgpui::AccessibilityRole::generic &&
                 accessibility_root->focused &&
                 element_tree.children(header_root_id).size() == 2 &&
                 lifecycle_context.element_id == header_root_id &&
                 !lifecycle_context.parent_element_id.has_value() &&
                 frame_statistics.paint_command_count == 5 &&
                 frame_statistics.rounded_rect_command_count == 1 &&
                 render_record.statistics.has_value() &&
                 render_record.statistics->submitted_command_count == 3 &&
                 ime_rect.element_id == cgpui::ElementId{2} &&
                 ime_rect.rect.size.height == 18.0F &&
                 render_record.sequence == 1 &&
                 render_record.root_element_id.has_value() &&
                 event_route.element_ancestry.size() == 1 &&
                 event_route.view_ancestry.size() == 1 &&
                 menu_installation.platform.accelerator_count == 1 &&
                 menu_installation.model.items[0].action_name == "file.open" &&
                 file_dialog_result.filter_count == 1 &&
                 file_dialog_result.kind ==
                     cgpui::NativeFileDialogKind::open_file &&
                 styled != nullptr && styled->style().padding.top == 1.0F &&
                 pointer->key().has_value() &&
                 pointer->key()->value == element_key.value &&
                 flex_element->key().has_value() &&
                 flex_element->key()->value == "header-flex" &&
                 resolved.background_color.has_value() &&
                 resolved.justify_content ==
                     cgpui::JustifyContent::space_between &&
                 resolved.font.family == "Hover" &&
                 resolved.font_size == 17.0F &&
                 static_cast<bool>(app_options.setup_context) &&
                 window_descriptor.title == "Header Window" &&
                 window_descriptor.size.width == 9.0F &&
                 !window_descriptor.chrome.titlebar_visible &&
                 !window_descriptor.chrome.decorations &&
                 !window_descriptor.chrome.resizable &&
                 window_descriptor.chrome.transparent_background &&
                 resolved.background_color->r == 64.0F / 255.0F &&
                 styled->style_state().hover.background_color.has_value() &&
                 styled->style().preferred_size.width == 1.0F &&
                 styled->style().background_color.has_value() &&
                 styled->style().background_color->r == 1.0F &&
                  styled->style().foreground_color.has_value() &&
                  styled->style().foreground_color->a == 0.5F &&
                  scroll != nullptr && scroll->state() == &scroll_state &&
                  list != nullptr && list->state() == &list_scroll_state &&
                  list->item_count() == 2 &&
                  list->content().children()[0]->key().has_value() &&
                  flex != nullptr &&
                  flex->align_items() == cgpui::AlignItems::center &&
                  flex->justify_content() == cgpui::JustifyContent::end &&
                  flex->children()[0]->flex_grow() == 1.0F &&
                  flex->children()[0]->flex_shrink() == 2.0F &&
                  flex->children()[0]->layer() == 4 &&
                  flex->children()[0]->z_order() == 4 &&
                  flex->children()[0]->position() ==
                      cgpui::Position::absolute &&
                  flex->children()[0]->inset().left == 3.0F &&
                  built_text != nullptr &&
                  built_text->font().family == "Text" &&
                  built_text->font_size() == 19.0F &&
                  label != nullptr && label->text() == "Header Label" &&
                  label->font().family == "Label" &&
                  label->font_size() == 13.0F &&
                  label_element->key().has_value() &&
                  label_element->key()->value == "header-label" &&
                  text_input != nullptr &&
                  text_input->model() == &text_model &&
                  text_input->focusable() &&
                  text_input->font().family == "Input" &&
                  text_input_element->key().has_value() &&
                  text_input_element->key()->value == "header-input" &&
                  button != nullptr &&
                  button->action_name() == "header.accept" &&
                  button->focusable() &&
                  scroll_model.offset().x == 0.0F
              ? 0
              : 1;
}
