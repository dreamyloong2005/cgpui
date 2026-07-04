#include "cgpui/cgpui.hpp"
#include "cgpui/ui/render.hpp"

#include <concepts>
#include <memory>

class RenderSpellingView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<RenderSpellingView>& context) override {
    context.request_render();
    return cgpui::into_element(cgpui::div().size(1.0F, 2.0F));
  }
};

static_assert(std::same_as<cgpui::Context<RenderSpellingView>,
                           cgpui::ViewContext>);
static_assert(std::same_as<cgpui::IntoElement, cgpui::AnyElement>);
static_assert(cgpui::Render<RenderSpellingView>);

int main() {
  cgpui::IntoElement element =
      cgpui::into_element(cgpui::div().size(1.0F, 2.0F));
  return element != nullptr ? 0 : 1;
}
