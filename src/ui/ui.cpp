#include "cgpui/ui/ui.hpp"

#include <algorithm>
#include <expected>
#include <memory>
#include <utility>
#include <variant>

namespace cgpui {
namespace {

EventKind event_kind_for(const PlatformEvent& event) {
  if (std::holds_alternative<WindowFocused>(event)) {
    return EventKind::window_focused;
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

bool modifiers_equal(KeyboardModifiers lhs, KeyboardModifiers rhs) {
  return lhs.shift == rhs.shift && lhs.control == rhs.control &&
         lhs.alt == rhs.alt && lhs.super == rhs.super;
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
    return;
  }
  if (const ElementId* element_id = owner.element_id();
      element_id != nullptr) {
    route.target_element_id = *element_id;
  }
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

} // namespace

void PaintList::clear() {
  commands_.clear();
  clip_stack_.clear();
}

void PaintList::push_clip(Rect rect) {
  clip_stack_.push_back(rect);
}

void PaintList::pop_clip() {
  if (!clip_stack_.empty()) {
    clip_stack_.pop_back();
  }
}

void PaintList::fill_rect(Rect rect, Color color) {
  commands_.push_back(PaintCommand{
      .solid_rect = SolidRect{.rect = rect, .color = color},
      .clip_rect = clip_stack_.empty()
                       ? std::optional<Rect>{}
                       : std::optional<Rect>{clip_stack_.back()}});
}

std::span<const PaintCommand> PaintList::commands() const {
  return commands_;
}

void StyledElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  const bool uses_hidden_overflow_clip =
      bounds.has_value() && style_.overflow == Overflow::hidden;
  if (uses_hidden_overflow_clip) {
    paint_list.push_clip(
        style_.clip_rect.has_value() ? *style_.clip_rect : *bounds);
  }
  if (bounds.has_value() && style_.background_color.has_value()) {
    paint_list.fill_rect(*bounds, *style_.background_color);
  }
  if (bounds.has_value() && style_.border_color.has_value()) {
    const Rect rect = *bounds;
    const Color color = *style_.border_color;
    const float top = style_.border_width.top;
    const float right = style_.border_width.right;
    const float bottom = style_.border_width.bottom;
    const float left = style_.border_width.left;
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
  if (child_ != nullptr) {
    child_->paint(paint_list);
  }
  if (uses_hidden_overflow_clip) {
    paint_list.pop_clip();
  }
}

void TextElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || text().empty()) {
    return;
  }
  paint_list.fill_rect(
      *bounds,
      Color{.r = 0.82F, .g = 0.86F, .b = 0.92F, .a = 1.0F});
}

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size) {
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

  PaintList paint_list;
  view.paint(paint_list, viewport_size);
  for (const auto& command : paint_list.commands()) {
    (*frame)->draw_rect(command.solid_rect);
  }

  return (*frame)->present();
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
  return runtime.run(options.window, options.runtime);
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
      .event_kind = event_kind_for(event)};
}

WindowRuntime::WindowRuntime(
    PlatformApplication& application,
    View& view,
    RendererFactory renderer_factory)
    : application_(application),
      view_(view),
      renderer_factory_(std::move(renderer_factory)) {}

int WindowRuntime::run(
    const WindowDescriptor& descriptor,
    WindowRuntimeOptions options) {
  should_quit_ = false;
  failed_ = false;
  window_ = nullptr;
  renderer_ = nullptr;
  viewport_size_ = descriptor.size;
  input_ = {};
  pointer_capture_owner_.reset();
  keyboard_focus_owner_.reset();
  keyboard_focus_element_owner_.reset();
  hovered_element_id_.reset();
  cursor_shape_ = CursorShape::default_arrow;
  last_event_result_ = EventResult::unhandled();
  last_event_dispatch_.reset();
  last_action_dispatch_.reset();
  current_event_route_.reset();
  invalidation_state_ = {};
  subscription_query_buffer_.clear();
  dispatching_view_event_ = false;
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  event_dispatch_sequence_ = 0;
  frame_index_ = 0;

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
  viewport_size_ = window_state.framebuffer_size;

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

  if (options.request_initial_redraw) {
    redraw_scheduled_ = true;
    window_->request_redraw();
  }

  const int run_result = application_.run();
  window_ = nullptr;
  renderer_ = nullptr;

  if (failed_) {
    return 1;
  }
  return run_result;
}

