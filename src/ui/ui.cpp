#include "cgpui/ui/ui.hpp"

#include <algorithm>
#include <expected>
#include <memory>
#include <utility>
#include <variant>

namespace cgpui {
namespace {

EventKind event_kind_for(const PlatformEvent& event) {
  if (std::holds_alternative<WindowActivated>(event)) {
    return EventKind::window_activated;
  }
  if (std::holds_alternative<WindowFocused>(event)) {
    return EventKind::window_focused;
  }
  if (std::holds_alternative<WindowMinimized>(event)) {
    return EventKind::window_minimized;
  }
  if (std::holds_alternative<WindowRestored>(event)) {
    return EventKind::window_restored;
  }
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    return EventKind::window_close_requested;
  }
  if (std::holds_alternative<PointerMoved>(event)) {
    return EventKind::pointer_moved;
  }
  if (std::holds_alternative<PointerButton>(event)) {
    return EventKind::pointer_button;
  }
  if (std::holds_alternative<PointerScrolled>(event)) {
    return EventKind::pointer_scrolled;
  }
  if (std::holds_alternative<DragEntered>(event)) {
    return EventKind::drag_entered;
  }
  if (std::holds_alternative<DragUpdated>(event)) {
    return EventKind::drag_updated;
  }
  if (std::holds_alternative<DragDropped>(event)) {
    return EventKind::drag_dropped;
  }
  if (std::holds_alternative<DragExited>(event)) {
    return EventKind::drag_exited;
  }
  if (std::holds_alternative<KeyboardKey>(event)) {
    return EventKind::keyboard_key;
  }
  if (std::holds_alternative<TextInput>(event)) {
    return EventKind::text_input;
  }
  if (std::holds_alternative<ImeComposition>(event)) {
    return EventKind::ime_composition;
  }
  return EventKind::unknown;
}

std::optional<Point> pointer_position_for(const PlatformEvent& event) {
  if (const auto* moved = std::get_if<PointerMoved>(&event);
      moved != nullptr) {
    return moved->position;
  }
  if (const auto* button = std::get_if<PointerButton>(&event);
      button != nullptr) {
    return button->position;
  }
  if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
      scrolled != nullptr) {
    return scrolled->position;
  }
  if (const auto* drag_entered = std::get_if<DragEntered>(&event);
      drag_entered != nullptr) {
    return drag_entered->position;
  }
  if (const auto* drag_updated = std::get_if<DragUpdated>(&event);
      drag_updated != nullptr) {
    return drag_updated->position;
  }
  if (const auto* drag_dropped = std::get_if<DragDropped>(&event);
      drag_dropped != nullptr) {
    return drag_dropped->position;
  }
  if (const auto* drag_exited = std::get_if<DragExited>(&event);
      drag_exited != nullptr) {
    return drag_exited->position;
  }
  return {};
}

bool is_keyboard_routed_event(const PlatformEvent& event) {
  return std::holds_alternative<KeyboardKey>(event) ||
         std::holds_alternative<TextInput>(event) ||
         std::holds_alternative<ImeComposition>(event);
}

bool is_focus_activation_event(const PlatformEvent& event) {
  const auto* button = std::get_if<PointerButton>(&event);
  return button != nullptr && button->button == MouseButton::left &&
         button->pressed;
}

bool is_focus_traversal_key(const KeyboardKey& key) {
  constexpr std::uint32_t tab_key_code = 9;
  return key.key_code == tab_key_code && key.action == KeyAction::pressed &&
         !key.modifiers.control && !key.modifiers.alt && !key.modifiers.super;
}

bool modifiers_equal(KeyboardModifiers lhs, KeyboardModifiers rhs) {
  return lhs.shift == rhs.shift && lhs.control == rhs.control &&
         lhs.alt == rhs.alt && lhs.super == rhs.super;
}

bool rect_equal(Rect lhs, Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width && lhs.size.height == rhs.size.height;
}

bool ime_text_input_placement_equal(
    const std::optional<ImeTextInputPlacement>& lhs,
    const std::optional<ImeTextInputPlacement>& rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }
  if (!lhs.has_value()) {
    return true;
  }
  return lhs->byte_offset == rhs->byte_offset &&
         rect_equal(lhs->rect, rhs->rect);
}

bool is_valid_pointer_capture_owner(const PointerCaptureOwner& owner) {
  if (const ViewId* view_id = owner.view_id(); view_id != nullptr) {
    return view_id->value != 0;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    return element_id->value != 0;
  }
  return false;
}

void apply_pointer_capture_owner_to_route(
    const PointerCaptureOwner& owner,
    EventRoute& route) {
  if (const ViewId* view_id = owner.view_id(); view_id != nullptr) {
    route.target_view_id = *view_id;
    route.target_element_id.reset();
    return;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    route.target_element_id = *element_id;
  }
}

PaintMetadata compose_paint_metadata(
    PaintMetadata parent,
    PaintMetadata child) {
  return PaintMetadata{
      .opacity = parent.opacity * child.opacity,
      .transform = compose(parent.transform, child.transform),
  };
}

PaintMetadata paint_metadata_for_style(const Style& style) {
  return PaintMetadata{
      .opacity = style.opacity,
      .transform = style.transform,
  };
}

ElementId hit_test_runtime_element_root(
    const ElementTree* tree,
    const Element* root,
    Point point) {
  if (tree != nullptr) {
    return tree->hit_test_root(point);
  }
  return root == nullptr ? ElementId{} : root->hit_test(point);
}

void paint_styled_box_base(
    PaintList& paint_list,
    const std::optional<Rect>& bounds,
    const Style& style) {
  if (bounds.has_value() && style.background_color.has_value()) {
    const BorderRadii radius = style.border_radius;
    if (radius.top_left > 0.0F || radius.top_right > 0.0F ||
        radius.bottom_right > 0.0F || radius.bottom_left > 0.0F) {
      paint_list.fill_rounded_rect(*bounds, *style.background_color, radius);
    } else {
      paint_list.fill_rect(*bounds, *style.background_color);
    }
  }
  if (bounds.has_value() && style.border_color.has_value()) {
    const Rect rect = *bounds;
    const Color color = *style.border_color;
    const float top = style.border_width.top;
    const float right = style.border_width.right;
    const float bottom = style.border_width.bottom;
    const float left = style.border_width.left;
    const float vertical_side_height =
        std::max(0.0F, rect.size.height - top - bottom);

    if (top > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin = rect.origin,
              .size = {.width = rect.size.width, .height = top},
          },
          color);
    }
    if (right > 0.0F && vertical_side_height > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin =
                  {
                      .x = rect.origin.x + rect.size.width - right,
                      .y = rect.origin.y + top,
                  },
              .size = {.width = right, .height = vertical_side_height},
          },
          color);
    }
    if (bottom > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin =
                  {
                      .x = rect.origin.x,
                      .y = rect.origin.y + rect.size.height - bottom,
                  },
              .size = {.width = rect.size.width, .height = bottom},
          },
          color);
    }
    if (left > 0.0F && vertical_side_height > 0.0F) {
      paint_list.fill_rect(
          Rect{
              .origin = {.x = rect.origin.x, .y = rect.origin.y + top},
              .size = {.width = left, .height = vertical_side_height},
          },
          color);
    }
  }
}

} // namespace

Subscription::~Subscription() {
  (void)release();
}

Subscription::Subscription(Subscription&& other) noexcept
    : runtime_(std::exchange(other.runtime_, nullptr)),
      id_(std::exchange(other.id_, {})) {}

Subscription& Subscription::operator=(Subscription&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  (void)release();
  runtime_ = std::exchange(other.runtime_, nullptr);
  id_ = std::exchange(other.id_, {});
  return *this;
}

bool Subscription::connected() const {
  return runtime_ != nullptr && runtime_->subscription_connected(id_);
}

bool Subscription::release() {
  if (runtime_ == nullptr) {
    return false;
  }

  WindowRuntime* runtime = runtime_;
  const SubscriptionId id = id_;
  runtime_ = nullptr;
  id_ = {};
  return runtime->remove_subscription(id);
}

bool TaskHandle::active() const {
  return runtime_ != nullptr && runtime_->task_active(id_);
}

bool TaskHandle::complete() const {
  return runtime_ != nullptr && runtime_->task_complete(id_);
}

void PaintList::clear() {
  commands_.clear();
  clip_stack_.clear();
  metadata_stack_.clear();
  scale_ = {};
}

void PaintList::set_scale(DpiScale scale) {
  scale_ = scale;
}

DpiScale PaintList::scale() const {
  return scale_;
}

void PaintList::push_clip(Rect rect) {
  clip_stack_.push_back(rect);
}

void PaintList::pop_clip() {
  if (!clip_stack_.empty()) {
    clip_stack_.pop_back();
  }
}

void PaintList::push_metadata(PaintMetadata metadata) {
  if (!metadata_stack_.empty()) {
    metadata = compose_paint_metadata(metadata_stack_.back(), metadata);
  }
  metadata_stack_.push_back(metadata);
}

