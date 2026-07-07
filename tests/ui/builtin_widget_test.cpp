#include "cgpui/prelude.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace {

cgpui::ImageAsset make_test_image_asset(std::uint64_t id) {
  return cgpui::ImageAsset{
      .id = cgpui::ImageAssetId{id},
      .logical_size = {.width = 20.0F, .height = 12.0F},
      .bitmap =
          cgpui::DecodedImageBitmap{
              .width = 4,
              .height = 3,
              .stride = 16,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .pixels = std::vector<std::uint8_t>(48, 255),
          },
  };
}

bool same_color(cgpui::Color left, cgpui::Color right) {
  return left.r == right.r && left.g == right.g && left.b == right.b &&
         left.a == right.a;
}

} // namespace

int test_button_label_convenience_builds_accessible_button() {
  cgpui::AnyElement element =
      cgpui::button("dialog.confirm")
          .label("Confirm")
          .style(cgpui::Style{}.with_padding(cgpui::edges(4.0F)))
          .key("confirm-button")
          .build();

  const auto* button = dynamic_cast<const cgpui::ButtonElement*>(element.get());
  if (button == nullptr || button->action_name() != "dialog.confirm") {
    return 1;
  }
  if (!button->focusable() || !button->key().has_value() ||
      button->key()->value != "confirm-button") {
    return 2;
  }
  const auto* label = dynamic_cast<const cgpui::LabelElement*>(button->child());
  if (label == nullptr || label->text() != "Confirm") {
    return 3;
  }
  if (button->accessibility_name() != "Confirm") {
    return 4;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  const std::optional<cgpui::Rect> button_bounds = button->layout_bounds();
  const std::optional<cgpui::Rect> label_bounds = label->layout_bounds();
  if (!button_bounds.has_value() || !label_bounds.has_value()) {
    return 5;
  }
  if (output.size.width != 64.0F || output.size.height != 24.0F ||
      label_bounds->origin.x != 4.0F || label_bounds->origin.y != 4.0F ||
      label_bounds->size.width != 56.0F || label_bounds->size.height != 16.0F) {
    return 6;
  }
  return 0;
}

int test_checkbox_widget_builder_tracks_checked_state() {
  int click_count = 0;
  std::string_view dispatched_action;
  cgpui::AnyElement element =
      cgpui::checkbox("settings.email")
          .label("Email updates")
          .checked(true)
          .on_click([&](const cgpui::ElementEventContext&) {
            click_count += 1;
            return cgpui::EventResult::unhandled();
          })
          .key("email-checkbox")
          .build();

  auto* checkbox = dynamic_cast<cgpui::ToggleControlElement*>(element.get());
  if (checkbox == nullptr ||
      checkbox->kind() != cgpui::ToggleControlKind::checkbox ||
      checkbox->accessibility_role() != cgpui::AccessibilityRole::checkbox) {
    return 20;
  }
  if (!checkbox->checked() || checkbox->accessibility_value() != "checked" ||
      checkbox->accessibility_name() != "Email updates") {
    return 21;
  }
  if (!checkbox->focusable() || !checkbox->key().has_value() ||
      checkbox->key()->value != "email-checkbox") {
    return 22;
  }

  cgpui::ElementEventContext click_context{.target_element_id = checkbox->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  click_context.dispatch_action = [&](std::string_view action) {
    dispatched_action = action;
    return cgpui::EventResult::consumed_event();
  };
  const cgpui::EventResult result = checkbox->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 2.0F, .y = 2.0F}},
      click_context);
  if (click_count != 1 || dispatched_action != "settings.email" ||
      !result.consumed || result.cancelled) {
    return 23;
  }
  return !checkbox->checked() && checkbox->accessibility_value() == "unchecked"
             ? 0
             : 24;
}

int test_radio_widget_builder_selects_without_unselecting() {
  cgpui::AnyElement element =
      cgpui::radio("theme.dark")
          .label("Dark theme")
          .selected(false)
          .key("dark-radio")
          .build();

  auto* radio = dynamic_cast<cgpui::ToggleControlElement*>(element.get());
  if (radio == nullptr ||
      radio->kind() != cgpui::ToggleControlKind::radio ||
      radio->accessibility_role() != cgpui::AccessibilityRole::radio) {
    return 30;
  }
  if (radio->selected() || radio->accessibility_value() != "unchecked" ||
      radio->accessibility_name() != "Dark theme") {
    return 31;
  }

  cgpui::ElementEventContext click_context{.target_element_id = radio->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  (void)radio->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 2.0F, .y = 2.0F}},
      click_context);
  (void)radio->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 2.0F, .y = 2.0F}},
      click_context);
  return radio->selected() && radio->accessibility_value() == "checked" ? 0
                                                                        : 32;
}