void WindowRuntime::handle_event(const PlatformEvent& event) {
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    should_quit_ = true;
    if (close_requested_callback_ && window_ != nullptr &&
        renderer_ != nullptr) {
      close_requested_callback_(context());
    }
    application_.quit();
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
      for (const KeyBinding& binding : key_bindings_) {
        if (binding.key_code == key->key_code &&
            binding.action == key->action &&
            modifiers_equal(binding.modifiers, key->modifiers)) {
          (void)dispatch_action(binding.action_name);
          break;
        }
      }
      if (keyboard_focus_element_owner_.has_value()) {
        const auto model =
            text_models_.find(keyboard_focus_element_owner_->value);
        if (model != text_models_.end() && model->second != nullptr) {
          for (const TextEditBinding& binding : text_edit_bindings_) {
            if (binding.key_code == key->key_code &&
                binding.action == key->action &&
                modifiers_equal(binding.modifiers, key->modifiers)) {
              (void)model->second->apply_edit_action(binding.edit_action);
              break;
            }
          }
        }
      }
    }
    if (const auto* text = std::get_if<TextInput>(&event);
        text != nullptr && keyboard_focus_element_owner_.has_value()) {
      const auto model =
          text_models_.find(keyboard_focus_element_owner_->value);
      if (model != text_models_.end() && model->second != nullptr) {
        model->second->insert_text(text->text);
      }
    }
    if (const auto* composition = std::get_if<ImeComposition>(&event);
        composition != nullptr && keyboard_focus_element_owner_.has_value()) {
      const auto model =
          text_models_.find(keyboard_focus_element_owner_->value);
      if (model != text_models_.end() && model->second != nullptr) {
        switch (composition->phase) {
          case ImeCompositionPhase::update:
            model->second->set_composition_text(composition->text);
            break;
          case ImeCompositionPhase::commit:
            model->second->set_composition_text(composition->text);
            model->second->commit_composition();
            break;
          case ImeCompositionPhase::cancel:
            model->second->cancel_composition();
            break;
        }
      }
    }
    EventResult result = EventResult::unhandled();
    if (current_event_route_->target_element_id.has_value()) {
      if (Element* element =
              routed_element(*current_event_route_->target_element_id);
          element != nullptr && element->enabled()) {
        result = element->handle_event(
            event,
            ElementEventContext{
                .target_element_id =
                    *current_event_route_->target_element_id,
            });
      }
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
    flush_deferred_redraw_request();
  }
}

void WindowRuntime::handle_resize(const WindowResized& event) {
  (void)resize_surface(event.size, event.scale);
}