void PaintList::pop_metadata() {
  if (!metadata_stack_.empty()) {
    metadata_stack_.pop_back();
  }
}

void PaintList::fill_rect(Rect rect, Color color) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::solid_rect,
      .solid_rect = SolidRect{.rect = rect, .color = color},
      .rounded_rect = RoundedRect{},
      .clip_rect = clip_stack_.empty()
                       ? std::optional<Rect>{}
                       : std::optional<Rect>{clip_stack_.back()},
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

void PaintList::fill_rounded_rect(Rect rect, Color color, BorderRadii radius) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::rounded_rect,
      .solid_rect = SolidRect{.rect = rect, .color = color},
      .rounded_rect =
          RoundedRect{
              .rect = rect,
              .color = color,
              .radius = radius,
          },
      .clip_rect = clip_stack_.empty()
                       ? std::optional<Rect>{}
                       : std::optional<Rect>{clip_stack_.back()},
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

void PaintList::fill_text(
    Rect bounds,
    Color color,
    std::string_view text,
    FontDescriptor font,
    float font_size) {
  const TextShapeRun shape_run = shape_text(text, font, font_size, scale_);
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::text,
      .text =
          TextPaint{
              .bounds = bounds,
              .color = color,
              .font = shape_run.font,
              .content = std::string(text),
              .byte_length = text.size(),
              .font_size = font_size,
              .scale = scale_,
              .device_font_size = shape_run.device_font_size,
              .glyphs = text_glyph_paint_metadata(shape_run, bounds.origin),
          },
      .clip_rect = clip_stack_.empty()
                       ? std::optional<Rect>{}
                       : std::optional<Rect>{clip_stack_.back()},
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

void PaintList::fill_text_selection(
    Rect rect,
    Color color,
    TextSelectionRange range,
    float font_size) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::text_selection,
      .text_selection =
          TextSelectionPaint{
              .rect = rect,
              .color = color,
              .range = range,
              .font_size = font_size,
          },
      .clip_rect = clip_stack_.empty()
                       ? std::optional<Rect>{}
                       : std::optional<Rect>{clip_stack_.back()},
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

void PaintList::fill_text_caret(
    Rect rect,
    Color color,
    std::size_t byte_offset,
    float font_size) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::text_caret,
      .text_caret =
          TextCaretPaint{
              .rect = rect,
              .color = color,
              .byte_offset = byte_offset,
              .font_size = font_size,
          },
      .clip_rect = clip_stack_.empty()
                       ? std::optional<Rect>{}
                       : std::optional<Rect>{clip_stack_.back()},
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

std::span<const PaintCommand> PaintList::commands() const {
  return commands_;
}

WindowOptions& WindowOptions::title(std::string title_value) {
  descriptor.title = std::move(title_value);
  return *this;
}

WindowOptions& WindowOptions::size(Size size_value) {
  descriptor.size = size_value;
  return *this;
}

WindowOptions& WindowOptions::size(float width, float height) {
  descriptor.size = Size{.width = width, .height = height};
  return *this;
}

WindowDescriptor WindowOptions::to_descriptor() const {
  return descriptor;
}

AppOpenedWindow AppContext::open_window(WindowOptions options) const {
  return runtime.open_window(std::move(options));
}

AppOpenedWindow AppContext::open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) const {
  return runtime.open_window(std::move(options), std::move(root_view));
}

void StyledElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  const Style& base_style = style();
  paint_list.push_metadata(paint_metadata_for_style(base_style));
  const bool uses_hidden_overflow_clip =
      bounds.has_value() && base_style.overflow == Overflow::hidden;
  if (uses_hidden_overflow_clip) {
    paint_list.push_clip(base_style.clip_rect.has_value()
                             ? *base_style.clip_rect
                             : *bounds);
  }
  paint_styled_box_base(paint_list, bounds, base_style);
  if (child_ != nullptr) {
    child_->paint(paint_list);
  }
  if (uses_hidden_overflow_clip) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

void ButtonElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  const Style& base_style = style_state_.base;
  paint_list.push_metadata(paint_metadata_for_style(base_style));
  const bool uses_hidden_overflow_clip =
      bounds.has_value() && base_style.overflow == Overflow::hidden;
  if (uses_hidden_overflow_clip) {
    paint_list.push_clip(base_style.clip_rect.has_value()
                             ? *base_style.clip_rect
                             : *bounds);
  }
  paint_styled_box_base(paint_list, bounds, base_style);
  if (child_) {
    child_->paint(paint_list);
  }
  if (uses_hidden_overflow_clip) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

void ScrollableListElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  paint_list.push_metadata(paint_metadata_for_style(style_));
  if (bounds.has_value()) {
    paint_list.push_clip(*bounds);
  }
  for (const auto& child : content_.children()) {
    if (child) {
      child->paint(paint_list);
    }
  }
  if (bounds.has_value()) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

void LabelElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || text().empty()) {
    return;
  }
  const Color text_color = style().foreground_color.value_or(
      Color{.r = 0.82F, .g = 0.86F, .b = 0.92F, .a = 1.0F});
  paint_list.push_metadata(paint_metadata_for_style(style()));
  paint_list.fill_text(*bounds, text_color, text(), style().font, font_size());
  paint_list.pop_metadata();
}

void TextElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || model_ == nullptr) {
    return;
  }
  const Style& text_style = style();
  const Color text_color = text_style.foreground_color.value_or(
      Color{.r = 0.82F, .g = 0.86F, .b = 0.92F, .a = 1.0F});
  paint_list.push_metadata(paint_metadata_for_style(text_style));
  const float font_size_value = text_style.font_size;
  const float glyph_width_value = glyph_width();
  const TextSelectionRange selection = model_->selection();
  if (!selection.collapsed) {
    paint_list.fill_text_selection(
        Rect{
            .origin =
                {
                    .x = bounds->origin.x +
                         (static_cast<float>(selection.start) *
                          glyph_width_value),
                    .y = bounds->origin.y,
                },
            .size =
                {
                    .width = static_cast<float>(selection.end - selection.start) *
                             glyph_width_value,
                    .height = font_size_value,
                },
        },
        Color{.r = 0.22F, .g = 0.42F, .b = 0.80F, .a = 0.38F},
        selection,
        font_size_value);
  }
  if (!text().empty()) {
    paint_list.fill_text(
        *bounds,
        text_color,
        text(),
        text_style.font,
        font_size_value);
  }
  paint_list.fill_text_caret(
      Rect{
          .origin =
              {
                  .x = bounds->origin.x +
                       (static_cast<float>(model_->cursor()) *
                        glyph_width_value),
                  .y = bounds->origin.y,
              },
          .size = {.width = 1.0F, .height = font_size_value},
      },
      text_color,
      model_->cursor(),
      font_size_value);
  paint_list.pop_metadata();
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    FrameStatistics* statistics) {
  return render_view(renderer, view, viewport_size, DpiScale{}, statistics);
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    FrameStatistics* statistics) {
  if (statistics != nullptr) {
    statistics->begin_frame_count += 1;
  }
  auto frame = renderer.begin_frame();
  if (!frame) {
    return std::unexpected(frame.error());
  }
  if (!*frame) {
    return std::unexpected(Error{
        .code = ErrorCode::frame_acquisition_failed,
        .message = "Renderer returned an empty frame",
    });
  }

  (*frame)->clear(Color{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});
  if (statistics != nullptr) {
    statistics->clear_count += 1;
  }

  PaintList paint_list;
  paint_list.set_scale(scale);
  view.paint(paint_list, viewport_size);
  if (statistics != nullptr) {
    statistics->paint_pass_count += 1;
    statistics->paint_command_count = paint_list.commands().size();
  }
  for (const auto& command : paint_list.commands()) {
    if (command.kind == PaintCommandKind::text_selection ||
        command.kind == PaintCommandKind::text_caret) {
      if (statistics != nullptr) {
        statistics->skipped_command_count += 1;
      }
      continue;
    }
    if (command.kind == PaintCommandKind::text) {
      const TextPaint& text = command.text;
      (*frame)->draw_text(TextDraw{
          .bounds = text.bounds,
          .color = text.color,
          .font = text.font,
          .content = text.content,
          .byte_length = text.byte_length,
          .font_size = text.font_size,
          .scale = text.scale,
          .device_font_size = text.device_font_size,
          .glyphs = text.glyphs,
          .clip_rect = command.clip_rect,
          .metadata = command.metadata,
      });
      if (statistics != nullptr) {
        statistics->submitted_command_count += 1;
        statistics->text_command_count += 1;
      }
      continue;
    }
    SolidRect rect = command.solid_rect;
    rect.clip_rect = command.clip_rect;
    rect.metadata = command.metadata;
    (*frame)->draw_rect(rect);
    if (statistics != nullptr) {
      statistics->submitted_command_count += 1;
      statistics->solid_rect_command_count += 1;
    }
  }

  auto result = (*frame)->present();
  if (result && statistics != nullptr) {
    statistics->present_count += 1;
  }
  return result;
}