int test_switch_widget_builder_uses_on_off_value_and_disabled_state() {
  cgpui::AnyElement element =
      cgpui::toggle_switch("network.airplane")
          .label("Airplane mode")
          .on(false)
          .disabled()
          .key("airplane-switch")
          .build();

  auto* toggle = dynamic_cast<cgpui::ToggleControlElement*>(element.get());
  if (toggle == nullptr ||
      toggle->kind() != cgpui::ToggleControlKind::toggle_switch ||
      toggle->accessibility_role() != cgpui::AccessibilityRole::switch_control) {
    return 40;
  }
  if (toggle->on() || toggle->accessibility_value() != "off" ||
      toggle->accessibility_name() != "Airplane mode") {
    return 41;
  }
  if (toggle->enabled() || !toggle->key().has_value() ||
      toggle->key()->value != "airplane-switch" || !toggle->focusable()) {
    return 42;
  }

  cgpui::ElementEventContext click_context{.target_element_id = toggle->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  const cgpui::EventResult result = toggle->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 2.0F, .y = 2.0F}},
      click_context);
  return !toggle->on() && !result.consumed && !result.cancelled ? 0 : 43;
}

int test_slider_widget_builder_tracks_range_value_and_click_updates() {
  float changed_value = -1.0F;
  std::string_view dispatched_action;
  cgpui::AnyElement element =
      cgpui::slider("audio.volume")
          .style(cgpui::Style{}.with_preferred_size({.width = 100.0F,
                                                     .height = 20.0F}))
          .range(0.0F, 100.0F)
          .value(25.0F)
          .step(5.0F)
          .on_change([&](float value, const cgpui::ElementEventContext&) {
            changed_value = value;
            return cgpui::EventResult::unhandled();
          })
          .key("volume-slider")
          .build();

  auto* slider = dynamic_cast<cgpui::SliderElement*>(element.get());
  if (slider == nullptr ||
      slider->accessibility_role() != cgpui::AccessibilityRole::slider ||
      slider->action_name() != "audio.volume") {
    return 50;
  }
  if (slider->min() != 0.0F || slider->max() != 100.0F ||
      slider->value() != 25.0F || slider->step() != 5.0F ||
      slider->accessibility_value() != "25") {
    return 51;
  }
  if (!slider->focusable() || !slider->key().has_value() ||
      slider->key()->value != "volume-slider") {
    return 52;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 100.0F || output.size.height != 20.0F) {
    return 53;
  }

  cgpui::ElementEventContext click_context{.target_element_id = slider->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  click_context.dispatch_action = [&](std::string_view action) {
    dispatched_action = action;
    return cgpui::EventResult::consumed_event();
  };
  const cgpui::EventResult result = slider->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 72.0F, .y = 10.0F}},
      click_context);
  if (changed_value != 70.0F || dispatched_action != "audio.volume" ||
      !result.consumed || result.cancelled) {
    return 54;
  }
  return slider->value() == 70.0F && slider->accessibility_value() == "70" ? 0
                                                                           : 55;
}

int test_slider_widget_builder_paints_and_ignores_disabled_input() {
  cgpui::AnyElement element =
      cgpui::slider("timeline.scrub")
          .style(cgpui::Style{}.with_preferred_size({.width = 80.0F,
                                                     .height = 16.0F}))
          .range(-1.0F, 1.0F)
          .value(0.0F)
          .disabled()
          .build();

  auto* slider = dynamic_cast<cgpui::SliderElement*>(element.get());
  if (slider == nullptr || slider->enabled() ||
      slider->accessibility_value() != "0") {
    return 60;
  }
  (void)element->layout(cgpui::LayoutInput{});
  cgpui::PaintList paint_list;
  slider->paint(paint_list);
  if (paint_list.commands().size() < 3) {
    return 61;
  }

  cgpui::ElementEventContext click_context{.target_element_id = slider->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  const cgpui::EventResult result = slider->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 80.0F, .y = 8.0F}},
      click_context);
  return slider->value() == 0.0F && !result.consumed && !result.cancelled ? 0
                                                                          : 62;
}

