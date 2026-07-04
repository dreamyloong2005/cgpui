#include "cgpui/prelude.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace {

struct PublicAuthoringModel {
  explicit PublicAuthoringModel(int initial_value) : value(initial_value) {}

  int value = 0;
};

class PublicAuthoringSurfaceView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::IntoElement render(
      cgpui::Context<PublicAuthoringSurfaceView>& context) override {
    const cgpui::App app = context.app();
    const cgpui::Window window = context.window();
    const cgpui::ViewHandle<PublicAuthoringSurfaceView> view =
        context.view<PublicAuthoringSurfaceView>();
    const cgpui::WeakViewHandle<PublicAuthoringSurfaceView> weak_view =
        view.downgrade();
    const cgpui::Model<PublicAuthoringModel> model =
        context.new_model<PublicAuthoringModel>(7);
    const cgpui::EntityHandle<PublicAuthoringModel> entity =
        context.entity(model);
    const cgpui::WeakEntity<PublicAuthoringModel> weak_entity =
        context.weak_entity(model);

    (void)app;
    (void)window;
    (void)weak_view;
    (void)entity;
    (void)weak_entity;

    return cgpui::into_element(
        cgpui::div()
            .size(context.viewport_size)
            .padding(cgpui::edges(8.0F))
            .background(cgpui::rgb(24, 32, 48))
            .child(cgpui::label("Public authoring surface").build()));
  }
};

using ContextRef = cgpui::Context<PublicAuthoringSurfaceView>&;

static_assert(cgpui::Render<PublicAuthoringSurfaceView>);
static_assert(std::same_as<cgpui::IntoElement, cgpui::AnyElement>);
static_assert(std::same_as<decltype(std::declval<ContextRef>().app()),
                           cgpui::App>);
static_assert(std::same_as<decltype(std::declval<ContextRef>().window()),
                           cgpui::Window>);
static_assert(std::same_as<decltype(std::declval<ContextRef>().view<
                                    PublicAuthoringSurfaceView>()),
                           cgpui::ViewHandle<PublicAuthoringSurfaceView>>);
static_assert(std::same_as<
              decltype(std::declval<ContextRef>().weak_view<
                       PublicAuthoringSurfaceView>()),
              cgpui::WeakViewHandle<PublicAuthoringSurfaceView>>);
static_assert(std::is_move_constructible_v<cgpui::Application>);
static_assert(!std::is_copy_constructible_v<cgpui::Application>);

} // namespace

int main() {
  cgpui::WindowOptions options;
  options.title("Public Authoring Surface").size(320.0F, 180.0F);
  const cgpui::WindowDescriptor descriptor = options.to_descriptor();

  cgpui::AnyElement element = cgpui::into_element(
      cgpui::div()
          .size(12.0F, 8.0F)
          .child(cgpui::label("prelude").font_size(13.0F).build()));
  const auto* root = dynamic_cast<const cgpui::StyledElement*>(element.get());

  return descriptor.title == "Public Authoring Surface" &&
                 descriptor.size.width == 320.0F && root != nullptr
             ? 0
             : 1;
}