int run_app(
    PlatformApplication& application,
    View& view,
    AppRendererFactory renderer_factory,
    AppRunnerOptions options) {
  std::unique_ptr<Renderer> renderer;
  WindowRuntime runtime(
      application,
      view,
      [&](const RenderSurfaceDescriptor& descriptor) -> Result<Renderer*> {
        if (!renderer_factory) {
          return std::unexpected(Error{
              .code = ErrorCode::renderer_initialization_failed,
              .message = "App runner requires a renderer factory"});
        }
        auto result = renderer_factory(descriptor);
        if (!result) {
          return std::unexpected(result.error());
        }
        if (*result == nullptr) {
          return std::unexpected(Error{
              .code = ErrorCode::renderer_initialization_failed,
              .message = "App renderer factory returned an empty renderer"});
        }
        renderer = std::move(*result);
        return renderer.get();
      });

  if (options.setup) {
    options.setup(runtime);
  }
  if (options.setup_context) {
    AppContext context{.runtime = runtime};
    options.setup_context(context);
  }
  return runtime.run(options.window, options.runtime);
}

AnyElement View::render(ViewContext& context) {
  (void)context;
  return {};
}

EventResult View::handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  (void)event;
  (void)context;
  return EventResult::unhandled();
}

EventRoute EventRouter::route_to_root(
    const PlatformEvent& event,
    ViewId root_view_id) {
  return EventRoute{
      .target_view_id = root_view_id,
      .view_ancestry = {root_view_id},
      .event_kind = event_kind_for(event)};
}

WindowRuntime::WindowRuntime(
    PlatformApplication& application,
    View& view,
    RendererFactory renderer_factory)
    : application_(application),
      view_(view),
      renderer_factory_(std::move(renderer_factory)) {
  view_registry_.insert_or_assign(
      root_view_id_.value,
      RegisteredView{.view = &view_});
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = root_window_runtime_id_,
      .descriptor = {},
      .root_view_id = root_view_id_,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = false,
      .owns_root_view = false,
      .active = false});
}

int WindowRuntime::run(
    const WindowDescriptor& descriptor,
    WindowRuntimeOptions options) {
  should_quit_ = false;
  failed_ = false;
  window_ = nullptr;
  renderer_ = nullptr;
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->descriptor = descriptor;
    root_record->root_view_id = root_view_id_;
    root_record->window = nullptr;
    root_record->renderer = nullptr;
    root_record->active = false;
  }
  framebuffer_size_ = descriptor.size;
  viewport_size_ = descriptor.size;
  scale_ = {};
  input_ = {};
  pointer_capture_owner_.reset();
  keyboard_focus_owner_.reset();
  keyboard_focus_element_owner_.reset();
  hovered_element_id_.reset();
  cursor_shape_ = CursorShape::default_arrow;
  applied_cursor_shape_ = CursorShape::default_arrow;
  last_event_result_ = EventResult::unhandled();
  last_render_record_.reset();
  last_frame_statistics_.reset();
  last_event_dispatch_.reset();
  last_action_dispatch_.reset();
  current_event_route_.reset();
  invalidation_state_ = {};
  subscription_query_buffer_.clear();
  entity_count_ = 0;
  dispatching_view_event_ = false;
  firing_timers_ = false;
  draining_task_completions_ = false;
  update_batch_depth_ = 0;
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  event_dispatch_sequence_ = 0;
  render_sequence_ = 0;
  frame_index_ = 0;
  applied_ime_text_input_placement_.reset();

  auto window_result = application_.create_window(
      descriptor,
      [this](const PlatformEvent& event) { handle_event(event); });
  if (!window_result) {
    if (error_callback_) {
      error_callback_(window_result.error());
    }
    return 1;
  }

  std::unique_ptr<PlatformWindow> window = std::move(*window_result);
  window_ = window.get();
  const WindowState window_state = window_->state();
  framebuffer_size_ = window_state.framebuffer_size;
  scale_ = window_state.scale;
  viewport_size_ = to_logical_pixels(framebuffer_size_, scale_);

  auto renderer_result = renderer_factory_(RenderSurfaceDescriptor{
      .native_surface = window_->native_surface(),
      .framebuffer_size = window_state.framebuffer_size,
      .scale = window_state.scale});
  if (!renderer_result || *renderer_result == nullptr) {
    if (error_callback_) {
      if (renderer_result) {
        error_callback_(Error{
            .code = ErrorCode::renderer_initialization_failed,
            .message = "Renderer factory returned an empty renderer"});
      } else {
        error_callback_(renderer_result.error());
      }
    }
    return 1;
  }
  renderer_ = *renderer_result;
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->window = window_;
    root_record->renderer = renderer_;
    root_record->active = true;
  }

  if (options.request_initial_redraw) {
    redraw_scheduled_ = true;
    window_->request_redraw();
  }

  const int run_result = application_.run();
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->window = nullptr;
    root_record->renderer = nullptr;
    root_record->active = false;
  }
  window_ = nullptr;
  renderer_ = nullptr;

  if (failed_) {
    return 1;
  }
  return run_result;
}

AppOpenedWindow WindowRuntime::open_window(WindowOptions options) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = {}};
  app_opened_windows_.push_back(opened);
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = {},
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = true,
      .owns_root_view = false,
      .active = false});
  return opened;
}

AppOpenedWindow WindowRuntime::open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  const ViewId root_view_id = register_view(std::move(root_view));
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = root_view_id};
  app_opened_windows_.push_back(opened);
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = root_view_id,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = true,
      .owns_root_view = root_view_id.value != 0,
      .active = false});
  return opened;
}

std::span<const AppOpenedWindow> WindowRuntime::app_opened_windows() const {
  return app_opened_windows_;
}

WindowRuntimeId WindowRuntime::root_window_runtime_id() const {
  return root_window_runtime_id_;
}

std::span<const WindowRuntimeRecord> WindowRuntime::window_runtime_records()
    const {
  return window_runtime_records_;
}

const WindowRuntimeRecord* WindowRuntime::window_runtime_record(
    WindowRuntimeId runtime_id) const {
  return find_window_runtime_record(runtime_id);
}

const View* WindowRuntime::app_opened_window_root_view(
    ViewId root_view_id) const {
  return find_view(root_view_id);
}

View* WindowRuntime::root_view() {
  return find_view(root_view_id_);
}

const View* WindowRuntime::root_view() const {
  return find_view(root_view_id_);
}

ViewId WindowRuntime::register_view(View& view) {
  const ViewId view_id = allocate_view_id();
  removed_view_ids_.erase(view_id.value);
  view_registry_.insert_or_assign(
      view_id.value,
      RegisteredView{.view = &view});
  return view_id;
}

ViewId WindowRuntime::register_view(std::unique_ptr<View> view) {
  const ViewId view_id = allocate_view_id();
  removed_view_ids_.erase(view_id.value);
  if (view != nullptr) {
    View* view_ptr = view.get();
    view_registry_.insert_or_assign(
        view_id.value,
        RegisteredView{.view = view_ptr, .owned_view = std::move(view)});
  }
  return view_id;
}

View* WindowRuntime::find_view(ViewId view_id) {
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return nullptr;
  }
  return entry->second.view;
}

const View* WindowRuntime::find_view(ViewId view_id) const {
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return nullptr;
  }
  return entry->second.view;
}

bool WindowRuntime::remove_view(ViewId view_id) {
  if (view_id.value == 0 || view_id == root_view_id_) {
    return false;
  }
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return false;
  }
  view_registry_.erase(entry);
  removed_view_ids_.insert(view_id.value);
  return true;
}

