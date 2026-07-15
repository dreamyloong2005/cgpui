#include "macos_accessibility_internal.hpp"
#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cmath>
#include <variant>
#include <vector>

namespace {

cgpui::PlatformAccessibilityTreeUpdate make_update(
    std::string button_name,
    std::vector<cgpui::PlatformAccessibilityLiveUpdate> live_updates = {}) {
  return cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 1,
      .node_count = 6,
      .focused_node_count = 1,
      .nodes = {
          {.element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::generic,
           .name = "Root",
           .bounds = cgpui::Rect{.size = {240.0F, 140.0F}},
           .child_count = 5},
          {.element_id = 2,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::button,
           .name = std::move(button_name),
           .patterns = {.invokable = true},
           .focusable = true,
           .bounds = cgpui::Rect{.origin = {10.0F, 10.0F},
                                 .size = {80.0F, 24.0F}}},
          {.element_id = 3,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::text_input,
           .name = "Editor",
           .text = "hello",
           .value = "hello",
           .patterns = {.value_settable = true},
           .focusable = true,
           .focused = true,
           .bounds = cgpui::Rect{.origin = {10.0F, 45.0F},
                                 .size = {140.0F, 24.0F}}},
          {.element_id = 4,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::checkbox,
           .name = "Enabled",
           .patterns = {.toggled = false}},
          {.element_id = 5,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::slider,
           .name = "Level",
           .patterns = {.range = cgpui::PlatformAccessibilityRangeValue{
                            .value = 5.0,
                            .minimum = 0.0,
                            .maximum = 10.0,
                            .small_change = 1.0,
                            .large_change = 2.0}}},
          {.element_id = 6,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::switch_control,
           .name = "Mode",
           .patterns = {.toggled = true}},
      },
      .live_updates = std::move(live_updates)};
}

}  // namespace

