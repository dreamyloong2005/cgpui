#include "macos_window_backdrop_internal.hpp"
#include "macos_window_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

namespace {

using cgpui::WindowBackdrop;

int test_plan() {
  if (cgpui::macos_plan_window_backdrop(WindowBackdrop::none, true, false).applied !=
          WindowBackdrop::none ||
      !cgpui::macos_plan_window_backdrop(WindowBackdrop::none, true, false)
           .reason.empty()) {
    return 1;
  }
  const auto reduced =
      cgpui::macos_plan_window_backdrop(WindowBackdrop::mica, true, true);
  const auto opaque =
      cgpui::macos_plan_window_backdrop(WindowBackdrop::mica, false, false);
  if (reduced.applied != WindowBackdrop::none || reduced.reason.empty() ||
      opaque.applied != WindowBackdrop::none || opaque.reason.empty()) {
    return 2;
  }
  const struct {
    WindowBackdrop request;
    NSVisualEffectMaterial material;
  } table[]{
      {WindowBackdrop::mica, NSVisualEffectMaterialWindowBackground},
      {WindowBackdrop::mica_alt, NSVisualEffectMaterialUnderWindowBackground},
      {WindowBackdrop::acrylic, NSVisualEffectMaterialPopover},
      {WindowBackdrop::blur, NSVisualEffectMaterialSidebar},
  };
  for (const auto& row : table) {
    const auto plan = cgpui::macos_plan_window_backdrop(row.request, true, false);
    if (plan.applied != row.request || plan.material != row.material ||
        !plan.reason.empty() ||
        plan.mechanism.find("NSVisualEffectView") != 0) {
      return 3;
    }
  }
  return 0;
}

}  // namespace

int main() {
  @autoreleasepool {
    if (const int plan = test_plan(); plan != 0) return plan;
    auto application = cgpui::create_platform_application();
    if (!application) return 10;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{.title = "CGPUI macOS Backdrop Test",
                                .size = {240.0F, 160.0F}},
        [](const cgpui::PlatformEvent&) {});
    if (!window) return 11;
    auto* macos = dynamic_cast<cgpui::MacOSWindow*>(window->get());
    if (macos == nullptr) return 12;
    NSWindow* native = macos->native_window();
    NSView* content = [native contentView];
    const cgpui::Size framebuffer = (*window)->state().framebuffer_size;

    const auto initial = (*window)->window_backdrop_state();
    if (!initial.supported || initial.backend != "macos" ||
        initial.applied != WindowBackdrop::none) {
      return 13;
    }
    const auto opaque = (*window)->apply_window_backdrop(WindowBackdrop::mica, false);
    if (!opaque.supported || opaque.applied != WindowBackdrop::none ||
        opaque.reason.empty() || [native contentView] != content) {
      return 14;
    }

    const bool reduced = [[NSWorkspace sharedWorkspace]
        accessibilityDisplayShouldReduceTransparency];
    const auto mica = (*window)->apply_window_backdrop(WindowBackdrop::mica, true);
    if (reduced) {
      // The material is withheld while Reduce Transparency is on.
      return mica.applied == WindowBackdrop::none && !mica.reason.empty() &&
                     [native contentView] == content
                 ? 0
                 : 15;
    }
    auto* effect = (NSVisualEffectView*)[native contentView];
    if (mica.applied != WindowBackdrop::mica || !mica.transparent_surface ||
        ![effect isKindOfClass:[NSVisualEffectView class]] ||
        effect.material != NSVisualEffectMaterialWindowBackground ||
        effect.blendingMode != NSVisualEffectBlendingModeBehindWindow ||
        [[effect subviews] count] != 1 || [effect subviews][0] != content ||
        !NSEqualRects([content frame], [effect bounds]) ||
        [native firstResponder] != content ||
        (*window)->window_backdrop_state().applied != WindowBackdrop::mica) {
      return 16;
    }
    const cgpui::Size wrapped = (*window)->state().framebuffer_size;
    if (wrapped.width != framebuffer.width || wrapped.height != framebuffer.height) {
      return 17;
    }

    // Another material reuses the same view; the settings notification
    // re-applies the request in place.
    const auto blur = (*window)->apply_window_backdrop(WindowBackdrop::blur, true);
    [[[NSWorkspace sharedWorkspace] notificationCenter]
        postNotificationName:NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification
                      object:[NSWorkspace sharedWorkspace]];
    if (blur.applied != WindowBackdrop::blur || [native contentView] != effect ||
        effect.material != NSVisualEffectMaterialSidebar ||
        (*window)->window_backdrop_state().applied != WindowBackdrop::blur) {
      return 18;
    }

    const auto none = (*window)->apply_window_backdrop(WindowBackdrop::none, true);
    return none.applied == WindowBackdrop::none && none.reason.empty() &&
                   [native contentView] == content &&
                   [native firstResponder] == content
               ? 0
               : 19;
  }
}