void WindowRuntime::handle_event(const PlatformEvent& event) {
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    should_quit_ = true;
    if (window_ != nullptr && renderer_ != nullptr) {
      record_lifecycle_event(event);
    }
    if (close_requested_callback_ && window_ != nullptr &&
        renderer_ != nullptr) {
      close_requested_callback_(context());
    }
    application_.quit();
    return;
  }

  if (std::holds_alternative<WindowActivated>(event) ||
      std::holds_alternative<WindowMinimized>(event) ||
      std::holds_alternative<WindowRestored>(event)) {
    if (window_ != nullptr && renderer_ != nullptr) {
      if (const auto* activated = std::get_if<WindowActivated>(&event);
          activated != nullptr) {
        input_.focused = activated->active;
      } else if (const auto* minimized = std::get_if<WindowMinimized>(&event);
                 minimized != nullptr) {
        input_.focused = !minimized->minimized;
      } else {
        input_.focused = false;
      }
      record_lifecycle_event(event);
    }
    return;
  }

  if (const auto* resized = std::get_if<WindowResized>(&event);
      resized != nullptr) {
    handle_resize(*resized);
    return;
  }

  if (std::holds_alternative<WindowRedrawRequested>(event)) {
    handle_redraw();
    return;
  }

  if (window_ != nullptr && renderer_ != nullptr) {
    if (const auto* focused = std::get_if<WindowFocused>(&event);
        focused != nullptr) {
      input_.focused = focused->focused;
    } else if (const auto* moved = std::get_if<PointerMoved>(&event);
               moved != nullptr) {
      input_.pointer_position = moved->position;
    } else if (const auto* button = std::get_if<PointerButton>(&event);
               button != nullptr) {
      input_.pointer_position = button->position;
    } else if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
               scrolled != nullptr) {
      input_.pointer_position = scrolled->position;
    } else if (const std::optional<Point> drag_position =
                   pointer_position_for(event);
               drag_position.has_value() &&
               (std::holds_alternative<DragEntered>(event) ||
                std::holds_alternative<DragUpdated>(event) ||
                std::holds_alternative<DragDropped>(event) ||
                std::holds_alternative<DragExited>(event))) {
      input_.pointer_position = *drag_position;
    }
    current_event_route_ = EventRouter::route_to_root(event, root_view_id_);
    std::optional<ElementId> hit_element_id;
    if (element_root() != nullptr) {
      if (const std::optional<Point> pointer_position =
              pointer_position_for(event);
          pointer_position.has_value()) {
        const ElementId hit = hit_test_runtime_element_root(
            owned_element_tree_.get(),
            element_root_,
            *pointer_position);
        if (hit.value != 0) {
          hit_element_id = hit;
        }
        if (std::holds_alternative<PointerMoved>(event)) {
          hovered_element_id_ = hit_element_id;
          cursor_shape_ = CursorShape::default_arrow;
          if (hit_element_id.has_value()) {
            const Element* hovered_element = routed_element(*hit_element_id);
            if (hovered_element == nullptr || hovered_element->enabled()) {
              if (const auto cursor =
                      element_cursors_.find(hit_element_id->value);
                  cursor != element_cursors_.end()) {
                cursor_shape_ = cursor->second;
              }
            }
          }
        }
      }
    } else if (std::holds_alternative<PointerMoved>(event)) {
      hovered_element_id_.reset();
      cursor_shape_ = CursorShape::default_arrow;
    }
    if (std::holds_alternative<PointerMoved>(event)) {
      apply_cursor_shape(cursor_shape_);
    }
    if (keyboard_focus_element_owner_.has_value() &&
        is_keyboard_routed_event(event)) {
      current_event_route_->target_element_id = keyboard_focus_element_owner_;
    } else if (pointer_capture_owner_.has_value() &&
        pointer_position_for(event).has_value()) {
      apply_pointer_capture_owner_to_route(
          *pointer_capture_owner_,
          *current_event_route_);
    } else if (hit_element_id.has_value()) {
      current_event_route_->target_element_id = hit_element_id;
    }
    refresh_route_ancestry(*current_event_route_);
    if (is_focus_activation_event(event) &&
        current_event_route_->target_element_id.has_value()) {
      if (Element* element =
              routed_element(*current_event_route_->target_element_id);
          element != nullptr && element->enabled() && element->focusable()) {
        request_keyboard_focus(*current_event_route_->target_element_id);
        element->focus(ElementFocusContext{
            .element_id = *current_event_route_->target_element_id,
        });
      }
    }
    if (const auto* key = std::get_if<KeyboardKey>(&event); key != nullptr) {
      if (is_focus_traversal_key(*key) &&
          focus_next_element(key->modifiers.shift)) {
        current_event_route_->target_element_id = keyboard_focus_element_owner_;
        refresh_route_ancestry(*current_event_route_);
      }
      for (const KeyBinding& binding : key_bindings_) {
        if (binding.key_code == key->key_code &&
            binding.action == key->action &&
            modifiers_equal(binding.modifiers, key->modifiers)) {
          (void)dispatch_action(binding.action_name);
          break;
        }
      }
      if (keyboard_focus_element_owner_.has_value()) {
        if (TextModel* model = focused_text_model(); model != nullptr) {
          for (const TextEditBinding& binding : text_edit_bindings_) {
            if (binding.key_code == key->key_code &&
                binding.action == key->action &&
                modifiers_equal(binding.modifiers, key->modifiers)) {
              (void)model->apply_edit_action(binding.edit_action);
              break;
            }
          }
        }
      }
    }
    if (const auto* text = std::get_if<TextInput>(&event);
        text != nullptr && keyboard_focus_element_owner_.has_value()) {
      if (TextModel* model = focused_text_model(); model != nullptr) {
        model->insert_text(text->text);
      }
    }
    if (const auto* composition = std::get_if<ImeComposition>(&event);
        composition != nullptr && keyboard_focus_element_owner_.has_value()) {
      if (TextModel* model = focused_text_model(); model != nullptr) {
        switch (composition->phase) {
          case ImeCompositionPhase::update:
            model->set_composition_text(composition->text);
            break;
          case ImeCompositionPhase::commit:
            model->set_composition_text(composition->text);
            model->commit_composition();
            break;
          case ImeCompositionPhase::cancel:
            model->cancel_composition();
            break;
        }
      }
    }
    EventResult result = EventResult::unhandled();
    if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
        scrolled != nullptr) {
      if (ScrollState* state = scroll_state_for_route(*current_event_route_);
          state != nullptr) {
        state->scroll_by(scrolled->delta);
        result = EventResult::consumed_event();
      }
    }
    if (!result.consumed && !result.cancelled) {
      result = dispatch_routed_element_event(event, *current_event_route_);
    }
    if (!result.consumed && !result.cancelled) {
      dispatching_view_event_ = true;
      result = view_.handle_event(event, context());
      dispatching_view_event_ = false;
    }
    last_event_result_ = result;
    last_event_dispatch_ = EventDispatchRecord{
        .sequence = ++event_dispatch_sequence_,
        .view_id = current_event_route_->target_view_id,
        .event_kind = current_event_route_->event_kind,
        .route = *current_event_route_,
        .result = last_event_result_};
    if (after_event_callback_) {
      after_event_callback_(context(), *last_event_dispatch_);
    }
    apply_focused_text_ime_placement();
    drain_deferred_callbacks();
    flush_deferred_redraw_request();
  }
}

void WindowRuntime::record_lifecycle_event(const PlatformEvent& event) {
  current_event_route_ = EventRouter::route_to_root(event, root_view_id_);
  last_event_result_ = EventResult::unhandled();
  last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++event_dispatch_sequence_,
      .view_id = current_event_route_->target_view_id,
      .event_kind = current_event_route_->event_kind,
      .route = *current_event_route_,
      .result = last_event_result_};
  if (after_event_callback_) {
    after_event_callback_(context(), *last_event_dispatch_);
  }
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

void WindowRuntime::handle_resize(const WindowResized& event) {
  (void)resize_surface(event.size, event.scale);
}

void WindowRuntime::handle_redraw() {
  if (renderer_ == nullptr || should_quit_) {
    return;
  }

  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context();
  AnyElement rendered = view_.render(render_context);
  std::optional<ElementId> rendered_root_id;
  if (rendered != nullptr) {
    auto tree = std::make_unique<ElementTree>();
    rendered_root_id = tree->set_root(std::move(rendered));
    set_element_tree(std::move(tree));
  }

  last_render_record_ = RenderRecord{
      .sequence = ++render_sequence_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
      .root_element_id = rendered_root_id,
  };
  if (after_render_callback_) {
    after_render_callback_(context(), *last_render_record_);
  }

  if (owned_element_tree_ != nullptr) {
    (void)owned_element_tree_->layout_root(LayoutInput{
        .constraints =
            {
                .max_size = viewport_size_,
            },
        .scale = scale_,
    });
    frame_statistics.layout_pass_count += 1;
  }
  apply_focused_text_ime_placement();

  auto result = render_view(
      *renderer_,
      view_,
      viewport_size_,
      scale_,
      &frame_statistics);
  if (!result) {
    fail_and_quit(result.error());
    return;
  }

  clear_invalidation();
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  frame_index_ += 1;
  frame_statistics.frame_index = frame_index_;
  if (last_render_record_.has_value()) {
    last_render_record_->statistics = frame_statistics;
  }
  last_frame_statistics_ = frame_statistics;
  if (after_frame_callback_) {
    after_frame_callback_(context());
  }
}

void WindowRuntime::fail_and_quit(Error error) {
  failed_ = true;
  should_quit_ = true;
  if (error_callback_) {
    error_callback_(error);
  }
  application_.quit();
}

WindowRuntimeId WindowRuntime::allocate_window_runtime_id() {
  const WindowRuntimeId runtime_id{next_window_runtime_id_};
  next_window_runtime_id_ += 1;
  return runtime_id;
}

WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(
    WindowRuntimeId runtime_id) {
  const auto record = std::find_if(
      window_runtime_records_.begin(),
      window_runtime_records_.end(),
      [runtime_id](const WindowRuntimeRecord& record) {
        return record.runtime_id == runtime_id;
      });
  return record == window_runtime_records_.end() ? nullptr : &*record;
}

const WindowRuntimeRecord* WindowRuntime::find_window_runtime_record(
    WindowRuntimeId runtime_id) const {
  const auto record = std::find_if(
      window_runtime_records_.begin(),
      window_runtime_records_.end(),
      [runtime_id](const WindowRuntimeRecord& record) {
        return record.runtime_id == runtime_id;
      });
  return record == window_runtime_records_.end() ? nullptr : &*record;
}

void WindowRuntime::refresh_route_ancestry(EventRoute& route) const {
  route.element_ancestry.clear();
  if (route.target_element_id.has_value()) {
    route.element_ancestry =
        element_ancestry_for(*route.target_element_id);
    if (const std::optional<ViewId> child_view_id =
            child_view_target_for(*route.target_element_id);
        child_view_id.has_value()) {
      route.target_view_id = *child_view_id;
    }
  }
  route.view_ancestry = view_ancestry_for(route.target_view_id);
}

std::vector<ElementId> WindowRuntime::element_ancestry_for(
    ElementId element_id) const {
  std::vector<ElementId> ancestry;
  if (element_id.value == 0) {
    return ancestry;
  }

  if (owned_element_tree_ != nullptr) {
    ElementId current = element_id;
    while (current.value != 0 && owned_element_tree_->get(current) != nullptr) {
      ancestry.push_back(current);
      const std::optional<ElementId> parent =
          owned_element_tree_->parent(current);
      if (!parent.has_value()) {
        break;
      }
      current = *parent;
    }
    return ancestry;
  }

  if (element_root_ != nullptr && element_root_->id() == element_id) {
    ancestry.push_back(element_id);
  }
  return ancestry;
}

std::vector<ViewId> WindowRuntime::view_ancestry_for(ViewId view_id) const {
  std::vector<ViewId> ancestry;
  if (view_id.value != 0) {
    ancestry.push_back(view_id);
  }
  if (view_id != root_view_id_ && root_view_id_.value != 0) {
    ancestry.push_back(root_view_id_);
  }
  return ancestry;
}

std::optional<ViewId> WindowRuntime::child_view_target_for(
    ElementId element_id) const {
  const auto* child_view =
      dynamic_cast<const ChildViewElement*>(routed_element(element_id));
  if (child_view == nullptr || child_view->view_id().value == 0 ||
      find_view(child_view->view_id()) == nullptr) {
    return {};
  }
  return child_view->view_id();
}

bool WindowRuntime::focus_next_element(bool reverse) {
  if (owned_element_tree_ == nullptr) {
    return false;
  }

  std::vector<ElementId> focusable_ids;
  for (ElementId element_id : owned_element_tree_->enabled_preorder_ids()) {
    const Element* element = routed_element(element_id);
    if (element != nullptr && element->focusable()) {
      focusable_ids.push_back(element_id);
    }
  }
  if (focusable_ids.empty()) {
    return false;
  }

  std::size_t next_index = reverse ? focusable_ids.size() - 1 : 0;
  if (keyboard_focus_element_owner_.has_value()) {
    const auto current = std::find(
        focusable_ids.begin(),
        focusable_ids.end(),
        *keyboard_focus_element_owner_);
    if (current != focusable_ids.end()) {
      const std::size_t current_index =
          static_cast<std::size_t>(current - focusable_ids.begin());
      if (reverse) {
        next_index = current_index == 0 ? focusable_ids.size() - 1
                                        : current_index - 1;
      } else {
        next_index = current_index + 1 == focusable_ids.size()
                         ? 0
                         : current_index + 1;
      }
    }
  }

  request_keyboard_focus(focusable_ids[next_index]);
  if (Element* element = routed_element(focusable_ids[next_index]);
      element != nullptr) {
    element->focus(ElementFocusContext{.element_id = focusable_ids[next_index]});
  }
  return true;
}

std::optional<ViewId> WindowRuntime::action_dispatch_view_id() const {
  if (current_event_route_.has_value()) {
    return current_event_route_->target_view_id;
  }
  return root_view_id_;
}

ScrollState* WindowRuntime::scroll_state_for_route(const EventRoute& route) {
  if (!route.target_element_id.has_value()) {
    return nullptr;
  }

  const std::span<const ElementId> ancestry(route.element_ancestry);
  const auto route_ids =
      ancestry.empty()
          ? std::span<const ElementId>(&*route.target_element_id, 1)
          : ancestry;
  for (ElementId element_id : route_ids) {
    auto* scroll = dynamic_cast<ScrollElement*>(routed_element(element_id));
    if (scroll != nullptr && scroll->enabled()) {
      return scroll->state();
    }
  }

  return nullptr;
}

EventResult WindowRuntime::dispatch_routed_element_event(
    const PlatformEvent& event,
    const EventRoute& route) {
  if (!route.target_element_id.has_value()) {
    return EventResult::unhandled();
  }

  const ElementEventContext context{
      .target_element_id = *route.target_element_id,
      .dispatch_action =
          [this](std::string_view action_name) {
            const ActionDispatchResult dispatch =
                dispatch_action(std::string(action_name));
            return dispatch.result;
          },
  };
  const std::span<const ElementId> ancestry(route.element_ancestry);
  const auto route_ids =
      ancestry.empty()
          ? std::span<const ElementId>(&*route.target_element_id, 1)
          : ancestry;
  for (ElementId element_id : route_ids) {
    Element* element = routed_element(element_id);
    if (element == nullptr || !element->enabled()) {
      continue;
    }

    const EventResult result = element->handle_event(event, context);
    if (result.consumed || result.cancelled) {
      return result;
    }
  }

  return EventResult::unhandled();
}

Element* WindowRuntime::routed_element(ElementId element_id) {
  if (element_id.value == 0) {
    return nullptr;
  }
  if (owned_element_tree_ != nullptr) {
    return owned_element_tree_->get(element_id);
  }
  if (element_root_ != nullptr && element_root_->id() == element_id) {
    return const_cast<Element*>(element_root_);
  }
  return nullptr;
}

const Element* WindowRuntime::routed_element(ElementId element_id) const {
  if (element_id.value == 0) {
    return nullptr;
  }
  if (owned_element_tree_ != nullptr) {
    return owned_element_tree_->get(element_id);
  }
  if (element_root_ != nullptr && element_root_->id() == element_id) {
    return element_root_;
  }
  return nullptr;
}

WindowRuntimeContext WindowRuntime::context() {
  ViewInputState input = input_state();

  return WindowRuntimeContext{
      .runtime = *this,
      .application = application_,
      .window = *window_,
      .renderer = *renderer_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
      .scale = scale_,
      .input = input,
      .event_route = current_event_route_,
      .last_event_result = last_event_result_,
      .last_event_dispatch = last_event_dispatch_,
      .frame_index = frame_index_};
}

void WindowRuntime::set_after_frame_callback(
    WindowRuntimeFrameCallback callback) {
  after_frame_callback_ = std::move(callback);
}

void WindowRuntime::set_after_render_callback(
    WindowRuntimeRenderCallback callback) {
  after_render_callback_ = std::move(callback);
}

void WindowRuntime::set_after_event_callback(
    WindowRuntimeEventCallback callback) {
  after_event_callback_ = std::move(callback);
}

void WindowRuntime::set_close_requested_callback(
    WindowRuntimeFrameCallback callback) {
  close_requested_callback_ = std::move(callback);
}

void WindowRuntime::set_error_callback(WindowRuntimeErrorCallback callback) {
  error_callback_ = std::move(callback);
}

void WindowRuntime::set_element_root(const Element* element) {
  owned_element_tree_.reset();
  element_root_ = element;
  apply_focused_text_ime_placement();
}

void WindowRuntime::set_element_tree(std::unique_ptr<ElementTree> tree) {
  owned_element_tree_ = std::move(tree);
  element_root_ = nullptr;
  apply_focused_text_ime_placement();
}

const ElementTree* WindowRuntime::element_tree() const {
  return owned_element_tree_.get();
}

const Element* WindowRuntime::element_root() const {
  if (owned_element_tree_ != nullptr) {
    return owned_element_tree_->get(owned_element_tree_->root_id());
  }
  return element_root_;
}

void WindowRuntime::capture_pointer(PointerCaptureOwner owner) {
  if (is_valid_pointer_capture_owner(owner)) {
    pointer_capture_owner_ = owner;
  }
}

void WindowRuntime::release_pointer(PointerCaptureOwner owner) {
  if (pointer_capture_owner_ == owner) {
    pointer_capture_owner_.reset();
  }
}

void FocusHandle::request(WindowRuntime& runtime) const {
  runtime.request_keyboard_focus(id_);
}

void FocusHandle::request(const WindowRuntimeContext& context) const {
  context.runtime.request_keyboard_focus(id_);
}

