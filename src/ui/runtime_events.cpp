#include "ui_internal.hpp"

namespace cgpui {

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

  if (std::holds_alternative<WindowWakeupRequested>(event)) {
    handle_wakeup();
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
    if (std::holds_alternative<DragEntered>(event) ||
        std::holds_alternative<DragUpdated>(event) ||
        std::holds_alternative<DragDropped>(event) ||
        std::holds_alternative<DragExited>(event)) {
      record_platform_diagnostic(PlatformDiagnosticEvent{
          .kind = PlatformDiagnosticKind::drag_drop,
          .event_kind = event_kind_for(event),
          .backend = "runtime",
          .operation = drag_drop_operation_for(event),
          .supported = true,
          .succeeded = true,
          .value_count = drag_drop_payload_value_count(event),
      });
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
    if (const auto* delete_surrounding =
            std::get_if<ImeDeleteSurroundingText>(&event);
        delete_surrounding != nullptr &&
        keyboard_focus_element_owner_.has_value()) {
      if (TextModel* model = focused_text_model(); model != nullptr) {
        (void)model->delete_surrounding_text(
            delete_surrounding->before_length,
            delete_surrounding->after_length);
      }
    }
    const bool text_selection_consumed =
        apply_text_pointer_selection(event, *current_event_route_);
    EventResult result = text_selection_consumed ? EventResult::consumed_event()
                                                 : EventResult::unhandled();
    if (!result.consumed && !result.cancelled) {
      if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
          scrolled != nullptr) {
        if (ScrollState* state = scroll_state_for_route(*current_event_route_);
            state != nullptr) {
          state->scroll_by(scrolled->delta);
          result = EventResult::consumed_event();
        }
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
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::window_lifecycle,
      .event_kind = event_kind_for(event),
      .backend = "runtime",
      .operation = "window-lifecycle",
      .supported = true,
      .succeeded = true,
      .value_count = 1,
  });
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

bool WindowRuntime::apply_text_pointer_selection(
    const PlatformEvent& event,
    const EventRoute& route) {
  if (const auto* button = std::get_if<PointerButton>(&event);
      button != nullptr && button->button == MouseButton::left) {
    if (button->pressed) {
      text_pointer_selection_drag_.reset();
      if (!route.target_element_id.has_value()) {
        return false;
      }

      TextInputElement* input = routed_text_input(*route.target_element_id);
      if (input == nullptr || !input->enabled() || input->model() == nullptr) {
        return false;
      }

      const std::optional<std::size_t> offset =
          text_offset_for_point(*input, button->position);
      if (!offset.has_value()) {
        return false;
      }

      input->model()->set_selection(*offset, *offset);
      text_pointer_selection_drag_ = TextPointerSelectionDrag{
          .element_id = *route.target_element_id,
          .anchor_offset = *offset,
      };
      return false;
    }

    if (!text_pointer_selection_drag_.has_value()) {
      return false;
    }

    const TextPointerSelectionDrag drag = *text_pointer_selection_drag_;
    text_pointer_selection_drag_.reset();
    TextInputElement* input = routed_text_input(drag.element_id);
    if (input == nullptr || input->model() == nullptr) {
      return true;
    }

    const std::optional<std::size_t> offset =
        text_offset_for_point(*input, button->position);
    if (offset.has_value()) {
      input->model()->set_selection(drag.anchor_offset, *offset);
    }
    return true;
  }

  if (const auto* moved = std::get_if<PointerMoved>(&event);
      moved != nullptr && text_pointer_selection_drag_.has_value()) {
    const TextPointerSelectionDrag drag = *text_pointer_selection_drag_;
    TextInputElement* input = routed_text_input(drag.element_id);
    if (input == nullptr || input->model() == nullptr) {
      text_pointer_selection_drag_.reset();
      return false;
    }

    const std::optional<std::size_t> offset =
        text_offset_for_point(*input, moved->position);
    if (offset.has_value()) {
      input->model()->set_selection(drag.anchor_offset, *offset);
    }
    return true;
  }

  return false;
}

std::optional<std::size_t> WindowRuntime::text_offset_for_point(
    const TextElement& element,
    Point point) const {
  const std::optional<Rect> bounds = element.layout_bounds();
  if (!bounds.has_value()) {
    return {};
  }

  const TextMeasurement measurement =
      measure_text(element.text(), element.font(), element.font_size(), scale_);
  return hit_test_text_position(measurement, *bounds, point).byte_offset;
}

TextInputElement* WindowRuntime::routed_text_input(ElementId element_id) {
  return dynamic_cast<TextInputElement*>(routed_element(element_id));
}

const TextInputElement* WindowRuntime::routed_text_input(
    ElementId element_id) const {
  return dynamic_cast<const TextInputElement*>(routed_element(element_id));
}


void WindowRuntime::dispatch_view_event_for_record(
    WindowRuntimeRecord& record,
    View& view,
    const PlatformEvent& event) {
  if (!record.active || record.window == nullptr || record.renderer == nullptr) {
    return;
  }

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

  current_event_route_ = EventRouter::route_to_root(event, record.root_view_id);
  refresh_route_ancestry(*current_event_route_);
  dispatching_view_event_ = true;
  EventResult result = view.handle_event(event, context_for_record(record));
  dispatching_view_event_ = false;
  last_event_result_ = result;
  last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++event_dispatch_sequence_,
      .view_id = current_event_route_->target_view_id,
      .event_kind = current_event_route_->event_kind,
      .route = *current_event_route_,
      .result = last_event_result_};
  if (after_event_callback_) {
    after_event_callback_(context_for_record(record), *last_event_dispatch_);
  }
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

void WindowRuntime::record_lifecycle_event_for_record(
    WindowRuntimeRecord& record,
    const PlatformEvent& event) {
  if (record.window == nullptr || record.renderer == nullptr) {
    return;
  }

  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::window_lifecycle,
      .event_kind = event_kind_for(event),
      .backend = "runtime",
      .operation = "window-lifecycle",
      .supported = true,
      .succeeded = true,
      .value_count = 1,
  });
  current_event_route_ = EventRouter::route_to_root(event, record.root_view_id);
  last_event_result_ = EventResult::unhandled();
  last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++event_dispatch_sequence_,
      .view_id = current_event_route_->target_view_id,
      .event_kind = current_event_route_->event_kind,
      .route = *current_event_route_,
      .result = last_event_result_};
  if (after_event_callback_) {
    after_event_callback_(context_for_record(record), *last_event_dispatch_);
  }
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

