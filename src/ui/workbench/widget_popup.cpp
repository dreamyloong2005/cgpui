#include "widget_popup_internal.hpp"

#include "cgpui/ui/paint.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/runtime_popups.hpp"
#include "cgpui/ui/view.hpp"

#include "../runtime_popups_internal.hpp"

#include <utility>
#include <variant>

namespace cgpui::workbench_detail {
namespace {

template <typename Shared>
class WidgetPopupView final : public View {
 public:
  explicit WidgetPopupView(std::weak_ptr<Shared> shared) : shared_(std::move(shared)) {}
  [[nodiscard]] std::weak_ptr<int> alive() const { return alive_; }

  void paint(PaintList& paint_list, Size) override {
    const auto shared = shared_.lock();
    if (!shared || !shared->content.paint) return;
    paint_list.push_metadata(PaintMetadata{.transform = AffineTransform::translation(
                                               -shared->origin.x, -shared->origin.y)});
    shared->content.paint(paint_list);
    paint_list.pop_metadata();
  }

  EventResult handle_event(const PlatformEvent& event,
                           const WindowRuntimeContext& context) override {
    const auto shared = shared_.lock();
    if (!shared || !shared->content.event) return EventResult::unhandled();
    PlatformEvent translated = event;
    std::visit(
        [&](auto& value) {
          if constexpr (requires { value.position; }) {
            value.position.x += shared->origin.x;
            value.position.y += shared->origin.y;
          }
        },
        translated);
    return shared->content.event(translated, context);
  }

 private:
  std::weak_ptr<Shared> shared_;
  std::shared_ptr<int> alive_ = std::make_shared<int>(0);
};

bool same_size(Size a, Size b) { return a.width == b.width && a.height == b.height; }

bool same_placement(const WindowPopupPlacement& a, const WindowPopupPlacement& b) {
  return a.anchor.origin.x == b.anchor.origin.x && a.anchor.origin.y == b.anchor.origin.y &&
         same_size(a.anchor.size, b.anchor.size) && a.side == b.side && a.gap == b.gap;
}

}  // namespace

std::function<EventResult(const PlatformEvent&, const WindowRuntimeContext&)>
forward_pointer_input_to_root() {
  return [](const PlatformEvent& event, const WindowRuntimeContext& context) {
    if (!std::holds_alternative<PointerMoved>(event) &&
        !std::holds_alternative<PointerButton>(event) &&
        !std::holds_alternative<PointerScrolled>(event)) {
      return EventResult::unhandled();
    }
    RuntimePopups::forward_to_root(context.runtime, event);
    return EventResult::consumed_event();
  };
}

WidgetPopup::~WidgetPopup() { close(); }

std::optional<Rect> WidgetPopup::show(
    WindowRuntime& runtime, WindowPopupPlacement placement, Size size,
    WindowPopupOptions options, WidgetPopupContent content) {
  if (open() && !same_size(size, size_)) close();
  if (!open()) {
    id_ = {};
    if (!popup_windows_supported(runtime)) return std::nullopt;
    auto shared = std::make_shared<Shared>();
    auto view = std::make_unique<WidgetPopupView<Shared>>(shared);
    std::weak_ptr<int> alive = view->alive();
    auto opened = open_popup(runtime, placement, size, std::move(view), options);
    if (!opened) return std::nullopt;
    view_alive_ = std::move(alive);
    placement_.reset();
    runtime_ = &runtime;
    id_ = opened->runtime_id;
    size_ = size;
    shared_ = std::move(shared);
  }
  shared_->content = std::move(content);
  // Layout runs every frame; re-place only when the request changed.
  if (placement_ && same_placement(*placement_, placement)) {
    redraw();
    return bounds_;
  }
  const PlatformPopupPlacementResult placed = place_popup(*runtime_, id_, placement);
  if (!placed.placed) {
    close();
    return std::nullopt;
  }
  bounds_ = placed.bounds;
  placement_ = placement;
  shared_->origin = bounds_.origin;
  redraw();
  return bounds_;
}

// Widgets call this from the root window's layout, inside its frame; the
// popup frame follows after it (one pending request at a time), never
// nested in the root frame.
void WidgetPopup::redraw() {
  if (!open() || !shared_ || shared_->redraw_pending) return;
  shared_->redraw_pending = true;
  runtime_->defer([popup = id_, shared = std::weak_ptr<Shared>(shared_)](
                      const WindowRuntimeContext& context) {
    const auto live = shared.lock();
    if (!live) return;
    live->redraw_pending = false;
    (void)request_popup_redraw(context.runtime, popup);
  });
}

void WidgetPopup::close() {
  if (!open()) {
    id_ = {};
    shared_.reset();
    return;
  }
  // The popup's view may be on the stack (an item activated in it), so the
  // window closes after this event; its content stops answering now.
  shared_.reset();
  runtime_->defer([popup = id_](const WindowRuntimeContext& context) {
    (void)close_popup(context.runtime, popup);
  });
  runtime_ = nullptr;
  id_ = {};
  size_ = {};
}

}  // namespace cgpui::workbench_detail