int test_list_item_widget_builder_tracks_selection_and_clicks() {
  int click_count = 0;
  std::string_view dispatched_action;
  cgpui::AnyElement element =
      cgpui::list_item("sidebar.inbox")
          .label("Inbox")
          .selected(true)
          .on_click([&](const cgpui::ElementEventContext&) {
            click_count += 1;
            return cgpui::EventResult::unhandled();
          })
          .key("inbox-item")
          .build();

  auto* item = dynamic_cast<cgpui::ItemElement*>(element.get());
  if (item == nullptr || item->kind() != cgpui::ItemKind::list_item ||
      item->accessibility_role() != cgpui::AccessibilityRole::list_item ||
      item->action_name() != "sidebar.inbox") {
    return 70;
  }
  if (!item->selected() || item->accessibility_value() != "selected" ||
      item->accessibility_name() != "Inbox") {
    return 71;
  }
  if (!item->focusable() || !item->key().has_value() ||
      item->key()->value != "inbox-item") {
    return 72;
  }

  cgpui::ElementEventContext click_context{.target_element_id = item->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  click_context.dispatch_action = [&](std::string_view action) {
    dispatched_action = action;
    return cgpui::EventResult::consumed_event();
  };
  const cgpui::EventResult result = item->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 4.0F, .y = 4.0F}},
      click_context);
  if (click_count != 1 || dispatched_action != "sidebar.inbox" ||
      !result.consumed || result.cancelled) {
    return 73;
  }
  return item->selected() && item->accessibility_value() == "selected" ? 0
                                                                       : 74;
}

int test_menu_item_widget_builder_paints_and_ignores_disabled_input() {
  int click_count = 0;
  cgpui::AnyElement element =
      cgpui::menu_item("file.open")
          .label("Open...")
          .disabled()
          .on_click([&](const cgpui::ElementEventContext&) {
            click_count += 1;
            return cgpui::EventResult::consumed_event();
          })
          .build();

  auto* item = dynamic_cast<cgpui::ItemElement*>(element.get());
  if (item == nullptr || item->kind() != cgpui::ItemKind::menu_item ||
      item->accessibility_role() != cgpui::AccessibilityRole::menu_item ||
      item->accessibility_name() != "Open...") {
    return 80;
  }
  if (item->selected() || item->accessibility_value() != "" ||
      item->enabled() || !item->focusable()) {
    return 81;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 56.0F || output.size.height != 16.0F) {
    return 82;
  }
  cgpui::PaintList paint_list;
  item->paint(paint_list);
  if (paint_list.commands().empty()) {
    return 83;
  }

  cgpui::ElementEventContext click_context{.target_element_id = item->id()};
  click_context.gesture = cgpui::ElementGestureKind::click;
  const cgpui::EventResult result = item->handle_event(
      cgpui::PointerButton{
          .button = cgpui::MouseButton::left,
          .pressed = false,
          .position = {.x = 4.0F, .y = 4.0F}},
      click_context);
  return click_count == 0 && !result.consumed && !result.cancelled ? 0 : 84;
}

int test_image_widget_builder_paints_asset_and_source_rect() {
  const cgpui::ImageAsset asset = make_test_image_asset(101);
  const cgpui::ImageAssetDescriptor descriptor =
      cgpui::describe_image_asset(asset);
  const cgpui::Rect source{
      .origin = {.x = 1.0F, .y = 2.0F},
      .size = {.width = 3.0F, .height = 4.0F},
  };
  cgpui::AnyElement element =
      cgpui::image(descriptor)
          .alt("Preview")
          .source_rect(source)
          .style(cgpui::Style{}.with_preferred_size({.width = 40.0F,
                                                     .height = 24.0F}))
          .key("preview-image")
          .build();

  auto* image = dynamic_cast<cgpui::ImageElement*>(element.get());
  if (image == nullptr ||
      image->kind() != cgpui::ImageElementKind::image ||
      image->asset().id.value != 101 ||
      image->accessibility_role() != cgpui::AccessibilityRole::image) {
    return 90;
  }
  if (image->accessibility_name() != "Preview" ||
      image->accessibility_value() != "" || image->focusable() ||
      !image->key().has_value() || image->key()->value != "preview-image") {
    return 91;
  }
  if (!image->source_rect().has_value() ||
      image->source_rect()->origin.x != 1.0F ||
      image->source_rect()->size.height != 4.0F) {
    return 92;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 24.0F) {
    return 93;
  }
  cgpui::PaintList paint_list;
  image->paint(paint_list);
  if (paint_list.commands().size() != 1 ||
      paint_list.commands()[0].kind != cgpui::PaintCommandKind::image) {
    return 94;
  }
  const cgpui::ImagePaint& command = paint_list.commands()[0].image;
  if (command.asset.id.value != 101 || command.asset.pixel_width != 4 ||
      command.bounds.size.width != 40.0F ||
      command.bounds.size.height != 24.0F || !command.source_rect.has_value() ||
      command.source_rect->origin.y != 2.0F || command.tint.has_value()) {
    return 95;
  }
  return 0;
}