void FocusHandle::release(WindowRuntime& runtime) const {
  runtime.release_keyboard_focus(id_);
}

void FocusHandle::release(const WindowRuntimeContext& context) const {
  context.runtime.release_keyboard_focus(id_);
}

bool FocusHandle::contains(const ViewInputState& input) const {
  return id_.value != 0 && input.keyboard_focus_element_owner == id_;
}

bool FocusHandle::contains(const WindowRuntime& runtime) const {
  return contains(runtime.input_state());
}

bool FocusHandle::contains(const WindowRuntimeContext& context) const {
  return contains(context.input_state());
}

bool FocusHandle::focused(const ViewInputState& input) const {
  return contains(input);
}

bool FocusHandle::focused(const WindowRuntime& runtime) const {
  return contains(runtime);
}

bool FocusHandle::focused(const WindowRuntimeContext& context) const {
  return contains(context);
}

void WindowRuntime::request_keyboard_focus() {
  request_keyboard_focus(root_view_id_);
}

void WindowRuntime::request_keyboard_focus(ViewId view_id) {
  keyboard_focus_owner_ = view_id;
  apply_focused_text_ime_placement();
}

void WindowRuntime::request_keyboard_focus(ElementId element_id) {
  if (element_id.value != 0) {
    keyboard_focus_element_owner_ = element_id;
    apply_focused_text_ime_placement();
  }
}

void WindowRuntime::release_keyboard_focus() {
  release_keyboard_focus(root_view_id_);
}

void WindowRuntime::release_keyboard_focus(ViewId view_id) {
  if (keyboard_focus_owner_ == view_id) {
    keyboard_focus_owner_.reset();
    apply_focused_text_ime_placement();
  }
}

void WindowRuntime::release_keyboard_focus(ElementId element_id) {
  if (keyboard_focus_element_owner_ == element_id) {
    keyboard_focus_element_owner_.reset();
    apply_focused_text_ime_placement();
  }
}

FocusHandle WindowRuntime::focus_handle(ElementId element_id) const {
  return FocusHandle(element_id);
}

ViewInputState WindowRuntime::input_state() const {
  ViewInputState input = input_;
  input.pointer_capture_owner = pointer_capture_owner_;
  input.pointer_captured = pointer_capture_owner_.has_value();
  input.keyboard_focus_owner = keyboard_focus_owner_;
  input.keyboard_focus_element_owner = keyboard_focus_element_owner_;
  input.hovered_element_id = hovered_element_id_;
  input.cursor_shape = cursor_shape_;
  input.keyboard_focused = keyboard_focus_owner_ == root_view_id_ ||
                           keyboard_focus_element_owner_.has_value();
  return input;
}

void WindowRuntime::register_action(std::string name, ActionHandler handler) {
  register_app_action(std::move(name), std::move(handler));
}

void WindowRuntime::register_app_action(
    std::string name,
    ActionHandler handler) {
  if (!name.empty() && handler) {
    action_handlers_[std::move(name)] = std::move(handler);
  }
}

void WindowRuntime::register_window_action(
    std::string name,
    ActionHandler handler) {
  if (!name.empty() && handler) {
    window_action_handlers_[std::move(name)] = std::move(handler);
  }
}

void WindowRuntime::register_view_action(
    ViewId view_id,
    std::string name,
    ActionHandler handler) {
  if (view_id.value != 0 && !name.empty() && handler) {
    view_action_handlers_[view_id.value][std::move(name)] =
        std::move(handler);
  }
}

void WindowRuntime::register_focused_element_action(
    ElementId element_id,
    std::string name,
    ActionHandler handler) {
  if (element_id.value != 0 && !name.empty() && handler) {
    focused_element_action_handlers_[element_id.value][std::move(name)] =
        std::move(handler);
  }
}

ActionDispatchResult WindowRuntime::dispatch_action(std::string name) {
  ActionDispatchResult dispatch{
      .name = std::move(name),
      .result = EventResult::unhandled()};

  if (keyboard_focus_element_owner_.has_value()) {
    const auto owner = focused_element_action_handlers_.find(
        keyboard_focus_element_owner_->value);
    if (owner != focused_element_action_handlers_.end()) {
      const auto handler = owner->second.find(dispatch.name);
      if (handler != owner->second.end()) {
        dispatch.handled = true;
        dispatch.scope = ActionScope::focused_element;
        dispatch.element_id = keyboard_focus_element_owner_;
        dispatch.result = handler->second(context());
        last_action_dispatch_ = dispatch;
        return dispatch;
      }
    }
  }

  if (const std::optional<ViewId> view_id = action_dispatch_view_id();
      view_id.has_value()) {
    const auto owner = view_action_handlers_.find(view_id->value);
    if (owner != view_action_handlers_.end()) {
      const auto handler = owner->second.find(dispatch.name);
      if (handler != owner->second.end()) {
        dispatch.handled = true;
        dispatch.scope = ActionScope::view;
        dispatch.view_id = view_id;
        dispatch.result = handler->second(context());
        last_action_dispatch_ = dispatch;
        return dispatch;
      }
    }
  }

  if (const auto handler = window_action_handlers_.find(dispatch.name);
      handler != window_action_handlers_.end()) {
    dispatch.handled = true;
    dispatch.scope = ActionScope::window;
    dispatch.result = handler->second(context());
    last_action_dispatch_ = dispatch;
    return dispatch;
  }

  if (const auto handler = action_handlers_.find(dispatch.name);
      handler != action_handlers_.end()) {
    dispatch.handled = true;
    dispatch.scope = ActionScope::app;
    dispatch.result = handler->second(context());
  }

  last_action_dispatch_ = dispatch;
  return dispatch;
}

std::optional<ActionDispatchResult> WindowRuntime::last_action_dispatch()
    const {
  return last_action_dispatch_;
}

void WindowRuntime::bind_key(KeyBinding binding) {
  if (!binding.action_name.empty()) {
    key_bindings_.push_back(std::move(binding));
  }
}

void WindowRuntime::bind_text_edit_action(TextEditBinding binding) {
  text_edit_bindings_.push_back(binding);
}

void WindowRuntime::bind_text_model(ElementId element_id, TextModel* model) {
  if (element_id.value == 0) {
    return;
  }
  if (model == nullptr) {
    text_models_.erase(element_id.value);
    return;
  }
  text_models_[element_id.value] = model;
}

TextModel* WindowRuntime::focused_text_model() {
  if (!keyboard_focus_element_owner_.has_value()) {
    return nullptr;
  }

  const auto model = text_models_.find(keyboard_focus_element_owner_->value);
  if (model != text_models_.end()) {
    return model->second;
  }

  auto* input = dynamic_cast<TextInputElement*>(
      routed_element(*keyboard_focus_element_owner_));
  return input == nullptr ? nullptr : input->model();
}

const TextModel* WindowRuntime::focused_text_model() const {
  if (!keyboard_focus_element_owner_.has_value()) {
    return nullptr;
  }

  const auto model = text_models_.find(keyboard_focus_element_owner_->value);
  if (model != text_models_.end()) {
    return model->second;
  }

  const auto* input = dynamic_cast<const TextInputElement*>(
      routed_element(*keyboard_focus_element_owner_));
  return input == nullptr ? nullptr : input->model();
}

std::optional<ImeCandidateRect> WindowRuntime::focused_text_ime_rect() const {
  const TextModel* model = focused_text_model();
  if (!keyboard_focus_element_owner_.has_value() || model == nullptr) {
    return {};
  }

  const auto* text_element = dynamic_cast<const TextElement*>(
      routed_element(*keyboard_focus_element_owner_));
  if (text_element == nullptr) {
    return {};
  }

  const std::optional<Rect> bounds = text_element->layout_bounds();
  if (!bounds.has_value()) {
    return {};
  }

  const std::size_t byte_offset = model->cursor();
  return ImeCandidateRect{
      .element_id = *keyboard_focus_element_owner_,
      .rect =
          Rect{
              .origin =
                  {
                      .x = bounds->origin.x +
                           (static_cast<float>(byte_offset) *
                            text_element->glyph_width()),
                      .y = bounds->origin.y,
                  },
              .size = {.width = 1.0F, .height = text_element->font_size()},
          },
      .byte_offset = byte_offset,
  };
}

void WindowRuntime::set_clipboard(Clipboard* clipboard) {
  clipboard_ = clipboard;
}

bool WindowRuntime::paste_clipboard_text() {
  if (clipboard_ == nullptr) {
    return false;
  }

  const auto text = clipboard_->read_text();
  if (!text.has_value()) {
    return false;
  }

  TextModel* model = focused_text_model();
  if (model == nullptr) {
    return false;
  }

  model->insert_text(*text);
  return true;
}

bool WindowRuntime::copy_selection_to_clipboard() {
  if (clipboard_ == nullptr) {
    return false;
  }

  TextModel* model = focused_text_model();
  if (model == nullptr) {
    return false;
  }

  const std::string selected_text = model->selected_text();
  if (selected_text.empty()) {
    return false;
  }
  return clipboard_->write_text(selected_text);
}

