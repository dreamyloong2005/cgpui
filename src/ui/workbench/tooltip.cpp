#include "cgpui/ui/tooltip.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "widget_popup_internal.hpp"
#include "workbench_internal.hpp"

#include <utility>
#include <variant>

namespace cgpui {

Tooltip::Tooltip(TooltipOptions options) : options_(std::move(options)) {
  live_->fade_ms = options_.fade_ms;
}

// Reuses the entry for `id`, so re-registering every frame with unchanged
// text does not allocate.
void Tooltip::set_target(TooltipTargetId id, Rect bounds, std::string_view text) {
  for (Target& target : targets_) {
    if (target.id != id) continue;
    target.bounds = bounds;
    if (target.text != text) target.text.assign(text);
    return;
  }
  targets_.push_back(Target{.id = id, .bounds = bounds, .text = std::string(text)});
}

void Tooltip::remove_target(TooltipTargetId id) {
  std::erase_if(targets_, [id](const Target& target) { return target.id == id; });
  if (active_ == id) {
    hide();
    active_.reset();
  }
}

void Tooltip::clear_targets() {
  targets_.clear();
  hide();
  active_.reset();
}

void Tooltip::hide() {
  if (popup_) popup_->close();
  in_popup_ = false;
  live_->generation += 1;
  live_->pending = false;
  live_->visible = false;
  live_->opacity = 0.0F;
  if (live_->fade.active()) (void)live_->fade.cancel();
}

void Tooltip::hide_and_suppress() {
  hide();
  suppressed_ = active_;
}

const Tooltip::Target* Tooltip::find(TooltipTargetId id) const {
  for (const Target& target : targets_) {
    if (target.id == id) return &target;
  }
  return nullptr;
}

// Later registrations win, matching paint order (last painted is on top).
std::optional<TooltipTargetId> Tooltip::target_at(Point point) const {
  for (auto it = targets_.rbegin(); it != targets_.rend(); ++it) {
    if (contains(it->bounds, point)) return it->id;
  }
  return std::nullopt;
}

void Tooltip::begin_hover(TooltipTargetId id, const WindowRuntimeContext& context) {
  runtime_ = &context.runtime;
  hide();
  active_ = id;
  anchor_pointer_ = pointer_;
  live_->pending = true;
  const std::uint64_t generation = live_->generation;
  (void)context.schedule_timer(
      options_.delay_ms,
      [weak = std::weak_ptr<Live>(live_), generation](
          const WindowRuntimeContext& timer_context) {
        const std::shared_ptr<Live> live = weak.lock();
        if (live == nullptr || live->generation != generation) return;
        live->pending = false;
        live->visible = true;
        if (live->reduced_motion || live->fade_ms == 0) {
          live->opacity = 1.0F;
        } else {
          live->opacity = 0.0F;
          live->fade = timer_context.start_animation(
              AnimationOptions{.duration_ms = live->fade_ms},
              [weak, generation](const WindowRuntimeContext& frame,
                                 const AnimationSnapshot& snapshot) {
                const std::shared_ptr<Live> faded = weak.lock();
                if (faded == nullptr || faded->generation != generation) return;
                faded->opacity = snapshot.complete ? 1.0F : snapshot.eased_progress;
                frame.request_paint();
              });
        }
        timer_context.request_render();
      });
}

EventResult Tooltip::handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  const bool showing = live_->visible || live_->pending;
  if (const auto* moved = std::get_if<PointerMoved>(&event)) {
    pointer_ = moved->position;
    const std::optional<TooltipTargetId> hit = target_at(pointer_);
    if (hit != active_) {
      if (showing) context.request_render();
      suppressed_.reset();
      if (hit.has_value()) {
        begin_hover(*hit, context);
      } else {
        hide();
        active_.reset();
      }
    }
    return EventResult::unhandled();
  }
  if (std::holds_alternative<PointerExited>(event) ||
      (std::holds_alternative<WindowFocused>(event) &&
       !std::get<WindowFocused>(event).focused)) {
    if (showing) context.request_render();
    hide();
    active_.reset();
    suppressed_.reset();
    return EventResult::unhandled();
  }
  // A press or scroll hides the tooltip until the pointer leaves the target.
  if ((std::holds_alternative<PointerButton>(event) &&
       std::get<PointerButton>(event).pressed) ||
      std::holds_alternative<PointerScrolled>(event)) {
    if (showing) context.request_render();
    if (active_.has_value()) hide_and_suppress();
    return EventResult::unhandled();
  }
  if (const auto* key = std::get_if<KeyboardKey>(&event)) {
    if (key->action == KeyAction::pressed &&
        key->key_code == workbench_detail::key_escape && live_->visible) {
      hide_and_suppress();
      context.request_render();
      return EventResult::consumed_event();
    }
  }
  return EventResult::unhandled();
}

bool Tooltip::visible() const {
  return live_->visible;
}

bool Tooltip::pending() const {
  return live_->pending;
}

float Tooltip::opacity() const {
  return live_->opacity;
}

std::optional<TooltipTargetId> Tooltip::target() const {
  return live_->visible || live_->pending ? active_ : std::nullopt;
}

} // namespace cgpui