int test_icon_widget_builder_uses_square_size_and_tint() {
  const cgpui::ImageAsset asset = make_test_image_asset(202);
  const cgpui::Color tint = cgpui::rgb(24, 96, 160);
  cgpui::AnyElement element =
      cgpui::icon(asset).alt("Search").size(16.0F).tint(tint).build();

  auto* icon = dynamic_cast<cgpui::ImageElement*>(element.get());
  if (icon == nullptr || icon->kind() != cgpui::ImageElementKind::icon ||
      icon->asset().id.value != 202 ||
      icon->accessibility_role() != cgpui::AccessibilityRole::image ||
      icon->accessibility_name() != "Search") {
    return 100;
  }
  if (!icon->tint().has_value() || !same_color(*icon->tint(), tint) ||
      icon->source_rect().has_value()) {
    return 101;
  }

  const cgpui::LayoutOutput output = element->layout(cgpui::LayoutInput{});
  if (output.size.width != 16.0F || output.size.height != 16.0F) {
    return 102;
  }
  cgpui::PaintList paint_list;
  icon->paint(paint_list);
  if (paint_list.commands().size() != 1 ||
      paint_list.commands()[0].kind != cgpui::PaintCommandKind::image) {
    return 103;
  }
  const cgpui::ImagePaint& command = paint_list.commands()[0].image;
  return command.asset.id.value == 202 && command.tint.has_value() &&
                 same_color(*command.tint, tint) &&
                 command.bounds.size.width == 16.0F &&
                 command.bounds.size.height == 16.0F
             ? 0
             : 104;
}

int test_text_input_widget_builder_keeps_text_model_boundary() {
  cgpui::TextModel model("typed");
  cgpui::AnyElement element =
      cgpui::text_input(model).key("primary-input").disabled().build();

  const auto* input =
      dynamic_cast<const cgpui::TextInputElement*>(element.get());
  if (input == nullptr || input->model() != &model ||
      input->accessibility_role() != cgpui::AccessibilityRole::text_input) {
    return 10;
  }
  if (input->enabled() || !input->key().has_value() ||
      input->key()->value != "primary-input") {
    return 11;
  }
  if (!input->focusable() || input->text() != "typed") {
    return 12;
  }
  return 0;
}

int main() {
  if (const int result =
          test_button_label_convenience_builds_accessible_button();
      result != 0) {
    return result;
  }
  if (const int result = test_checkbox_widget_builder_tracks_checked_state();
      result != 0) {
    return result;
  }
  if (const int result = test_radio_widget_builder_selects_without_unselecting();
      result != 0) {
    return result;
  }
  if (const int result =
          test_switch_widget_builder_uses_on_off_value_and_disabled_state();
      result != 0) {
    return result;
  }
  if (const int result =
          test_slider_widget_builder_tracks_range_value_and_click_updates();
      result != 0) {
    return result;
  }
  if (const int result =
          test_slider_widget_builder_paints_and_ignores_disabled_input();
      result != 0) {
    return result;
  }
  if (const int result =
          test_list_item_widget_builder_tracks_selection_and_clicks();
      result != 0) {
    return result;
  }
  if (const int result =
          test_menu_item_widget_builder_paints_and_ignores_disabled_input();
      result != 0) {
    return result;
  }
  if (const int result =
          test_image_widget_builder_paints_asset_and_source_rect();
      result != 0) {
    return result;
  }
  if (const int result =
          test_icon_widget_builder_uses_square_size_and_tint();
      result != 0) {
    return result;
  }
  return test_text_input_widget_builder_keeps_text_model_boundary();
}