bool WindowRuntime::cut_selection_to_clipboard() {
  if (!copy_selection_to_clipboard()) {
    return false;
  }

  TextModel* model = focused_text_model();
  if (model == nullptr) {
    return false;
  }

  return model->delete_forward();
}

void WindowRuntime::set_element_cursor(
    ElementId element_id,
    CursorShape cursor_shape) {
  if (element_id.value == 0) {
    return;
  }
  element_cursors_[element_id.value] = cursor_shape;
}

void WindowRuntime::request_render() {
  invalidation_state_.render = true;
  invalidation_state_.layout = true;
  invalidation_state_.paint = true;
  schedule_redraw();
}

void WindowRuntime::request_layout() {
  invalidation_state_.layout = true;
  invalidation_state_.paint = true;
  schedule_redraw();
}

void WindowRuntime::request_paint() {
  invalidation_state_.paint = true;
  schedule_redraw();
}

void WindowRuntime::defer(DeferredCallback callback) {
  if (!callback) {
    return;
  }
  deferred_callbacks_.push_back(std::move(callback));
}

TimerId WindowRuntime::schedule_timer(
    std::uint64_t delay_ms,
    TimerCallback callback) {
  if (!callback) {
    return {};
  }
  const TimerId id{next_timer_id_++};
  timers_.push_back(RuntimeTimer{
      .id = id,
      .due_ms = current_time_ms_ + delay_ms,
      .interval_ms = 0,
      .repeating = false,
      .callback = std::move(callback),
  });
  return id;
}

TimerId WindowRuntime::schedule_repeating_timer(
    std::uint64_t interval_ms,
    TimerCallback callback) {
  if (!callback || interval_ms == 0) {
    return {};
  }
  const TimerId id{next_timer_id_++};
  timers_.push_back(RuntimeTimer{
      .id = id,
      .due_ms = current_time_ms_ + interval_ms,
      .interval_ms = interval_ms,
      .repeating = true,
      .callback = std::move(callback),
  });
  return id;
}

bool WindowRuntime::cancel_timer(TimerId id) {
  if (id.value == 0) {
    return false;
  }
  const auto timer = std::find_if(
      timers_.begin(),
      timers_.end(),
      [id](const RuntimeTimer& timer) {
        return timer.id == id;
      });
  if (timer == timers_.end()) {
    return false;
  }
  timers_.erase(timer);
  return true;
}

void WindowRuntime::advance_time(std::uint64_t delta_ms) {
  current_time_ms_ += delta_ms;
  fire_due_timers();
  flush_deferred_redraw_request();
}

TaskHandle WindowRuntime::spawn_task(TaskCompletionCallback callback) {
  if (!callback) {
    return {};
  }

  const TaskId id{next_task_id_++};
  tasks_.push_back(RuntimeTask{
      .id = id,
      .callback = std::move(callback),
      .queued = false,
      .completed = false,
  });
  return TaskHandle(*this, id);
}

bool WindowRuntime::complete_task(TaskId id) {
  if (id.value == 0) {
    return false;
  }

  const auto task = std::find_if(
      tasks_.begin(),
      tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  if (task == tasks_.end() || task->queued || task->completed) {
    return false;
  }

  task->queued = true;
  task_completion_queue_.push_back(id);
  return true;
}

void WindowRuntime::drain_task_completions() {
  if (draining_task_completions_ || should_quit_) {
    return;
  }

  draining_task_completions_ = true;
  while (!task_completion_queue_.empty() && !should_quit_) {
    std::vector<TaskId> queued;
    queued.swap(task_completion_queue_);
    for (const TaskId id : queued) {
      const auto task = std::find_if(
          tasks_.begin(),
          tasks_.end(),
          [id](const RuntimeTask& task) {
            return task.id == id;
          });
      if (task == tasks_.end() || task->completed) {
        continue;
      }

      TaskCompletionCallback callback = task->callback;
      task->queued = false;
      task->completed = true;
      if (callback) {
        callback(context());
      }
    }
  }
  draining_task_completions_ = false;
  flush_deferred_redraw_request();
}

void WindowRuntime::batch_updates(UpdateBatchCallback callback) {
  if (!callback || should_quit_) {
    return;
  }

  update_batch_depth_ += 1;
  callback(context());
  update_batch_depth_ -= 1;
  if (update_batch_depth_ == 0) {
    flush_deferred_redraw_request();
  }
}

void WindowRuntime::clear_invalidation() {
  invalidation_state_ = {};
}

InvalidationState WindowRuntime::invalidation_state() const {
  return invalidation_state_;
}

std::optional<RenderRecord> WindowRuntime::last_render_record() const {
  return last_render_record_;
}

RuntimeDiagnosticsSnapshot WindowRuntime::diagnostics_snapshot() const {
  std::size_t connected_subscription_count = 0;
  for (const EntityObserver& observer : entity_observers_) {
    if (observer.subscription_id.value != 0 && observer.callback) {
      connected_subscription_count += 1;
    }
  }

  return RuntimeDiagnosticsSnapshot{
      .entity_store_count = entity_stores_.size(),
      .entity_count = entity_count_,
      .view_entity_subscription_count = entity_subscriptions_.size(),
      .entity_observer_count = entity_observers_.size(),
      .connected_subscription_count = connected_subscription_count,
      .invalidation = invalidation_state_,
      .frame_index = frame_index_,
      .last_render_record = last_render_record_,
      .last_frame_statistics = last_frame_statistics_,
  };
}

std::span<const EntitySubscription> WindowRuntime::subscriptions_for_view(
    ViewId view_id) const {
  subscription_query_buffer_.clear();
  for (const EntitySubscription& subscription : entity_subscriptions_) {
    if (subscription.view_id == view_id) {
      subscription_query_buffer_.push_back(subscription);
    }
  }
  return subscription_query_buffer_;
}

bool WindowRuntime::subscription_connected(SubscriptionId id) const {
  if (id.value == 0) {
    return false;
  }

  for (const EntityObserver& observer : entity_observers_) {
    if (observer.subscription_id == id && observer.callback) {
      return true;
    }
  }
  return false;
}

bool WindowRuntime::remove_subscription(SubscriptionId id) {
  if (id.value == 0) {
    return false;
  }

  for (EntityObserver& observer : entity_observers_) {
    if (observer.subscription_id == id && observer.callback) {
      observer.callback = {};
      return true;
    }
  }
  return false;
}

void WindowRuntime::schedule_redraw() {
  if (window_ == nullptr || redraw_scheduled_ || should_quit_) {
    return;
  }
  redraw_scheduled_ = true;
  if (dispatching_view_event_ || draining_deferred_callbacks_ || firing_timers_ ||
      draining_task_completions_ || update_batch_depth_ > 0) {
    deferred_redraw_request_ = true;
    return;
  }
  window_->request_redraw();
}

void WindowRuntime::flush_deferred_redraw_request() {
  if (!deferred_redraw_request_ || window_ == nullptr || should_quit_) {
    return;
  }
  deferred_redraw_request_ = false;
  window_->request_redraw();
}

void WindowRuntime::drain_deferred_callbacks() {
  while (!deferred_callbacks_.empty() && !should_quit_) {
    std::vector<DeferredCallback> callbacks;
    callbacks.swap(deferred_callbacks_);
    draining_deferred_callbacks_ = true;
    for (DeferredCallback& callback : callbacks) {
      if (callback) {
        callback(context());
      }
    }
    draining_deferred_callbacks_ = false;
  }
}

void WindowRuntime::fire_due_timers() {
  if (firing_timers_ || should_quit_) {
    return;
  }

  firing_timers_ = true;
  while (!should_quit_) {
    auto timer = std::find_if(
        timers_.begin(),
        timers_.end(),
        [this](const RuntimeTimer& timer) {
          return timer.callback && timer.due_ms <= current_time_ms_;
        });
    if (timer == timers_.end()) {
      break;
    }

    TimerCallback callback = timer->callback;
    if (timer->repeating) {
      timer->due_ms += timer->interval_ms;
    } else {
      timer = timers_.erase(timer);
    }
    if (callback) {
      callback(context());
    }
  }
  firing_timers_ = false;
}

bool WindowRuntime::task_active(TaskId id) const {
  if (id.value == 0) {
    return false;
  }

  const auto task = std::find_if(
      tasks_.begin(),
      tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  return task != tasks_.end() && !task->queued && !task->completed;
}

bool WindowRuntime::task_complete(TaskId id) const {
  if (id.value == 0) {
    return false;
  }

  const auto task = std::find_if(
      tasks_.begin(),
      tasks_.end(),
      [id](const RuntimeTask& task) {
        return task.id == id;
      });
  return task != tasks_.end() && task->completed;
}

void WindowRuntime::apply_cursor_shape(CursorShape cursor_shape) {
  if (window_ == nullptr || applied_cursor_shape_ == cursor_shape) {
    return;
  }
  applied_cursor_shape_ = cursor_shape;
  window_->set_cursor(cursor_shape);
}

void WindowRuntime::apply_focused_text_ime_placement() {
  std::optional<ImeTextInputPlacement> placement;
  if (const std::optional<ImeCandidateRect> candidate =
          focused_text_ime_rect();
      candidate.has_value()) {
    placement = ImeTextInputPlacement{
        .rect = candidate->rect,
        .byte_offset = candidate->byte_offset,
    };
  }

  if (ime_text_input_placement_equal(
          applied_ime_text_input_placement_,
          placement)) {
    return;
  }

  applied_ime_text_input_placement_ = placement;
  if (window_ != nullptr) {
    window_->set_ime_text_input_placement(placement);
  }
}

ViewId WindowRuntime::allocate_view_id() {
  const ViewId view_id{next_view_id_};
  next_view_id_ += 1;
  return view_id;
}

bool WindowRuntime::is_view_id_allocated(ViewId view_id) const {
  return view_id.value != 0 && view_id.value < next_view_id_ &&
         !removed_view_ids_.contains(view_id.value);
}

std::optional<ViewId> WindowRuntime::upgrade_view(WeakView view) const {
  if (view.empty() || !is_view_id_allocated(view.id())) {
    return std::nullopt;
  }
  return view.id();
}

bool WindowRuntime::notify_entity_changed(
    std::type_index entity_type,
    std::uint64_t entity_id_value) {
  bool notified = false;
  for (const EntitySubscription& subscription : entity_subscriptions_) {
    if (subscription.entity_type == entity_type &&
        subscription.entity_id_value == entity_id_value) {
      notified = true;
    }
  }
  for (const EntityObserver& observer : entity_observers_) {
    if (observer.entity_type == entity_type &&
        observer.entity_id_value == entity_id_value &&
        observer.callback) {
      notified = true;
      observer.callback(context(), entity_id_value);
    }
  }
  if (notified) {
    request_render();
  }
  return notified;
}

Result<void> WindowRuntime::resize_surface(Size size, DpiScale scale) {
  framebuffer_size_ = size;
  scale_ = scale;
  viewport_size_ = to_logical_pixels(framebuffer_size_, scale_);
  if (renderer_ == nullptr) {
    return {};
  }

  auto result = renderer_->resize(size, scale);
  if (!result) {
    fail_and_quit(result.error());
  }
  return result;
}

ViewId WindowRuntimeContext::allocate_view_id() const {
  return runtime.allocate_view_id();
}

std::optional<EventRoute> WindowRuntimeContext::current_event_route() const {
  return event_route;
}

ViewInputState WindowRuntimeContext::input_state() const {
  return input;
}

bool WindowRuntimeContext::is_view_id_allocated(ViewId view_id) const {
  return runtime.is_view_id_allocated(view_id);
}

std::optional<ViewId> WindowRuntimeContext::upgrade_view(WeakView view) const {
  return runtime.upgrade_view(view);
}

void WindowRuntimeContext::capture_pointer(PointerCaptureOwner owner) const {
  runtime.capture_pointer(owner);
}

void WindowRuntimeContext::capture_pointer(ElementId element_id) const {
  runtime.capture_pointer(PointerCaptureOwner::element(element_id));
}

void WindowRuntimeContext::release_pointer(PointerCaptureOwner owner) const {
  runtime.release_pointer(owner);
}

void WindowRuntimeContext::release_pointer(ElementId element_id) const {
  runtime.release_pointer(PointerCaptureOwner::element(element_id));
}

void WindowRuntimeContext::request_keyboard_focus() const {
  runtime.request_keyboard_focus();
}

void WindowRuntimeContext::request_keyboard_focus(ViewId view_id) const {
  runtime.request_keyboard_focus(view_id);
}

void WindowRuntimeContext::request_keyboard_focus(ElementId element_id) const {
  runtime.request_keyboard_focus(element_id);
}

void WindowRuntimeContext::focus(ElementId element_id) const {
  runtime.request_keyboard_focus(element_id);
}

void WindowRuntimeContext::release_keyboard_focus() const {
  runtime.release_keyboard_focus();
}

void WindowRuntimeContext::release_keyboard_focus(ViewId view_id) const {
  runtime.release_keyboard_focus(view_id);
}

void WindowRuntimeContext::release_keyboard_focus(ElementId element_id) const {
  runtime.release_keyboard_focus(element_id);
}

void WindowRuntimeContext::blur(ElementId element_id) const {
  runtime.release_keyboard_focus(element_id);
}

FocusHandle WindowRuntimeContext::focus_handle(ElementId element_id) const {
  return runtime.focus_handle(element_id);
}

void WindowRuntimeContext::set_element_tree(
    std::unique_ptr<ElementTree> tree) const {
  runtime.set_element_tree(std::move(tree));
}

void WindowRuntimeContext::register_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_action(std::move(name), std::move(handler));
}

void WindowRuntimeContext::register_app_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_app_action(std::move(name), std::move(handler));
}

