#include "macos_input_internal.hpp"
#include "macos_window_backdrop_internal.hpp"
#include "macos_window_internal.hpp"

#include <dispatch/dispatch.h>

#include <optional>

namespace cgpui {
namespace {

struct MaterialChoice {
  NSVisualEffectMaterial material;
  const char* name;
};

MaterialChoice material_for(WindowBackdrop backdrop) {
  switch (backdrop) {
    case WindowBackdrop::mica_alt:
      return {NSVisualEffectMaterialUnderWindowBackground, "underWindowBackground"};
    case WindowBackdrop::acrylic:
      return {NSVisualEffectMaterialPopover, "popover"};
    case WindowBackdrop::blur:
      return {NSVisualEffectMaterialSidebar, "sidebar"};
    case WindowBackdrop::none:
    case WindowBackdrop::mica:
      break;
  }
  return {NSVisualEffectMaterialWindowBackground, "windowBackground"};
}

}  // namespace

MacOSBackdropPlan macos_plan_window_backdrop(
    WindowBackdrop requested,
    bool transparent_surface,
    bool reduce_transparency) {
  if (requested == WindowBackdrop::none) return {};
  if (reduce_transparency) {
    return {.reason = "Reduce transparency is on in System Settings"};
  }
  if (!transparent_surface) {
    return {.reason = "renderer surface is opaque; the material would be "
                      "hidden behind it"};
  }
  const MaterialChoice choice = material_for(requested);
  return {.applied = requested,
          .material = choice.material,
          .mechanism = std::string("NSVisualEffectView material=") +
                       choice.name + " blending=behindWindow"};
}

PlatformWindowBackdropState MacOSWindow::apply_window_backdrop(
    WindowBackdrop backdrop, bool transparent_surface) {
  if (![NSThread isMainThread]) {
    __block std::optional<PlatformWindowBackdropState> result;
    dispatch_sync(dispatch_get_main_queue(), ^{
      result = apply_window_backdrop(backdrop, transparent_surface);
    });
    return std::move(*result);
  }
  PlatformWindowBackdropState state{
      .supported = window_ != nil,
      .transparent_surface = transparent_surface,
      .backend = "macos",
      .requested = backdrop};
  if (window_ == nil) {
    state.reason = "native window closed";
    return state;
  }
  const MacOSBackdropPlan plan = macos_plan_window_backdrop(
      backdrop,
      transparent_surface,
      [[NSWorkspace sharedWorkspace] accessibilityDisplayShouldReduceTransparency]);
  if (plan.applied == WindowBackdrop::none) {
    remove_backdrop_view();
  } else {
    install_backdrop_view(plan.material);
  }
  // Reduce Transparency can change while a material is requested.
  observe_backdrop_settings(backdrop != WindowBackdrop::none);
  state.applied = plan.applied;
  state.mechanism = plan.mechanism;
  state.reason = plan.reason;
  backdrop_state_ = state;
  return state;
}

PlatformWindowBackdropState MacOSWindow::window_backdrop_state() const {
  return backdrop_state_;
}

void MacOSWindow::install_backdrop_view(NSVisualEffectMaterial material) {
  if (backdrop_view_ == nil) {
    NSVisualEffectView* effect =
        [[NSVisualEffectView alloc] initWithFrame:[content_view_ frame]];
    effect.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    effect.state = NSVisualEffectStateFollowsWindowActiveState;
    const bool responder = [window_ firstResponder] == content_view_;
    [window_ setContentView:effect];
    content_view_.frame = effect.bounds;
    content_view_.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [effect addSubview:content_view_];
    if (responder) [window_ makeFirstResponder:content_view_];
    backdrop_view_ = effect;
  }
  backdrop_view_.material = material;
}

void MacOSWindow::remove_backdrop_view() {
  if (backdrop_view_ == nil) return;
  const bool responder = [window_ firstResponder] == content_view_;
  [content_view_ removeFromSuperview];
  [window_ setContentView:content_view_];
  if (responder) [window_ makeFirstResponder:content_view_];
  backdrop_view_ = nil;
}

void MacOSWindow::observe_backdrop_settings(bool observe) {
  NSNotificationCenter* center =
      [[NSWorkspace sharedWorkspace] notificationCenter];
  if (!observe) {
    if (backdrop_observer_ != nil) [center removeObserver:backdrop_observer_];
    backdrop_observer_ = nil;
    return;
  }
  if (backdrop_observer_ != nil) return;
  MacOSWindow* window = this;
  backdrop_observer_ = [center
      addObserverForName:NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification
                  object:nil
                   queue:[NSOperationQueue mainQueue]
              usingBlock:^(NSNotification*) {
                const PlatformWindowBackdropState previous =
                    window->window_backdrop_state();
                (void)window->apply_window_backdrop(
                    previous.requested, previous.transparent_surface);
                window->request_redraw();
              }];
}

}  // namespace cgpui