void WindowRuntime::handle_redraw() {
  if (renderer_ == nullptr || should_quit_) {
    return;
  }

  if (owned_element_tree_ != nullptr) {
    (void)owned_element_tree_->layout_root(LayoutInput{
        .constraints =
            {
                .max_size = viewport_size_,
            },
    });
  }

  auto result = render_view(*renderer_, view_, viewport_size_);
  if (!result) {
    fail_and_quit(result.error());
    return;
  }

  clear_invalidation();
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  frame_index_ += 1;
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
  ViewInputState input = input_;
  input.pointer_capture_owner = pointer_capture_owner_;
  input.pointer_captured = pointer_capture_owner_.has_value();
  input.keyboard_focus_owner = keyboard_focus_owner_;
  input.keyboard_focus_element_owner = keyboard_focus_element_owner_;
  input.hovered_element_id = hovered_element_id_;
  input.cursor_shape = cursor_shape_;
  input.keyboard_focused = keyboard_focus_owner_ == root_view_id_ ||
                           keyboard_focus_element_owner_.has_value();

  return WindowRuntimeContext{
      .runtime = *this,
      .application = application_,
      .window = *window_,
      .renderer = *renderer_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
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
}

void WindowRuntime::set_element_tree(std::unique_ptr<ElementTree> tree) {
  owned_element_tree_ = std::move(tree);
  element_root_ = nullptr;
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

void WindowRuntime::request_keyboard_focus() {
  request_keyboard_focus(root_view_id_);
}

void WindowRuntime::request_keyboard_focus(ViewId view_id) {
  keyboard_focus_owner_ = view_id;
}

void WindowRuntime::request_keyboard_focus(ElementId element_id) {
  if (element_id.value != 0) {
    keyboard_focus_element_owner_ = element_id;
  }
}

void WindowRuntime::release_keyboard_focus() {
  release_keyboard_focus(root_view_id_);
}

void WindowRuntime::release_keyboard_focus(ViewId view_id) {
  if (keyboard_focus_owner_ == view_id) {
    keyboard_focus_owner_.reset();
  }
}

void WindowRuntime::release_keyboard_focus(ElementId element_id) {
  if (keyboard_focus_element_owner_ == element_id) {
    keyboard_focus_element_owner_.reset();
  }
}

void WindowRuntime::register_action(std::string name, ActionHandler handler) {
  if (!name.empty() && handler) {
    action_handlers_[std::move(name)] = std::move(handler);
  }
}

ActionDispatchResult WindowRuntime::dispatch_action(std::string name) {
  ActionDispatchResult dispatch{
      .name = std::move(name),
      .result = EventResult::unhandled()};

  const auto handler = action_handlers_.find(dispatch.name);
  if (handler != action_handlers_.end()) {
    dispatch.handled = true;
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

void WindowRuntime::set_clipboard(Clipboard* clipboard) {
  clipboard_ = clipboard;
}

bool WindowRuntime::paste_clipboard_text() {
  if (clipboard_ == nullptr || !keyboard_focus_element_owner_.has_value()) {
    return false;
  }

  const auto text = clipboard_->read_text();
  if (!text.has_value()) {
    return false;
  }

  const auto model = text_models_.find(keyboard_focus_element_owner_->value);
  if (model == text_models_.end() || model->second == nullptr) {
    return false;
  }

  model->second->insert_text(*text);
  return true;
}

bool WindowRuntime::copy_selection_to_clipboard() {
  if (clipboard_ == nullptr || !keyboard_focus_element_owner_.has_value()) {
    return false;
  }

  const auto model = text_models_.find(keyboard_focus_element_owner_->value);
  if (model == text_models_.end() || model->second == nullptr) {
    return false;
  }

  const std::string selected_text = model->second->selected_text();
  if (selected_text.empty()) {
    return false;
  }
  return clipboard_->write_text(selected_text);
}

bool WindowRuntime::cut_selection_to_clipboard() {
  if (!copy_selection_to_clipboard() ||
      !keyboard_focus_element_owner_.has_value()) {
    return false;
  }

  const auto model = text_models_.find(keyboard_focus_element_owner_->value);
  if (model == text_models_.end() || model->second == nullptr) {
    return false;
  }

  return model->second->delete_forward();
}

void WindowRuntime::set_element_cursor(
    ElementId element_id,
    CursorShape cursor_shape) {
  if (element_id.value == 0) {
    return;
  }
  element_cursors_[element_id.value] = cursor_shape;
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

void WindowRuntime::clear_invalidation() {
  invalidation_state_ = {};
}

InvalidationState WindowRuntime::invalidation_state() const {
  return invalidation_state_;
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

void WindowRuntime::schedule_redraw() {
  if (window_ == nullptr || redraw_scheduled_ || should_quit_) {
    return;
  }
  redraw_scheduled_ = true;
  if (dispatching_view_event_) {
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

ViewId WindowRuntime::allocate_view_id() {
  const ViewId view_id{next_view_id_};
  next_view_id_ += 1;
  return view_id;
}

bool WindowRuntime::is_view_id_allocated(ViewId view_id) const {
  return view_id.value != 0 && view_id.value < next_view_id_;
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
  if (notified) {
    request_layout();
  }
  return notified;
}

Result<void> WindowRuntime::resize_surface(Size size, DpiScale scale) {
  viewport_size_ = size;
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

bool WindowRuntimeContext::is_view_id_allocated(ViewId view_id) const {
  return runtime.is_view_id_allocated(view_id);
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

void WindowRuntimeContext::set_element_tree(
    std::unique_ptr<ElementTree> tree) const {
  runtime.set_element_tree(std::move(tree));
}

void WindowRuntimeContext::register_action(
    std::string name,
    ActionHandler handler) const {
  runtime.register_action(std::move(name), std::move(handler));
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

void WindowRuntimeContext::request_layout() const {
  runtime.request_layout();
}

void WindowRuntimeContext::request_paint() const {
  runtime.request_paint();
}

void WindowRuntimeContext::clear_invalidation() const {
  runtime.clear_invalidation();
}

InvalidationState WindowRuntimeContext::invalidation_state() const {
  return runtime.invalidation_state();
}

} // namespace cgpui