void WindowRuntimeContext::register_window_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_window_action(std::move(name), std::move(handler));
}

void WindowRuntimeContext::register_view_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_view_action(view_id, std::move(name), std::move(handler));
}

void WindowRuntimeContext::register_view_action(
    ViewId target_view_id,
    std::string name,
    ActionHandler handler) const {
  runtime.register_view_action(
      target_view_id,
      std::move(name),
      std::move(handler));
}

void WindowRuntimeContext::register_focused_element_action(
    ElementId element_id,
    std::string name,
    ActionHandler handler) const {
  runtime.register_focused_element_action(
      element_id,
      std::move(name),
      std::move(handler));
}

ActionDispatchResult WindowRuntimeContext::dispatch_action(
    std::string name) const {
  return runtime.dispatch_action(std::move(name));
}

std::optional<ActionDispatchResult> WindowRuntimeContext::last_action_dispatch()
    const {
  return runtime.last_action_dispatch();
}

void WindowRuntimeContext::bind_key(KeyBinding binding) const {
  runtime.bind_key(std::move(binding));
}

void WindowRuntimeContext::bind_text_edit_action(
    TextEditBinding binding) const {
  runtime.bind_text_edit_action(std::move(binding));
}

void WindowRuntimeContext::bind_text_model(
    ElementId element_id,
    TextModel* model) const {
  runtime.bind_text_model(element_id, model);
}

bool WindowRuntimeContext::mutate_focused_text_model(
    FocusedTextModelMutation mutation) const {
  if (!mutation) {
    return false;
  }

  TextModel* model = runtime.focused_text_model();
  if (model == nullptr) {
    return false;
  }

  mutation(*model);
  return true;
}

std::optional<ImeCandidateRect> WindowRuntimeContext::focused_text_ime_rect()
    const {
  return runtime.focused_text_ime_rect();
}

void WindowRuntimeContext::set_element_cursor(
    ElementId element_id,
    CursorShape cursor_shape) const {
  runtime.set_element_cursor(element_id, cursor_shape);
}

bool WindowRuntimeContext::paste_clipboard_text() const {
  return runtime.paste_clipboard_text();
}

bool WindowRuntimeContext::copy_selection_to_clipboard() const {
  return runtime.copy_selection_to_clipboard();
}

bool WindowRuntimeContext::cut_selection_to_clipboard() const {
  return runtime.cut_selection_to_clipboard();
}

void WindowRuntimeContext::request_render() const {
  runtime.request_render();
}

void WindowRuntimeContext::request_layout() const {
  runtime.request_layout();
}

void WindowRuntimeContext::request_paint() const {
  runtime.request_paint();
}

void WindowRuntimeContext::defer(DeferredCallback callback) const {
  runtime.defer(std::move(callback));
}

TimerId WindowRuntimeContext::schedule_timer(
    std::uint64_t delay_ms,
    TimerCallback callback) const {
  return runtime.schedule_timer(delay_ms, std::move(callback));
}

TimerId WindowRuntimeContext::schedule_repeating_timer(
    std::uint64_t interval_ms,
    TimerCallback callback) const {
  return runtime.schedule_repeating_timer(interval_ms, std::move(callback));
}

TaskHandle WindowRuntimeContext::spawn_task(
    TaskCompletionCallback callback) const {
  return runtime.spawn_task(std::move(callback));
}

void WindowRuntimeContext::batch_updates(
    UpdateBatchCallback callback) const {
  runtime.batch_updates(std::move(callback));
}

void WindowRuntimeContext::clear_invalidation() const {
  runtime.clear_invalidation();
}

InvalidationState WindowRuntimeContext::invalidation_state() const {
  return runtime.invalidation_state();
}

RuntimeDiagnosticsSnapshot WindowRuntimeContext::diagnostics_snapshot() const {
  return runtime.diagnostics_snapshot();
}

} // namespace cgpui