void WindowRuntime::handle_native_additional_window_event(
    WindowRuntimeId runtime_id,
    const PlatformEvent& event) {
  WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
  if (record == nullptr) {
    return;
  }
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    record_lifecycle_event_for_record(*record, event);
    cleanup_closed_additional_window(*record);
    return;
  }
  View* view = find_view(record->root_view_id);
  if (view == nullptr) {
    return;
  }
  if (const auto* resized = std::get_if<WindowResized>(&event);
      resized != nullptr) {
    record->descriptor.size = to_logical_pixels(resized->size, resized->scale);
    if (record->renderer != nullptr) {
      (void)record->renderer->resize(resized->size, resized->scale);
    }
    return;
  }
  if (std::holds_alternative<WindowRedrawRequested>(event)) {
    handle_redraw_for_record(*record, *view);
    return;
  }
  if (std::holds_alternative<WindowActivated>(event) ||
      std::holds_alternative<WindowMinimized>(event) ||
      std::holds_alternative<WindowRestored>(event)) {
    if (const auto* activated = std::get_if<WindowActivated>(&event);
        activated != nullptr) {
      input_.focused = activated->active;
    } else if (const auto* minimized = std::get_if<WindowMinimized>(&event);
               minimized != nullptr) {
      input_.focused = !minimized->minimized;
    } else {
      input_.focused = false;
    }
    record_lifecycle_event_for_record(*record, event);
    return;
  }
  dispatch_view_event_for_record(*record, *view, event);
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
      .window_runtime_id = root_window_runtime_id_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
      .scale = scale_,
      .input = input,
      .event_route = current_event_route_,
      .last_event_result = last_event_result_,
      .last_event_dispatch = last_event_dispatch_,
      .frame_index = frame_index_};
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

void WindowRuntime::register_command_palette_entry(CommandPaletteEntry entry) {
  if (!entry.action_name.empty() && !entry.title.empty()) {
    command_palette_entries_.push_back(std::move(entry));
  }
}

std::span<const CommandPaletteEntry> WindowRuntime::command_palette_entries()
    const {
  return command_palette_entries_;
}