int main() {
  @autoreleasepool {
    auto application = cgpui::create_platform_application();
    if (!application) return 1;
    std::vector<cgpui::AccessibilityActionRequested> actions;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{.title = "CGPUI macOS Accessibility Test",
                                .size = {240.0F, 140.0F}},
        [&](const cgpui::PlatformEvent& event) {
          if (const auto* action =
                  std::get_if<cgpui::AccessibilityActionRequested>(&event)) {
            actions.push_back(*action);
          }
        });
    if (!window) return 2;
    (*window)->update_accessibility_tree(make_update("Launch"));

    auto* native = dynamic_cast<cgpui::MacOSWindow*>(window->get());
    auto* content = (CGPUIMacOSContentView*)[native->native_window() contentView];
    CGPUIMacOSAccessibilityElement* root =
        cgpui::macos_accessibility_root(**window);
    CGPUIMacOSAccessibilityElement* button =
        cgpui::macos_accessibility_provider(**window, 2);
    CGPUIMacOSAccessibilityElement* button_again =
        cgpui::macos_accessibility_provider(**window, 2);
    CGPUIMacOSAccessibilityElement* editor =
        cgpui::macos_accessibility_provider(**window, 3);
    CGPUIMacOSAccessibilityElement* checkbox =
        cgpui::macos_accessibility_provider(**window, 4);
    CGPUIMacOSAccessibilityElement* slider =
        cgpui::macos_accessibility_provider(**window, 5);
    CGPUIMacOSAccessibilityElement* switch_control =
        cgpui::macos_accessibility_provider(**window, 6);
    if (root == nil || button == nil || button != button_again || editor == nil ||
        checkbox == nil || slider == nil || switch_control == nil ||
        [content accessibilityChildren].count != 1 ||
        [content accessibilityFocusedUIElement] != editor) {
      return 3;
    }
    if (![[root accessibilityRole] isEqualToString:NSAccessibilityGroupRole] ||
        [root accessibilityChildren].count != 5 ||
        [button accessibilityParent] != root ||
        ![[button accessibilityRole] isEqualToString:NSAccessibilityButtonRole] ||
        ![[button accessibilityLabel] isEqualToString:@"Launch"] ||
        NSIsEmptyRect([button accessibilityFrame])) {
      return 4;
    }
    if (![[switch_control accessibilityRole]
            isEqualToString:NSAccessibilityCheckBoxRole] ||
        ![[switch_control accessibilitySubrole]
            isEqualToString:NSAccessibilitySwitchSubrole]) {
      return 11;
    }
    if (![[editor accessibilityRole] isEqualToString:NSAccessibilityTextFieldRole] ||
        ![[editor accessibilityValue] isEqual:@"hello"] ||
        ![editor accessibilityFocused] ||
        [editor accessibilityNumberOfCharacters] != 5 ||
        !NSEqualRanges([editor accessibilitySelectedTextRange], NSMakeRange(5, 0))) {
      return 5;
    }

    [button accessibilityPerformAction:NSAccessibilityPressAction];
    [editor setAccessibilityValue:@"world"];
    [checkbox accessibilityPerformAction:NSAccessibilityPressAction];
    [slider accessibilityPerformAction:NSAccessibilityIncrementAction];
    if (actions.size() != 4 ||
        actions[0].kind != cgpui::AccessibilityActionKind::invoke ||
        actions[0].element_id != 2 ||
        actions[1].kind != cgpui::AccessibilityActionKind::set_value ||
        actions[1].value != "world" ||
        actions[2].kind != cgpui::AccessibilityActionKind::toggle ||
        actions[3].kind != cgpui::AccessibilityActionKind::set_range_value ||
        std::fabs(actions[3].numeric_value - 6.0) > 0.01) {
      return 6;
    }

    (*window)->update_accessibility_tree(make_update(
        "Launch Updated",
        {{.kind = cgpui::PlatformAccessibilityLiveUpdateKind::value_changed,
          .element_id = 5,
          .value = "6"},
         {.kind = cgpui::PlatformAccessibilityLiveUpdateKind::text_changed,
          .element_id = 3,
          .text = "hello"},
         {.kind = cgpui::PlatformAccessibilityLiveUpdateKind::focus_changed,
          .element_id = 3,
          .focused = true}}));
    CGPUIMacOSAccessibilityElement* updated_button =
        cgpui::macos_accessibility_provider(**window, 2);
    const auto diagnostics = cgpui::macos_accessibility_diagnostics(**window);
    if (updated_button == nil || updated_button == button ||
        [button accessibilityRole] != nil ||
        [button accessibilityWindow] != nil ||
        [button accessibilityTopLevelUIElement] != nil ||
        ![[updated_button accessibilityLabel] isEqualToString:@"Launch Updated"] ||
        diagnostics.generation != 2 || diagnostics.node_count != 6 ||
        diagnostics.notification_count != 3 ||
        diagnostics.notification_names.size() != 3 ||
        diagnostics.notification_names[0] !=
            NSAccessibilityValueChangedNotification.UTF8String ||
        diagnostics.notification_names[1] !=
            NSAccessibilityValueChangedNotification.UTF8String ||
        diagnostics.notification_names[2] !=
            NSAccessibilityFocusedUIElementChangedNotification.UTF8String) {
      return 7;
    }

    auto second = (*application)->create_window(
        cgpui::WindowDescriptor{.title = "CGPUI macOS Accessibility Second",
                                .size = {180.0F, 100.0F}},
        [](const cgpui::PlatformEvent&) {});
    if (!second) return 8;
    (*second)->update_accessibility_tree(make_update("Second"));
    auto* second_button = cgpui::macos_accessibility_provider(**second, 2);
    if (second_button == nil || second_button == updated_button ||
        ![[second_button accessibilityLabel] isEqualToString:@"Second"] ||
        ![[updated_button accessibilityLabel] isEqualToString:@"Launch Updated"]) {
      return 9;
    }

    CGPUIMacOSAccessibilityElement* retained = updated_button;
    window->reset();
    if ([retained accessibilityRole] != nil || [retained isAccessibilityElement]) {
      return 10;
    }
  }
  return 0;
}
