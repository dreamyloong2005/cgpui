#include "cgpui/prelude.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace {

struct CapabilityGlobal {
  int value = 0;
};

class AppContextCapabilityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<AppContextCapabilityView>& context) override {
    const cgpui::AppContext app_context = context.app_context();
    const cgpui::App app = app_context.app();
    const cgpui::Window root_window = app.root_window();
    if (root_window.runtime_id().value == 0) {
      return nullptr;
    }

    app_context.set_global(CapabilityGlobal{.value = 4});
    if (!app_context.update_global<CapabilityGlobal>(
            [](CapabilityGlobal& global) {
              global.value += 3;
            })) {
      return nullptr;
    }
    const CapabilityGlobal* global =
        app_context.global<CapabilityGlobal>();
    if (global == nullptr || global->value != 7) {
      return nullptr;
    }

    cgpui::Theme theme;
    theme.set_color(cgpui::ThemeTokenId{"accent"},
                    cgpui::Color{.r = 0.2F,
                                  .g = 0.4F,
                                  .b = 0.6F,
                                  .a = 1.0F});
    app_context.set_app_theme(theme);

    return cgpui::into_element(cgpui::div().size(1.0F, 1.0F));
  }
};

using ContextRef = cgpui::Context<AppContextCapabilityView>&;

static_assert(std::same_as<decltype(std::declval<ContextRef>().app_context()),
                           cgpui::AppContext>);
static_assert(
    std::same_as<decltype(std::declval<cgpui::AppContext>().app()),
                 cgpui::App>);
static_assert(cgpui::Render<AppContextCapabilityView>);
static_assert(std::is_copy_constructible_v<cgpui::AppContext>);

} // namespace

int main() {
  return 0;
}