std::vector<CommandPaletteEntry>
WindowRuntime::command_palette_entries_for_group(std::string_view group) const {
  std::vector<CommandPaletteEntry> entries;
  for (const CommandPaletteEntry& entry : command_palette_entries_) {
    if (entry.group == group) {
      entries.push_back(entry);
    }
  }
  return entries;
}

ActionDispatchResult WindowRuntime::dispatch_command_palette_entry(
    const CommandPaletteEntry& entry) {
  if (!entry.enabled || entry.action_name.empty()) {
    ActionDispatchResult dispatch{
        .name = entry.action_name,
        .result = EventResult::unhandled()};
    last_action_dispatch_ = dispatch;
    return dispatch;
  }

  return dispatch_action(entry.action_name);
}

ActionDispatchResult WindowRuntime::dispatch_command_palette_action(
    std::string action_name) {
  for (const CommandPaletteEntry& entry : command_palette_entries_) {
    if (entry.action_name == action_name) {
      return dispatch_command_palette_entry(entry);
    }
  }

  ActionDispatchResult dispatch{
      .name = std::move(action_name),
      .result = EventResult::unhandled()};
  last_action_dispatch_ = dispatch;
  return dispatch;
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

AccessibilityTreeSnapshot WindowRuntime::accessibility_snapshot() const {
  if (owned_element_tree_ == nullptr) {
    return {};
  }
  return owned_element_tree_->accessibility_snapshot(
      AccessibilitySnapshotOptions{
          .focused_element_id = keyboard_focus_element_owner_,
      });
}

void WindowRuntime::set_clipboard(Clipboard* clipboard) {
  clipboard_ = clipboard;
}

bool WindowRuntime::paste_clipboard_text() {
  if (clipboard_ == nullptr) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::clipboard,
        .backend = "runtime",
        .operation = "paste-text",
        .supported = false,
        .succeeded = false,
    });
    return false;
  }

  const auto text = clipboard_->read_text();
  if (!text.has_value()) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::clipboard,
        .backend = "runtime",
        .operation = "paste-text",
        .supported = true,
        .succeeded = false,
    });
    return false;
  }

  TextModel* model = focused_text_model();
  if (model == nullptr) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::clipboard,
        .backend = "runtime",
        .operation = "paste-text",
        .supported = true,
        .succeeded = false,
        .value_count = text->size(),
    });
    return false;
  }

  model->insert_text(*text);
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::clipboard,
      .backend = "runtime",
      .operation = "paste-text",
      .supported = true,
      .succeeded = true,
      .value_count = text->size(),
  });
  return true;
}

bool WindowRuntime::copy_selection_to_clipboard() {
  if (clipboard_ == nullptr) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::clipboard,
        .backend = "runtime",
        .operation = "copy-selection",
        .supported = false,
        .succeeded = false,
    });
    return false;
  }

  TextModel* model = focused_text_model();
  if (model == nullptr) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::clipboard,
        .backend = "runtime",
        .operation = "copy-selection",
        .supported = true,
        .succeeded = false,
    });
    return false;
  }

  const std::string selected_text = model->selected_text();
  if (selected_text.empty()) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::clipboard,
        .backend = "runtime",
        .operation = "copy-selection",
        .supported = true,
        .succeeded = false,
    });
    return false;
  }
  const bool succeeded = clipboard_->write_text(selected_text);
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::clipboard,
      .backend = "runtime",
      .operation = "copy-selection",
      .supported = true,
      .succeeded = succeeded,
      .value_count = selected_text.size(),
  });
  return succeeded;
}

bool WindowRuntime::cut_selection_to_clipboard() {
  if (!copy_selection_to_clipboard()) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::clipboard,
        .backend = "runtime",
        .operation = "cut-selection",
        .supported = true,
        .succeeded = false,
    });
    return false;
  }

  TextModel* model = focused_text_model();
  if (model == nullptr) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::clipboard,
        .backend = "runtime",
        .operation = "cut-selection",
        .supported = true,
        .succeeded = false,
    });
    return false;
  }

  const bool succeeded = model->delete_forward();
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::clipboard,
      .backend = "runtime",
      .operation = "cut-selection",
      .supported = true,
      .succeeded = succeeded,
      .value_count = succeeded ? 1U : 0U,
  });
  return succeeded;
}

void WindowRuntime::set_element_cursor(
    ElementId element_id,
    CursorShape cursor_shape) {
  if (element_id.value == 0) {
    return;
  }
  element_cursors_[element_id.value] = cursor_shape;
}


} // namespace cgpui
