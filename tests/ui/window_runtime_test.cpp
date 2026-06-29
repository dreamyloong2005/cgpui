#include "cgpui/platform/platform.hpp"
#include "cgpui/platform/clipboard.hpp"
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/text.hpp"
#include "cgpui/ui/ui.hpp"

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace {

struct RuntimeEntity {
  int value = 0;
};

bool equal(cgpui::Size lhs, cgpui::Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

bool equal(cgpui::DpiScale lhs, cgpui::DpiScale rhs) {
  return lhs.value == rhs.value;
}

bool equal(cgpui::Point lhs, cgpui::Point rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

class RecordingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color color) override {
    clear_count += 1;
    last_clear = color;
  }

  void draw_rect(const cgpui::SolidRect& rect) override {
    draw_count += 1;
    last_rect = rect;
  }

  cgpui::Result<void> present() override {
    present_count += 1;
    return {};
  }

  int clear_count = 0;
  int draw_count = 0;
  int present_count = 0;
  cgpui::Color last_clear{};
  cgpui::SolidRect last_rect{};
};

class RecordingRenderer final : public cgpui::Renderer {
 public:
  explicit RecordingRenderer(RecordingFrame& frame) : frame_(frame) {}

  cgpui::Result<void> resize(cgpui::Size size, cgpui::DpiScale scale) override {
    resize_count += 1;
    last_resize_size = size;
    last_resize_scale = scale;
    return {};
  }

  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    begin_frame_count += 1;
    if (fail_begin_frame) {
      return std::unexpected(cgpui::Error{
          .code = cgpui::ErrorCode::frame_acquisition_failed,
          .message = "test frame failure"});
    }
    return std::unique_ptr<cgpui::RenderFrame>(
        new BorrowedFrame(frame_));
  }

  int resize_count = 0;
  int begin_frame_count = 0;
  bool fail_begin_frame = false;
  cgpui::Size last_resize_size{};
  cgpui::DpiScale last_resize_scale{};

 private:
  class BorrowedFrame final : public cgpui::RenderFrame {
   public:
    explicit BorrowedFrame(RecordingFrame& frame) : frame_(frame) {}

    void clear(cgpui::Color color) override { frame_.clear(color); }
    void draw_rect(const cgpui::SolidRect& rect) override {
      frame_.draw_rect(rect);
    }
    cgpui::Result<void> present() override { return frame_.present(); }

   private:
    RecordingFrame& frame_;
  };

  RecordingFrame& frame_;
};

class RecordingView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport_size) override {
    paint_count += 1;
    last_viewport_size = viewport_size;
    paint_list.fill_rect(
        cgpui::Rect{
            .origin = {1.0F, 2.0F},
            .size = {3.0F, 4.0F}},
        cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 1.0F});
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    event_count += 1;
    last_event_result_consumed = context.last_event_result.consumed;
    last_event_result_cancelled = context.last_event_result.cancelled;
    saw_event_route = context.event_route.has_value();
    if (context.event_route) {
      last_event_route = *context.event_route;
      last_route_element_id = context.event_route->target_element_id;
    }
    saw_last_event_dispatch = context.last_event_dispatch.has_value();
    if (context.last_event_dispatch) {
      last_event_dispatch = *context.last_event_dispatch;
    }
    if (!saw_first_view_id) {
      saw_first_view_id = true;
      first_view_id = context.view_id;
    } else if (context.view_id != first_view_id) {
      view_id_stayed_stable = false;
    }
    last_view_id = context.view_id;
    last_event_viewport_size = context.viewport_size;
    last_event_frame_index = context.frame_index;
    last_input_focused = context.input.focused;
    last_input_pointer_position = context.input.pointer_position;
    last_hovered_element_id = context.input.hovered_element_id;
    last_cursor_shape = context.input.cursor_shape;
    last_pointer_captured = context.input.pointer_captured;
    last_pointer_capture_owner_present =
        context.input.pointer_capture_owner.has_value();
    last_pointer_capture_owner_matches_view =
        context.input.pointer_capture_owner ==
        cgpui::PointerCaptureOwner::view(context.view_id);
    last_pointer_capture_owner_matches_captured_element =
        context.input.pointer_capture_owner ==
        cgpui::PointerCaptureOwner::element(captured_pointer_element_id);
    last_keyboard_focused = context.input.keyboard_focused;
    last_keyboard_focus_owner_present =
        context.input.keyboard_focus_owner.has_value();
    last_keyboard_focus_owner_matches_view =
        context.input.keyboard_focus_owner.has_value() &&
        *context.input.keyboard_focus_owner == context.view_id;
    last_keyboard_focus_element_owner_present =
        context.input.keyboard_focus_element_owner.has_value();
    last_keyboard_focus_element_owner =
        context.input.keyboard_focus_element_owner;

    if (std::holds_alternative<cgpui::WindowFocused>(event)) {
      focus_count += 1;
      focus_event_saw_focused = context.input.focused;
    } else if (std::holds_alternative<cgpui::PointerMoved>(event)) {
      pointer_move_count += 1;
      pointer_move_event_position = context.input.pointer_position;
      if (capture_on_first_pointer_move && pointer_move_count == 1) {
        context.runtime.capture_pointer(
            cgpui::PointerCaptureOwner::view(context.view_id));
      }
      if (capture_pointer_owner_on_first_pointer_move &&
          pointer_move_count == 1) {
        context.runtime.capture_pointer(
            cgpui::PointerCaptureOwner::view(context.view_id));
      }
      if (release_pointer_with_wrong_owner_on_second_pointer_move &&
          pointer_move_count == 2) {
        context.runtime.release_pointer(
            cgpui::PointerCaptureOwner::view(
                cgpui::ViewId{context.view_id.value + 1}));
      }
      if (release_on_third_pointer_move && pointer_move_count == 3) {
        context.runtime.release_pointer(
            cgpui::PointerCaptureOwner::view(context.view_id));
      }
      if (release_pointer_owner_on_third_pointer_move &&
          pointer_move_count == 3) {
        context.runtime.release_pointer(
            cgpui::PointerCaptureOwner::view(context.view_id));
      }
      if (capture_route_element_on_first_pointer_move &&
          pointer_move_count == 1 && context.event_route &&
          context.event_route->target_element_id.has_value()) {
        captured_pointer_element_id = *context.event_route->target_element_id;
        context.runtime.capture_pointer(
            cgpui::PointerCaptureOwner::element(captured_pointer_element_id));
      }
      if (release_pointer_element_with_wrong_owner_on_second_pointer_move &&
          pointer_move_count == 2) {
        context.runtime.release_pointer(
            cgpui::PointerCaptureOwner::view(
                cgpui::ViewId{captured_pointer_element_id.value}));
      }
      if (release_pointer_element_owner_on_third_pointer_move &&
          pointer_move_count == 3) {
        context.runtime.release_pointer(
            cgpui::PointerCaptureOwner::element(captured_pointer_element_id));
      }
      if (pointer_move_count == 2) {
        second_pointer_move_hovered_element_id =
            context.input.hovered_element_id;
        second_pointer_move_cursor_shape = context.input.cursor_shape;
        second_pointer_move_saw_capture = context.input.pointer_captured;
        second_pointer_move_saw_capture_owner =
            last_pointer_capture_owner_matches_view;
        second_pointer_move_saw_element_capture_owner =
            context.input.pointer_capture_owner ==
            cgpui::PointerCaptureOwner::element(captured_pointer_element_id);
      } else if (pointer_move_count == 3) {
        third_pointer_move_hovered_element_id =
            context.input.hovered_element_id;
        third_pointer_move_cursor_shape = context.input.cursor_shape;
        third_pointer_move_saw_capture = context.input.pointer_captured;
        third_pointer_move_saw_capture_owner =
            last_pointer_capture_owner_matches_view;
        third_pointer_move_saw_element_capture_owner =
            context.input.pointer_capture_owner ==
            cgpui::PointerCaptureOwner::element(captured_pointer_element_id);
      } else if (pointer_move_count == 4) {
        fourth_pointer_move_hovered_element_id =
            context.input.hovered_element_id;
        fourth_pointer_move_saw_capture = context.input.pointer_captured;
        fourth_pointer_move_saw_capture_owner =
            last_pointer_capture_owner_matches_view;
        fourth_pointer_move_saw_element_capture_owner =
            context.input.pointer_capture_owner ==
            cgpui::PointerCaptureOwner::element(captured_pointer_element_id);
      }
    } else if (std::holds_alternative<cgpui::PointerButton>(event)) {
      pointer_button_count += 1;
      pointer_button_event_position = context.input.pointer_position;
    } else if (std::holds_alternative<cgpui::PointerScrolled>(event)) {
      pointer_scroll_count += 1;
      pointer_scroll_event_position = context.input.pointer_position;
    } else if (std::holds_alternative<cgpui::KeyboardKey>(event)) {
      keyboard_key_count += 1;
      if (exercise_entity_context_access && keyboard_key_count == 1) {
        entity_id = context.insert_entity(RuntimeEntity{.value = 10});
        inserted_entity_id = entity_id;
        const RuntimeEntity* inserted = context.read_entity(entity_id);
        first_entity_read_value = inserted == nullptr ? -1 : inserted->value;
        if (RuntimeEntity* mutated = context.mutate_entity(entity_id);
            mutated != nullptr) {
          mutated->value = 21;
        }
      }
      if (exercise_entity_context_access && keyboard_key_count == 2) {
        const RuntimeEntity* entity = context.read_entity(entity_id);
        second_entity_read_value = entity == nullptr ? -1 : entity->value;
        removed_entity = context.remove_entity(entity_id);
        removed_entity_again = context.remove_entity(entity_id);
        missing_entity_after_remove = context.read_entity(entity_id) == nullptr;
        emplaced_entity_id = context.emplace_entity<RuntimeEntity>(42);
        const RuntimeEntity* emplaced = context.read_entity(emplaced_entity_id);
        emplaced_entity_read_value =
            emplaced == nullptr ? -1 : emplaced->value;
      }
      if (exercise_view_model_subscriptions && keyboard_key_count == 1) {
        entity_id = context.insert_entity(RuntimeEntity{.value = 30});
        context.subscribe_view_to_entity(context.view_id, entity_id);
        subscriptions_after_subscribe =
            context.runtime.subscriptions_for_view(context.view_id).size();
        first_subscription_matches_entity =
            subscriptions_after_subscribe == 1 &&
            context.runtime.subscriptions_for_view(context.view_id)[0]
                    .entity_id_value == entity_id.value;
        notified_subscribed_entity =
            context.runtime.notify_entity_changed(entity_id);
        invalidation_after_subscribed_notify =
            context.runtime.invalidation_state();
        notified_missing_entity =
            context.runtime.notify_entity_changed(
                cgpui::EntityId<RuntimeEntity>{entity_id.value + 100});
      }
      if (exercise_view_identity_allocation && keyboard_key_count == 1) {
        root_view_id_was_allocated =
            context.is_view_id_allocated(context.view_id);
        zero_view_id_was_allocated =
            context.is_view_id_allocated(cgpui::ViewId{});
        first_allocated_view_id = context.allocate_view_id();
        second_allocated_view_id = context.allocate_view_id();
        first_allocated_view_id_was_allocated =
            context.is_view_id_allocated(first_allocated_view_id);
        second_allocated_view_id_was_allocated =
            context.is_view_id_allocated(second_allocated_view_id);
        next_unallocated_view_id_was_missing =
            !context.is_view_id_allocated(
                cgpui::ViewId{second_allocated_view_id.value + 1});
      }
      if (exercise_view_identity_allocation && keyboard_key_count == 2) {
        first_allocated_view_id_stayed_allocated =
            context.is_view_id_allocated(first_allocated_view_id);
        second_allocated_view_id_stayed_allocated =
            context.is_view_id_allocated(second_allocated_view_id);
        third_allocated_view_id = context.allocate_view_id();
        third_allocated_view_id_was_allocated =
            context.is_view_id_allocated(third_allocated_view_id);
      }
      if (exercise_action_dispatch && keyboard_key_count == 1) {
        context.runtime.register_action(
            "app.save",
            [this](const cgpui::WindowRuntimeContext& action_context) {
              dispatched_action_count += 1;
              action_saw_context_view_id = action_context.view_id;
              return cgpui::EventResult::consumed_event();
            });
        first_action_result = context.runtime.dispatch_action("app.save");
        second_action_result = context.runtime.dispatch_action("app.missing");
        last_action_result_from_context =
            context.runtime.last_action_dispatch();
      }
      if (exercise_invalidation_requests && keyboard_key_count == 1) {
        initial_invalidation = context.runtime.invalidation_state();
        context.runtime.request_layout();
        after_layout_request_invalidation =
            context.runtime.invalidation_state();
        context.runtime.request_paint();
        after_paint_request_invalidation =
            context.runtime.invalidation_state();
        context.runtime.clear_invalidation();
        after_clear_invalidation = context.runtime.invalidation_state();
        context.runtime.request_paint();
      }
      if (exercise_scheduled_invalidation_redraw &&
          keyboard_key_count == 1) {
        context.runtime.request_layout();
        context.runtime.request_paint();
      }
      if (request_keyboard_focus_on_first_key && keyboard_key_count == 1) {
        context.runtime.request_keyboard_focus();
      }
      if (request_keyboard_focus_owner_on_first_key &&
          keyboard_key_count == 1) {
        context.runtime.request_keyboard_focus(context.view_id);
      }
      if (request_keyboard_focus_element_on_first_key &&
          keyboard_key_count == 1) {
        context.runtime.request_keyboard_focus(focused_keyboard_element_id);
      }
      if (release_keyboard_focus_with_wrong_owner_on_second_key &&
          keyboard_key_count == 2) {
        context.runtime.release_keyboard_focus(
            cgpui::ViewId{context.view_id.value + 1});
      }
      if (release_keyboard_focus_element_with_wrong_owner_on_second_key &&
          keyboard_key_count == 2) {
        context.runtime.release_keyboard_focus(
            cgpui::ElementId{focused_keyboard_element_id.value + 1});
      }
      if (release_keyboard_focus_on_third_key && keyboard_key_count == 3) {
        context.runtime.release_keyboard_focus();
      }
      if (release_keyboard_focus_owner_on_third_key &&
          keyboard_key_count == 3) {
        context.runtime.release_keyboard_focus(context.view_id);
      }
      if (release_keyboard_focus_element_owner_on_third_key &&
          keyboard_key_count == 3) {
        context.runtime.release_keyboard_focus(focused_keyboard_element_id);
      }
      if (keyboard_key_count == 2) {
        second_key_saw_keyboard_focus = context.input.keyboard_focused;
        second_key_saw_keyboard_focus_owner =
            last_keyboard_focus_owner_matches_view;
        second_key_saw_keyboard_focus_element_owner =
            context.input.keyboard_focus_element_owner ==
            focused_keyboard_element_id;
      } else if (keyboard_key_count == 3) {
        third_key_saw_keyboard_focus = context.input.keyboard_focused;
        third_key_saw_keyboard_focus_owner =
            last_keyboard_focus_owner_matches_view;
        third_key_saw_keyboard_focus_element_owner =
            context.input.keyboard_focus_element_owner ==
            focused_keyboard_element_id;
      } else if (keyboard_key_count == 4) {
        fourth_key_saw_keyboard_focus = context.input.keyboard_focused;
        fourth_key_saw_keyboard_focus_owner =
            last_keyboard_focus_owner_matches_view;
        fourth_key_saw_keyboard_focus_element_owner =
            context.input.keyboard_focus_element_owner ==
            focused_keyboard_element_id;
      }
    } else if (std::holds_alternative<cgpui::TextInput>(event)) {
      text_input_count += 1;
      text_input_saw_keyboard_focus = context.input.keyboard_focused;
      text_input_saw_keyboard_focus_owner =
          last_keyboard_focus_owner_matches_view;
      text_input_saw_keyboard_focus_element_owner =
          context.input.keyboard_focus_element_owner ==
          focused_keyboard_element_id;
    } else if (std::holds_alternative<cgpui::ImeComposition>(event)) {
      ime_composition_count += 1;
      ime_composition_saw_keyboard_focus = context.input.keyboard_focused;
      ime_composition_saw_keyboard_focus_element_owner =
          context.input.keyboard_focus_element_owner ==
          focused_keyboard_element_id;
    }

    if (request_redraw_on_event && event_redraw_requests == 0) {
      event_redraw_requests += 1;
      context.window.request_redraw();
    }

    if (consume_next_event) {
      consume_next_event = false;
      return cgpui::EventResult::consumed_event();
    }
    if (cancel_next_event) {
      cancel_next_event = false;
      return cgpui::EventResult::cancelled_event();
    }
    return cgpui::EventResult::unhandled();
  }

  int paint_count = 0;
  int event_count = 0;
  int focus_count = 0;
  int pointer_move_count = 0;
  int pointer_button_count = 0;
  int pointer_scroll_count = 0;
  int keyboard_key_count = 0;
  int text_input_count = 0;
  int ime_composition_count = 0;
  int event_redraw_requests = 0;
  bool request_redraw_on_event = false;
  bool consume_next_event = false;
  bool cancel_next_event = false;
  bool exercise_entity_context_access = false;
  bool exercise_view_model_subscriptions = false;
  bool exercise_view_identity_allocation = false;
  bool exercise_action_dispatch = false;
  bool exercise_invalidation_requests = false;
  bool exercise_scheduled_invalidation_redraw = false;
  bool capture_on_first_pointer_move = false;
  bool release_on_third_pointer_move = false;
  bool capture_pointer_owner_on_first_pointer_move = false;
  bool release_pointer_with_wrong_owner_on_second_pointer_move = false;
  bool release_pointer_owner_on_third_pointer_move = false;
  bool capture_route_element_on_first_pointer_move = false;
  bool release_pointer_element_with_wrong_owner_on_second_pointer_move = false;
  bool release_pointer_element_owner_on_third_pointer_move = false;
  bool request_keyboard_focus_on_first_key = false;
  bool release_keyboard_focus_on_third_key = false;
  bool request_keyboard_focus_owner_on_first_key = false;
  bool release_keyboard_focus_with_wrong_owner_on_second_key = false;
  bool release_keyboard_focus_owner_on_third_key = false;
  bool request_keyboard_focus_element_on_first_key = false;
  bool release_keyboard_focus_element_with_wrong_owner_on_second_key = false;
  bool release_keyboard_focus_element_owner_on_third_key = false;
  bool last_input_focused = false;
  bool focus_event_saw_focused = false;
  bool last_pointer_captured = false;
  bool last_pointer_capture_owner_present = false;
  bool last_pointer_capture_owner_matches_view = false;
  bool last_pointer_capture_owner_matches_captured_element = false;
  bool second_pointer_move_saw_capture = false;
  bool second_pointer_move_saw_capture_owner = false;
  bool second_pointer_move_saw_element_capture_owner = false;
  bool third_pointer_move_saw_capture = false;
  bool third_pointer_move_saw_capture_owner = false;
  bool third_pointer_move_saw_element_capture_owner = false;
  bool fourth_pointer_move_saw_capture = true;
  bool fourth_pointer_move_saw_capture_owner = true;
  bool fourth_pointer_move_saw_element_capture_owner = true;
  bool last_keyboard_focused = false;
  bool last_keyboard_focus_owner_present = false;
  bool last_keyboard_focus_owner_matches_view = false;
  bool last_keyboard_focus_element_owner_present = false;
  bool second_key_saw_keyboard_focus = false;
  bool second_key_saw_keyboard_focus_owner = false;
  bool second_key_saw_keyboard_focus_element_owner = false;
  bool third_key_saw_keyboard_focus = false;
  bool third_key_saw_keyboard_focus_owner = false;
  bool third_key_saw_keyboard_focus_element_owner = false;
  bool fourth_key_saw_keyboard_focus = true;
  bool fourth_key_saw_keyboard_focus_owner = true;
  bool fourth_key_saw_keyboard_focus_element_owner = true;
  bool text_input_saw_keyboard_focus = false;
  bool text_input_saw_keyboard_focus_owner = false;
  bool text_input_saw_keyboard_focus_element_owner = false;
  bool ime_composition_saw_keyboard_focus = false;
  bool ime_composition_saw_keyboard_focus_element_owner = false;
  bool last_event_result_consumed = false;
  bool last_event_result_cancelled = false;
  bool saw_event_route = false;
  bool saw_last_event_dispatch = false;
  bool saw_first_view_id = false;
  bool view_id_stayed_stable = true;
  bool removed_entity = false;
  bool removed_entity_again = true;
  bool missing_entity_after_remove = false;
  bool first_subscription_matches_entity = false;
  bool notified_subscribed_entity = false;
  bool notified_missing_entity = true;
  int first_entity_read_value = -1;
  int second_entity_read_value = -1;
  int emplaced_entity_read_value = -1;
  std::size_t subscriptions_after_subscribe = 0;
  cgpui::EntityId<RuntimeEntity> entity_id{};
  cgpui::EntityId<RuntimeEntity> inserted_entity_id{};
  cgpui::EntityId<RuntimeEntity> emplaced_entity_id{};
  bool root_view_id_was_allocated = false;
  bool zero_view_id_was_allocated = true;
  bool first_allocated_view_id_was_allocated = false;
  bool second_allocated_view_id_was_allocated = false;
  bool first_allocated_view_id_stayed_allocated = false;
  bool second_allocated_view_id_stayed_allocated = false;
  bool third_allocated_view_id_was_allocated = false;
  bool next_unallocated_view_id_was_missing = false;
  int dispatched_action_count = 0;
  cgpui::ViewId action_saw_context_view_id{};
  cgpui::ActionDispatchResult first_action_result{};
  cgpui::ActionDispatchResult second_action_result{};
  std::optional<cgpui::ActionDispatchResult> last_action_result_from_context;
  cgpui::InvalidationState initial_invalidation{};
  cgpui::InvalidationState after_layout_request_invalidation{};
  cgpui::InvalidationState after_paint_request_invalidation{};
  cgpui::InvalidationState after_clear_invalidation{};
  cgpui::InvalidationState invalidation_after_subscribed_notify{};
  cgpui::ViewId first_allocated_view_id{};
  cgpui::ViewId second_allocated_view_id{};
  cgpui::ViewId third_allocated_view_id{};
  cgpui::ElementId captured_pointer_element_id{};
  cgpui::ElementId focused_keyboard_element_id{21};
  cgpui::EventRoute last_event_route{};
  cgpui::EventDispatchRecord last_event_dispatch{};
  std::optional<cgpui::ElementId> last_route_element_id;
  std::optional<cgpui::ElementId> last_hovered_element_id;
  std::optional<cgpui::ElementId> second_pointer_move_hovered_element_id;
  std::optional<cgpui::ElementId> third_pointer_move_hovered_element_id;
  std::optional<cgpui::ElementId> fourth_pointer_move_hovered_element_id;
  std::optional<cgpui::ElementId> last_keyboard_focus_element_owner;
  cgpui::CursorShape last_cursor_shape = cgpui::CursorShape::default_arrow;
  cgpui::CursorShape second_pointer_move_cursor_shape =
      cgpui::CursorShape::default_arrow;
  cgpui::CursorShape third_pointer_move_cursor_shape =
      cgpui::CursorShape::default_arrow;
  cgpui::ViewId first_view_id{};
  cgpui::ViewId last_view_id{};
  cgpui::Size last_viewport_size{};
  cgpui::Size last_event_viewport_size{};
  cgpui::Point last_input_pointer_position{};
  cgpui::Point pointer_move_event_position{};
  cgpui::Point pointer_button_event_position{};
  cgpui::Point pointer_scroll_event_position{};
  int last_event_frame_index = -1;
};

class RuntimeEventElement final : public cgpui::FixedSizeElement {
 public:
  explicit RuntimeEventElement(cgpui::Size preferred_size)
      : cgpui::FixedSizeElement(preferred_size) {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::ElementEventContext& context) override {
    event_count += 1;
    last_target_element_id = context.target_element_id;
    saw_pointer_event =
        saw_pointer_event || std::holds_alternative<cgpui::PointerMoved>(event);
    return result;
  }

  int event_count = 0;
  bool saw_pointer_event = false;
  cgpui::ElementId last_target_element_id;
  cgpui::EventResult result = cgpui::EventResult::unhandled();
};

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  explicit FakeWindow(cgpui::WindowState state) : state_(state) {}

  cgpui::NativeSurfaceHandle native_surface() const override {
    return cgpui::Win32SurfaceHandle{};
  }

  cgpui::WindowState state() const override { return state_; }

  void request_redraw() override {
    request_redraw_count += 1;
    if (callback) {
      callback(cgpui::WindowRedrawRequested{});
    }
  }

  void request_close() override {
    request_close_count += 1;
    if (callback) {
      callback(cgpui::WindowCloseRequested{});
    }
  }

  void set_title(std::string_view title) override { last_title = title; }

  void dispatch_resize(cgpui::Size size, cgpui::DpiScale scale) {
    state_.framebuffer_size = size;
    state_.scale = scale;
    if (callback) {
      callback(cgpui::WindowResized{.size = size, .scale = scale});
    }
  }

  cgpui::PlatformEventCallback callback;
  int request_redraw_count = 0;
  int request_close_count = 0;
  std::string_view last_title;

 private:
  cgpui::WindowState state_;
};

class FakeApplication final : public cgpui::PlatformApplication {
 public:
  explicit FakeApplication(FakeWindow& window) : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    last_descriptor = descriptor;
    window_.callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(
        new BorrowedWindow(window_));
  }

  int run() override {
    run_count += 1;
    if (on_run) {
      on_run();
    }
    return run_result;
  }

  void quit() override { quit_count += 1; }

  FakeWindow& window_;
  int create_window_count = 0;
  int run_count = 0;
  int quit_count = 0;
  int run_result = 0;
  cgpui::WindowDescriptor last_descriptor{};
  void (*on_run)() = nullptr;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(FakeWindow& window) : window_(window) {}

    cgpui::NativeSurfaceHandle native_surface() const override {
      return window_.native_surface();
    }
    cgpui::WindowState state() const override { return window_.state(); }
    void request_redraw() override { window_.request_redraw(); }
    void request_close() override { window_.request_close(); }
    void set_title(std::string_view title) override {
      window_.set_title(title);
    }

   private:
    FakeWindow& window_;
  };
};

struct RuntimeFixture {
  FakeWindow window{cgpui::WindowState{
      .framebuffer_size = {640.0F, 480.0F},
      .scale = cgpui::DpiScale{1.0F},
      .close_requested = false}};
  FakeApplication app{window};
  RecordingFrame frame;
  RecordingRenderer renderer{frame};
  RecordingView view;
};

int test_redraw_paints_initial_viewport() {
  RuntimeFixture fixture;
  fixture.app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor) {
        if (!equal(descriptor.framebuffer_size, cgpui::Size{640.0F, 480.0F})) {
          return cgpui::Result<cgpui::Renderer*>{std::unexpected(cgpui::Error{
              .code = cgpui::ErrorCode::renderer_initialization_failed,
              .message = "wrong descriptor size"})};
        }
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .title = "Runtime Test",
      .size = {640.0F, 480.0F}});

  if (result != 0) {
    return 1;
  }
  if (fixture.app.create_window_count != 1 || fixture.app.run_count != 1) {
    return 2;
  }
  if (fixture.window.request_redraw_count != 1) {
    return 3;
  }
  if (fixture.renderer.begin_frame_count != 1 ||
      fixture.view.paint_count != 1 ||
      fixture.frame.present_count != 1) {
    return 4;
  }
  if (fixture.frame.draw_count != 1) {
    return 6;
  }
  if (!equal(fixture.view.last_viewport_size, cgpui::Size{640.0F, 480.0F})) {
    return 5;
  }

  return 0;
}

RuntimeFixture* resize_fixture = nullptr;

void dispatch_runtime_resize() {
  resize_fixture->window.dispatch_resize(
      cgpui::Size{320.0F, 240.0F},
      cgpui::DpiScale{2.0F});
  resize_fixture->window.request_redraw();
}

int test_resize_updates_renderer_and_viewport() {
  RuntimeFixture fixture;
  resize_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_resize;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .title = "Runtime Test",
      .size = {640.0F, 480.0F}});
  resize_fixture = nullptr;

  if (result != 0) {
    return 10;
  }
  if (fixture.renderer.resize_count != 1) {
    return 11;
  }
  if (!equal(fixture.renderer.last_resize_size, cgpui::Size{320.0F, 240.0F}) ||
      !equal(fixture.renderer.last_resize_scale, cgpui::DpiScale{2.0F})) {
    return 12;
  }
  if (!equal(fixture.view.last_viewport_size, cgpui::Size{320.0F, 240.0F})) {
    return 13;
  }
  if (fixture.renderer.begin_frame_count != 2 || fixture.view.paint_count != 2) {
    return 14;
  }

  return 0;
}

RuntimeFixture* close_fixture = nullptr;

void dispatch_runtime_close() {
  close_fixture->window.request_close();
}

int test_close_request_quits_application() {
  RuntimeFixture fixture;
  close_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_close;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  close_fixture = nullptr;

  if (result != 0) {
    return 20;
  }
  if (fixture.app.quit_count != 1) {
    return 21;
  }

  return 0;
}

RuntimeFixture* render_failure_fixture = nullptr;

void dispatch_render_failure() {
  render_failure_fixture->renderer.fail_begin_frame = true;
  render_failure_fixture->window.request_redraw();
}

int test_render_failure_quits_and_returns_failure() {
  RuntimeFixture fixture;
  render_failure_fixture = &fixture;
  fixture.app.on_run = &dispatch_render_failure;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  render_failure_fixture = nullptr;

  if (result == 0) {
    return 30;
  }
  if (fixture.app.quit_count != 1) {
    return 31;
  }

  return 0;
}

RuntimeFixture* event_dispatch_fixture = nullptr;

void dispatch_view_events() {
  auto& callback = event_dispatch_fixture->window.callback;
  callback(cgpui::WindowFocused{.focused = true});
  callback(cgpui::PointerMoved{.position = {12.0F, 24.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .position = {12.0F, 24.0F}});
  callback(cgpui::PointerScrolled{
      .delta = {0.0F, -4.0F},
      .position = {12.0F, 24.0F}});
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true}});
  callback(cgpui::TextInput{
      .text = "A",
      .modifiers = {.shift = true}});
}

int test_runtime_dispatches_input_events_to_view() {
  RuntimeFixture fixture;
  event_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_events;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_dispatch_fixture = nullptr;

  if (result != 0) {
    return 40;
  }
  if (fixture.view.event_count != 6) {
    return 41;
  }
  if (fixture.view.focus_count != 1 ||
      fixture.view.pointer_move_count != 1 ||
      fixture.view.pointer_button_count != 1 ||
      fixture.view.pointer_scroll_count != 1 ||
      fixture.view.keyboard_key_count != 1 ||
      fixture.view.text_input_count != 1) {
    return 42;
  }
  if (!equal(
          fixture.view.last_event_viewport_size,
          cgpui::Size{640.0F, 480.0F})) {
    return 43;
  }
  if (fixture.view.last_event_frame_index != 1) {
    return 44;
  }
  if (!fixture.view.focus_event_saw_focused ||
      !fixture.view.last_input_focused) {
    return 45;
  }
  if (!equal(
          fixture.view.pointer_move_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 46;
  }
  if (!equal(
          fixture.view.pointer_button_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 47;
  }
  if (!equal(
          fixture.view.pointer_scroll_event_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 48;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{12.0F, 24.0F})) {
    return 49;
  }

  return 0;
}

RuntimeFixture* event_redraw_fixture = nullptr;

void dispatch_event_driven_redraw() {
  event_redraw_fixture->window.callback(
      cgpui::PointerMoved{.position = {5.0F, 6.0F}});
}

int test_view_event_can_request_redraw() {
  RuntimeFixture fixture;
  event_redraw_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_driven_redraw;
  fixture.view.request_redraw_on_event = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_redraw_fixture = nullptr;

  if (result != 0) {
    return 50;
  }
  if (fixture.view.event_count != 1 ||
      fixture.view.event_redraw_requests != 1) {
    return 51;
  }
  if (fixture.window.request_redraw_count != 2) {
    return 52;
  }
  if (fixture.renderer.begin_frame_count != 2 ||
      fixture.view.paint_count != 2 ||
      fixture.frame.present_count != 2) {
    return 53;
  }

  return 0;
}

RuntimeFixture* event_result_fixture = nullptr;

void dispatch_event_result_sequence() {
  auto& callback = event_result_fixture->window.callback;
  event_result_fixture->view.consume_next_event = true;
  callback(cgpui::PointerMoved{.position = {7.0F, 8.0F}});
  callback(cgpui::PointerMoved{.position = {9.0F, 10.0F}});
  event_result_fixture->view.cancel_next_event = true;
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "a"});
}

int test_runtime_exposes_last_view_event_result() {
  RuntimeFixture fixture;
  event_result_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_result_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_result_fixture = nullptr;

  if (result != 0) {
    return 55;
  }
  if (fixture.view.pointer_move_count != 2 ||
      fixture.view.keyboard_key_count != 1 ||
      fixture.view.text_input_count != 1) {
    return 56;
  }
  if (!fixture.view.last_event_result_consumed ||
      !fixture.view.last_event_result_cancelled) {
    return 57;
  }

  return 0;
}

RuntimeFixture* event_observability_fixture = nullptr;

void dispatch_event_observability_sequence() {
  auto& callback = event_observability_fixture->window.callback;
  event_observability_fixture->view.consume_next_event = true;
  callback(cgpui::PointerMoved{.position = {11.0F, 12.0F}});
  event_observability_fixture->view.cancel_next_event = true;
  callback(cgpui::KeyboardKey{
      .key_code = 66,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "b"});
}

int test_runtime_reports_each_view_event_dispatch() {
  RuntimeFixture fixture;
  event_observability_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_observability_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  int callback_count = 0;
  cgpui::EventDispatchRecord first_record{};
  cgpui::EventDispatchRecord second_record{};
  cgpui::EventDispatchRecord third_record{};
  bool callback_context_saw_current_record = true;
  bool callback_view_id_matched_context = true;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (callback_count == 1) {
          first_record = record;
        } else if (callback_count == 2) {
          second_record = record;
        } else if (callback_count == 3) {
          third_record = record;
        }
        callback_context_saw_current_record =
            callback_context_saw_current_record &&
            context.last_event_dispatch.has_value() &&
            context.last_event_dispatch->sequence == record.sequence &&
            context.last_event_dispatch->event_kind == record.event_kind &&
            context.last_event_dispatch->view_id == record.view_id &&
            context.last_event_dispatch->result.consumed ==
                record.result.consumed &&
            context.last_event_dispatch->result.cancelled ==
                record.result.cancelled;
        callback_view_id_matched_context =
            callback_view_id_matched_context &&
            record.view_id == context.view_id;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_observability_fixture = nullptr;

  if (result != 0) {
    return 58;
  }
  if (callback_count != 3) {
    return 59;
  }
  if (first_record.sequence != 1 ||
      first_record.event_kind != cgpui::EventKind::pointer_moved ||
      !first_record.result.consumed ||
      first_record.result.cancelled) {
    return 100;
  }
  if (second_record.sequence != 2 ||
      second_record.event_kind != cgpui::EventKind::keyboard_key ||
      !second_record.result.consumed ||
      !second_record.result.cancelled) {
    return 101;
  }
  if (third_record.sequence != 3 ||
      third_record.event_kind != cgpui::EventKind::text_input ||
      third_record.result.consumed ||
      third_record.result.cancelled) {
    return 102;
  }
  if (first_record.view_id.value == 0 ||
      first_record.view_id != second_record.view_id ||
      second_record.view_id != third_record.view_id) {
    return 103;
  }
  if (!callback_context_saw_current_record ||
      !callback_view_id_matched_context) {
    return 104;
  }
  if (!fixture.view.saw_last_event_dispatch ||
      fixture.view.last_event_dispatch.sequence != 2 ||
      fixture.view.last_event_dispatch.event_kind !=
          cgpui::EventKind::keyboard_key ||
      !fixture.view.last_event_dispatch.result.cancelled) {
    return 105;
  }

  return 0;
}

int test_event_router_routes_events_to_root_view() {
  const cgpui::ViewId root_view_id{42};

  const auto pointer_route = cgpui::EventRouter::route_to_root(
      cgpui::PointerMoved{.position = {1.0F, 2.0F}},
      root_view_id);
  const auto key_route = cgpui::EventRouter::route_to_root(
      cgpui::KeyboardKey{
          .key_code = 13,
          .action = cgpui::KeyAction::pressed},
      root_view_id);

  if (pointer_route.target_view_id != root_view_id ||
      pointer_route.event_kind != cgpui::EventKind::pointer_moved) {
    return 106;
  }
  if (key_route.target_view_id != root_view_id ||
      key_route.event_kind != cgpui::EventKind::keyboard_key) {
    return 107;
  }

  return 0;
}

RuntimeFixture* event_route_fixture = nullptr;

void dispatch_event_route_sequence() {
  event_route_fixture->window.callback(cgpui::PointerMoved{
      .position = {21.0F, 22.0F}});
}

int test_runtime_exposes_current_event_route() {
  RuntimeFixture fixture;
  event_route_fixture = &fixture;
  fixture.app.on_run = &dispatch_event_route_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::EventDispatchRecord callback_record{};
  cgpui::EventRoute callback_route{};
  bool callback_saw_route = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_record = record;
        callback_saw_route = context.event_route.has_value();
        if (context.event_route) {
          callback_route = *context.event_route;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  event_route_fixture = nullptr;

  if (result != 0) {
    return 108;
  }
  if (!fixture.view.saw_event_route ||
      fixture.view.last_event_route.target_view_id != fixture.view.last_view_id ||
      fixture.view.last_event_route.event_kind !=
          cgpui::EventKind::pointer_moved) {
    return 109;
  }
  if (!callback_saw_route ||
      callback_route.target_view_id != fixture.view.last_view_id ||
      callback_route.event_kind != cgpui::EventKind::pointer_moved) {
    return 110;
  }
  if (callback_record.route.target_view_id != fixture.view.last_view_id ||
      callback_record.route.event_kind != cgpui::EventKind::pointer_moved ||
      callback_record.view_id != callback_record.route.target_view_id ||
      callback_record.event_kind != callback_record.route.event_kind) {
    return 111;
  }

  return 0;
}

RuntimeFixture* pointer_hit_route_fixture = nullptr;

void dispatch_pointer_hit_route_sequence() {
  auto& callback = pointer_hit_route_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerButton{
      .button = cgpui::MouseButton::left,
      .pressed = true,
      .position = {5.0F, 15.0F}});
  callback(cgpui::PointerScrolled{
      .delta = {0.0F, -1.0F},
      .position = {30.0F, 15.0F}});
  callback(cgpui::KeyboardKey{
      .key_code = 70,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_routes_pointer_events_to_hit_element() {
  RuntimeFixture fixture;
  pointer_hit_route_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_hit_route_sequence;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{10});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{11});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 20.0F, .height = 30.0F});
  second->assign_id(cgpui::ElementId{12});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));
  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 40.0F) {
    return 130;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  int callback_count = 0;
  cgpui::EventDispatchRecord move_record{};
  cgpui::EventDispatchRecord button_record{};
  cgpui::EventDispatchRecord scroll_record{};
  cgpui::EventDispatchRecord key_record{};
  bool callback_context_matched_record_route = true;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (callback_count == 1) {
          move_record = record;
        } else if (callback_count == 2) {
          button_record = record;
        } else if (callback_count == 3) {
          scroll_record = record;
        } else if (callback_count == 4) {
          key_record = record;
        }
        callback_context_matched_record_route =
            callback_context_matched_record_route &&
            context.event_route.has_value() &&
            context.event_route->target_element_id ==
                record.route.target_element_id;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_hit_route_fixture = nullptr;

  if (result != 0) {
    return 123;
  }
  if (callback_count != 4 || fixture.view.event_count != 4) {
    return 124;
  }
  if (!move_record.route.target_element_id.has_value() ||
      *move_record.route.target_element_id != cgpui::ElementId{11}) {
    return 125;
  }
  if (!button_record.route.target_element_id.has_value() ||
      *button_record.route.target_element_id != cgpui::ElementId{12}) {
    return 126;
  }
  if (!scroll_record.route.target_element_id.has_value() ||
      *scroll_record.route.target_element_id != cgpui::ElementId{10}) {
    return 127;
  }
  if (key_record.route.target_element_id.has_value() ||
      fixture.view.last_route_element_id.has_value()) {
    return 128;
  }
  if (!callback_context_matched_record_route) {
    return 129;
  }

  return 0;
}

RuntimeFixture* owned_element_tree_fixture = nullptr;

void dispatch_owned_element_tree_sequence() {
  auto& callback = owned_element_tree_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_runtime_owns_installed_element_tree() {
  RuntimeFixture fixture;
  owned_element_tree_fixture = &fixture;
  fixture.app.on_run = &dispatch_owned_element_tree_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 40.0F, .height = 20.0F}));
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  if (tree != nullptr || runtime.element_tree() == nullptr ||
      runtime.element_root() == nullptr) {
    return 200;
  }
  if (runtime.element_tree()->root_id() != root_id ||
      runtime.element_root()->id() != root_id) {
    return 201;
  }

  std::optional<cgpui::ElementId> routed_element_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        routed_element_id = record.route.target_element_id;
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  owned_element_tree_fixture = nullptr;

  if (result != 0) {
    return 202;
  }
  if (!routed_element_id.has_value() || *routed_element_id != root_id) {
    return 203;
  }

  runtime.set_element_tree(nullptr);
  return runtime.element_tree() == nullptr && runtime.element_root() == nullptr
      ? 0
      : 204;
}

RuntimeFixture* runtime_layout_owned_tree_fixture = nullptr;

void dispatch_runtime_layout_owned_tree_sequence() {
  runtime_layout_owned_tree_fixture->window.request_redraw();
  auto& callback = runtime_layout_owned_tree_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_runtime_lays_out_owned_element_tree_on_redraw() {
  RuntimeFixture fixture;
  runtime_layout_owned_tree_fixture = &fixture;
  fixture.app.on_run = &dispatch_runtime_layout_owned_tree_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId root_id =
      tree->set_root(std::make_unique<cgpui::FixedSizeElement>(
          cgpui::Size{.width = 700.0F, .height = 600.0F}));

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));

  std::optional<cgpui::ElementId> routed_element_id;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        routed_element_id = record.route.target_element_id;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  runtime_layout_owned_tree_fixture = nullptr;

  if (result != 0) {
    return 205;
  }
  if (fixture.renderer.begin_frame_count != 1 ||
      runtime.element_root() == nullptr ||
      !runtime.element_root()->layout_bounds().has_value()) {
    return 206;
  }
  const cgpui::Rect bounds = *runtime.element_root()->layout_bounds();
  if (bounds.size.width != 640.0F || bounds.size.height != 480.0F) {
    return 207;
  }
  if (!routed_element_id.has_value() || *routed_element_id != root_id) {
    return 208;
  }

  return 0;
}

RuntimeFixture* element_event_dispatch_fixture = nullptr;

void dispatch_element_event_dispatch_sequence() {
  auto& callback = element_event_dispatch_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
}

int test_runtime_dispatches_consumed_element_event_before_view_fallback() {
  RuntimeFixture fixture;
  element_event_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_element_event_dispatch_sequence;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  RuntimeEventElement* root_ptr = root.get();
  root_ptr->result = cgpui::EventResult::consumed_event();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::EventDispatchRecord dispatch_record{};
  int callback_count = 0;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  element_event_dispatch_fixture = nullptr;

  if (result != 0) {
    return 209;
  }
  if (root_ptr->event_count != 1 || !root_ptr->saw_pointer_event ||
      root_ptr->last_target_element_id != root_id) {
    return 210;
  }
  if (fixture.view.event_count != 0) {
    return 211;
  }
  if (callback_count != 1 || !dispatch_record.result.consumed ||
      dispatch_record.result.cancelled) {
    return 212;
  }
  if (!dispatch_record.route.target_element_id.has_value() ||
      *dispatch_record.route.target_element_id != root_id) {
    return 213;
  }

  return 0;
}

int test_runtime_falls_back_to_view_after_unhandled_element_event() {
  RuntimeFixture fixture;
  element_event_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_element_event_dispatch_sequence;
  fixture.view.consume_next_event = true;

  auto tree = std::make_unique<cgpui::ElementTree>();
  auto root = std::make_unique<RuntimeEventElement>(
      cgpui::Size{.width = 40.0F, .height = 20.0F});
  RuntimeEventElement* root_ptr = root.get();
  const cgpui::ElementId root_id = tree->set_root(std::move(root));
  (void)tree->layout_root(cgpui::LayoutInput{});

  cgpui::EventDispatchRecord dispatch_record{};
  int callback_count = 0;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        dispatch_record = record;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  element_event_dispatch_fixture = nullptr;

  if (result != 0) {
    return 214;
  }
  if (root_ptr->event_count != 1 || fixture.view.event_count != 1) {
    return 215;
  }
  if (fixture.view.last_route_element_id != root_id ||
      !fixture.view.saw_event_route) {
    return 216;
  }
  if (callback_count != 1 || !dispatch_record.result.consumed ||
      dispatch_record.result.cancelled) {
    return 217;
  }
  if (!dispatch_record.route.target_element_id.has_value() ||
      *dispatch_record.route.target_element_id != root_id) {
    return 218;
  }

  return 0;
}

RuntimeFixture* hover_state_fixture = nullptr;

void dispatch_hover_state_sequence() {
  auto& callback = hover_state_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
  callback(cgpui::PointerMoved{.position = {100.0F, 100.0F}});
}

int test_runtime_tracks_hovered_pointer_element() {
  RuntimeFixture fixture;
  hover_state_fixture = &fixture;
  fixture.app.on_run = &dispatch_hover_state_sequence;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{10});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{11});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 20.0F, .height = 30.0F});
  second->assign_id(cgpui::ElementId{12});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));
  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 40.0F) {
    return 165;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  int callback_count = 0;
  std::optional<cgpui::ElementId> first_hover;
  std::optional<cgpui::ElementId> second_hover;
  std::optional<cgpui::ElementId> third_hover;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        callback_count += 1;
        if (callback_count == 1) {
          first_hover = context.input.hovered_element_id;
        } else if (callback_count == 2) {
          second_hover = context.input.hovered_element_id;
        } else if (callback_count == 3) {
          third_hover = context.input.hovered_element_id;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  hover_state_fixture = nullptr;

  if (result != 0) {
    return 166;
  }
  if (callback_count != 3 || fixture.view.pointer_move_count != 3) {
    return 167;
  }
  if (!first_hover.has_value() ||
      *first_hover != cgpui::ElementId{11}) {
    return 168;
  }
  if (!fixture.view.second_pointer_move_hovered_element_id.has_value() ||
      *fixture.view.second_pointer_move_hovered_element_id !=
          cgpui::ElementId{12}) {
    return 169;
  }
  if (!second_hover.has_value() ||
      *second_hover != cgpui::ElementId{12}) {
    return 170;
  }
  if (third_hover.has_value() ||
      fixture.view.last_hovered_element_id.has_value()) {
    return 171;
  }

  return 0;
}

RuntimeFixture* cursor_shape_fixture = nullptr;

void dispatch_cursor_shape_sequence() {
  auto& callback = cursor_shape_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
  callback(cgpui::PointerMoved{.position = {100.0F, 100.0F}});
}

int test_runtime_routes_cursor_shape_from_hovered_element() {
  RuntimeFixture fixture;
  cursor_shape_fixture = &fixture;
  fixture.app.on_run = &dispatch_cursor_shape_sequence;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{10});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{11});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 20.0F, .height = 30.0F});
  second->assign_id(cgpui::ElementId{12});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));
  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 40.0F) {
    return 178;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);
  runtime.set_element_cursor(
      cgpui::ElementId{11},
      cgpui::CursorShape::pointing_hand);
  runtime.set_element_cursor(cgpui::ElementId{12}, cgpui::CursorShape::text);

  int callback_count = 0;
  cgpui::CursorShape first_cursor = cgpui::CursorShape::default_arrow;
  cgpui::CursorShape second_cursor = cgpui::CursorShape::default_arrow;
  cgpui::CursorShape third_cursor = cgpui::CursorShape::pointing_hand;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        callback_count += 1;
        if (callback_count == 1) {
          first_cursor = context.input.cursor_shape;
        } else if (callback_count == 2) {
          second_cursor = context.input.cursor_shape;
        } else if (callback_count == 3) {
          third_cursor = context.input.cursor_shape;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  cursor_shape_fixture = nullptr;

  if (result != 0) {
    return 179;
  }
  if (callback_count != 3 || fixture.view.pointer_move_count != 3) {
    return 180;
  }
  if (first_cursor != cgpui::CursorShape::pointing_hand) {
    return 181;
  }
  if (fixture.view.second_pointer_move_cursor_shape !=
          cgpui::CursorShape::text ||
      second_cursor != cgpui::CursorShape::text) {
    return 182;
  }
  if (fixture.view.last_cursor_shape != cgpui::CursorShape::default_arrow ||
      third_cursor != cgpui::CursorShape::default_arrow) {
    return 183;
  }

  return 0;
}

RuntimeFixture* pointer_capture_fixture = nullptr;

void dispatch_pointer_capture_sequence() {
  auto& callback = pointer_capture_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {1.0F, 1.0F}});
  callback(cgpui::PointerMoved{.position = {2.0F, 2.0F}});
  callback(cgpui::PointerMoved{.position = {3.0F, 3.0F}});
  callback(cgpui::PointerMoved{.position = {4.0F, 4.0F}});
}

int test_view_owner_can_capture_and_release_pointer() {
  RuntimeFixture fixture;
  pointer_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_capture_sequence;
  fixture.view.capture_on_first_pointer_move = true;
  fixture.view.release_on_third_pointer_move = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_capture_fixture = nullptr;

  if (result != 0) {
    return 60;
  }
  if (fixture.view.pointer_move_count != 4) {
    return 61;
  }
  if (!fixture.view.second_pointer_move_saw_capture) {
    return 62;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.last_pointer_captured) {
    return 63;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{4.0F, 4.0F})) {
    return 64;
  }

  return 0;
}

int test_pointer_capture_tracks_owner_view_id() {
  RuntimeFixture fixture;
  pointer_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_capture_sequence;
  fixture.view.capture_pointer_owner_on_first_pointer_move = true;
  fixture.view.release_pointer_with_wrong_owner_on_second_pointer_move = true;
  fixture.view.release_pointer_owner_on_third_pointer_move = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_capture_fixture = nullptr;

  if (result != 0) {
    return 90;
  }
  if (!fixture.view.saw_first_view_id ||
      fixture.view.first_view_id.value == 0 ||
      !fixture.view.view_id_stayed_stable) {
    return 91;
  }
  if (!fixture.view.second_pointer_move_saw_capture ||
      !fixture.view.second_pointer_move_saw_capture_owner) {
    return 92;
  }
  if (!fixture.view.third_pointer_move_saw_capture ||
      !fixture.view.third_pointer_move_saw_capture_owner) {
    return 93;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.fourth_pointer_move_saw_capture_owner ||
      fixture.view.last_pointer_capture_owner_present) {
    return 94;
  }
  if (!equal(
          fixture.view.last_input_pointer_position,
          cgpui::Point{4.0F, 4.0F})) {
    return 95;
  }

  return 0;
}

RuntimeFixture* pointer_element_capture_fixture = nullptr;

void dispatch_pointer_element_capture_sequence() {
  auto& callback = pointer_element_capture_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
  callback(cgpui::PointerMoved{.position = {30.0F, 15.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
}

int test_pointer_capture_routes_to_owner_element() {
  RuntimeFixture fixture;
  pointer_element_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_pointer_element_capture_sequence;
  fixture.view.capture_route_element_on_first_pointer_move = true;
  fixture.view.release_pointer_element_with_wrong_owner_on_second_pointer_move =
      true;
  fixture.view.release_pointer_element_owner_on_third_pointer_move = true;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{10});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{11});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 20.0F, .height = 30.0F});
  second->assign_id(cgpui::ElementId{12});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));
  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 40.0F) {
    return 140;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  int callback_count = 0;
  cgpui::EventDispatchRecord first_record{};
  cgpui::EventDispatchRecord second_record{};
  cgpui::EventDispatchRecord third_record{};
  cgpui::EventDispatchRecord fourth_record{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (callback_count == 1) {
          first_record = record;
        } else if (callback_count == 2) {
          second_record = record;
        } else if (callback_count == 3) {
          third_record = record;
        } else if (callback_count == 4) {
          fourth_record = record;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  pointer_element_capture_fixture = nullptr;

  if (result != 0) {
    return 131;
  }
  if (callback_count != 4 || fixture.view.pointer_move_count != 4) {
    return 132;
  }
  if (fixture.view.captured_pointer_element_id != cgpui::ElementId{11}) {
    return 133;
  }
  if (!first_record.route.target_element_id.has_value() ||
      *first_record.route.target_element_id != cgpui::ElementId{11}) {
    return 134;
  }
  if (!second_record.route.target_element_id.has_value() ||
      *second_record.route.target_element_id != cgpui::ElementId{11}) {
    return 135;
  }
  if (!third_record.route.target_element_id.has_value() ||
      *third_record.route.target_element_id != cgpui::ElementId{11}) {
    return 136;
  }
  if (!fourth_record.route.target_element_id.has_value() ||
      *fourth_record.route.target_element_id != cgpui::ElementId{12}) {
    return 137;
  }
  if (!fixture.view.second_pointer_move_saw_capture ||
      !fixture.view.second_pointer_move_saw_element_capture_owner ||
      !fixture.view.third_pointer_move_saw_capture ||
      !fixture.view.third_pointer_move_saw_element_capture_owner) {
    return 138;
  }
  if (fixture.view.fourth_pointer_move_saw_capture ||
      fixture.view.fourth_pointer_move_saw_element_capture_owner ||
      fixture.view.last_pointer_capture_owner_matches_captured_element) {
    return 139;
  }

  return 0;
}

RuntimeFixture* hover_with_capture_fixture = nullptr;

void dispatch_hover_with_capture_sequence() {
  auto& callback = hover_with_capture_fixture->window.callback;
  callback(cgpui::PointerMoved{.position = {5.0F, 5.0F}});
  callback(cgpui::PointerMoved{.position = {5.0F, 15.0F}});
}

int test_hover_tracks_hit_element_while_pointer_is_captured() {
  RuntimeFixture fixture;
  hover_with_capture_fixture = &fixture;
  fixture.app.on_run = &dispatch_hover_with_capture_sequence;
  fixture.view.capture_route_element_on_first_pointer_move = true;

  cgpui::VerticalStackElement stack;
  stack.assign_id(cgpui::ElementId{10});
  auto first = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 40.0F, .height = 10.0F});
  first->assign_id(cgpui::ElementId{11});
  auto second = std::make_unique<cgpui::FixedSizeElement>(
      cgpui::Size{.width = 20.0F, .height = 30.0F});
  second->assign_id(cgpui::ElementId{12});
  stack.append_child(std::move(first));
  stack.append_child(std::move(second));
  const cgpui::LayoutOutput output = stack.layout(cgpui::LayoutInput{});
  if (output.size.width != 40.0F || output.size.height != 40.0F) {
    return 172;
  }

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_root(&stack);

  const int result = runtime.run(cgpui::WindowDescriptor{});
  hover_with_capture_fixture = nullptr;

  if (result != 0) {
    return 173;
  }
  if (fixture.view.captured_pointer_element_id != cgpui::ElementId{11}) {
    return 174;
  }
  if (!fixture.view.second_pointer_move_saw_capture ||
      !fixture.view.second_pointer_move_saw_element_capture_owner) {
    return 175;
  }
  if (fixture.view.last_route_element_id != cgpui::ElementId{11}) {
    return 176;
  }
  if (!fixture.view.second_pointer_move_hovered_element_id.has_value() ||
      *fixture.view.second_pointer_move_hovered_element_id !=
          cgpui::ElementId{12} ||
      fixture.view.last_hovered_element_id != cgpui::ElementId{12}) {
    return 177;
  }

  return 0;
}

RuntimeFixture* keyboard_focus_fixture = nullptr;

void dispatch_keyboard_focus_sequence() {
  auto& callback = keyboard_focus_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 65,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 66,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "b"});
  callback(cgpui::KeyboardKey{
      .key_code = 67,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 68,
      .action = cgpui::KeyAction::pressed});
}

int test_view_can_request_and_release_keyboard_focus() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.request_keyboard_focus_on_first_key = true;
  fixture.view.release_keyboard_focus_on_third_key = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 70;
  }
  if (fixture.view.keyboard_key_count != 4 ||
      fixture.view.text_input_count != 1) {
    return 71;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.text_input_saw_keyboard_focus) {
    return 72;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.last_keyboard_focused) {
    return 73;
  }

  return 0;
}

int test_keyboard_focus_tracks_owner_view_id() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.request_keyboard_focus_owner_on_first_key = true;
  fixture.view.release_keyboard_focus_with_wrong_owner_on_second_key = true;
  fixture.view.release_keyboard_focus_owner_on_third_key = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 80;
  }
  if (!fixture.view.saw_first_view_id ||
      fixture.view.first_view_id.value == 0 ||
      !fixture.view.view_id_stayed_stable) {
    return 81;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.second_key_saw_keyboard_focus_owner) {
    return 82;
  }
  if (!fixture.view.text_input_saw_keyboard_focus ||
      !fixture.view.text_input_saw_keyboard_focus_owner) {
    return 83;
  }
  if (!fixture.view.third_key_saw_keyboard_focus ||
      !fixture.view.third_key_saw_keyboard_focus_owner) {
    return 84;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.fourth_key_saw_keyboard_focus_owner ||
      fixture.view.last_keyboard_focus_owner_present) {
    return 85;
  }

  return 0;
}

int test_keyboard_focus_routes_to_owner_element() {
  RuntimeFixture fixture;
  keyboard_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_keyboard_focus_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;
  fixture.view.release_keyboard_focus_element_with_wrong_owner_on_second_key =
      true;
  fixture.view.release_keyboard_focus_element_owner_on_third_key = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  int callback_count = 0;
  cgpui::EventDispatchRecord first_key_record{};
  cgpui::EventDispatchRecord second_key_record{};
  cgpui::EventDispatchRecord text_record{};
  cgpui::EventDispatchRecord third_key_record{};
  cgpui::EventDispatchRecord fourth_key_record{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        callback_count += 1;
        if (!context.event_route.has_value() ||
            context.event_route->target_view_id != record.route.target_view_id ||
            context.event_route->target_element_id !=
                record.route.target_element_id ||
            context.event_route->event_kind != record.route.event_kind) {
          callback_count = -100;
          return;
        }
        if (callback_count == 1) {
          first_key_record = record;
        } else if (callback_count == 2) {
          second_key_record = record;
        } else if (callback_count == 3) {
          text_record = record;
        } else if (callback_count == 4) {
          third_key_record = record;
        } else if (callback_count == 5) {
          fourth_key_record = record;
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  keyboard_focus_fixture = nullptr;

  if (result != 0) {
    return 141;
  }
  if (callback_count != 5 || fixture.view.keyboard_key_count != 4 ||
      fixture.view.text_input_count != 1) {
    return 142;
  }
  if (first_key_record.route.target_element_id.has_value()) {
    return 143;
  }
  if (!second_key_record.route.target_element_id.has_value() ||
      *second_key_record.route.target_element_id != cgpui::ElementId{21}) {
    return 144;
  }
  if (!text_record.route.target_element_id.has_value() ||
      *text_record.route.target_element_id != cgpui::ElementId{21}) {
    return 145;
  }
  if (!third_key_record.route.target_element_id.has_value() ||
      *third_key_record.route.target_element_id != cgpui::ElementId{21}) {
    return 146;
  }
  if (fourth_key_record.route.target_element_id.has_value()) {
    return 147;
  }
  if (!fixture.view.second_key_saw_keyboard_focus ||
      !fixture.view.second_key_saw_keyboard_focus_element_owner ||
      !fixture.view.text_input_saw_keyboard_focus ||
      !fixture.view.text_input_saw_keyboard_focus_element_owner ||
      !fixture.view.third_key_saw_keyboard_focus ||
      !fixture.view.third_key_saw_keyboard_focus_element_owner) {
    return 148;
  }
  if (fixture.view.fourth_key_saw_keyboard_focus ||
      fixture.view.fourth_key_saw_keyboard_focus_element_owner ||
      fixture.view.last_keyboard_focus_element_owner_present) {
    return 149;
  }

  return 0;
}

RuntimeFixture* entity_context_fixture = nullptr;

void dispatch_entity_context_sequence() {
  auto& callback = entity_context_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 70,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 71,
      .action = cgpui::KeyAction::pressed});
}

int test_context_can_access_runtime_entities() {
  RuntimeFixture fixture;
  entity_context_fixture = &fixture;
  fixture.app.on_run = &dispatch_entity_context_sequence;
  fixture.view.exercise_entity_context_access = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  entity_context_fixture = nullptr;

  if (result != 0) {
    return 112;
  }
  if (fixture.view.keyboard_key_count != 2) {
    return 113;
  }
  if (fixture.view.inserted_entity_id.value == 0 ||
      fixture.view.first_entity_read_value != 10 ||
      fixture.view.second_entity_read_value != 21) {
    return 114;
  }
  if (!fixture.view.removed_entity ||
      fixture.view.removed_entity_again ||
      !fixture.view.missing_entity_after_remove) {
    return 115;
  }
  if (fixture.view.emplaced_entity_id.value <=
          fixture.view.inserted_entity_id.value ||
      fixture.view.emplaced_entity_read_value != 42) {
    return 116;
  }

  return 0;
}

RuntimeFixture* view_model_subscription_fixture = nullptr;

void dispatch_view_model_subscription_sequence() {
  auto& callback = view_model_subscription_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 77,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_tracks_view_model_subscriptions() {
  RuntimeFixture fixture;
  view_model_subscription_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_model_subscription_sequence;
  fixture.view.exercise_view_model_subscriptions = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  view_model_subscription_fixture = nullptr;

  if (result != 0) {
    return 188;
  }
  if (fixture.view.subscriptions_after_subscribe != 1 ||
      !fixture.view.first_subscription_matches_entity) {
    return 189;
  }
  if (!fixture.view.notified_subscribed_entity ||
      fixture.view.notified_missing_entity) {
    return 190;
  }
  if (!fixture.view.invalidation_after_subscribed_notify.layout ||
      !fixture.view.invalidation_after_subscribed_notify.paint) {
    return 191;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1) {
    return 192;
  }

  return 0;
}

RuntimeFixture* view_identity_fixture = nullptr;

void dispatch_view_identity_sequence() {
  auto& callback = view_identity_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 80,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 81,
      .action = cgpui::KeyAction::pressed});
}

int test_context_allocates_stable_view_ids() {
  RuntimeFixture fixture;
  view_identity_fixture = &fixture;
  fixture.app.on_run = &dispatch_view_identity_sequence;
  fixture.view.exercise_view_identity_allocation = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  view_identity_fixture = nullptr;

  if (result != 0) {
    return 117;
  }
  if (fixture.view.keyboard_key_count != 2) {
    return 118;
  }
  if (!fixture.view.root_view_id_was_allocated ||
      fixture.view.zero_view_id_was_allocated) {
    return 119;
  }
  if (fixture.view.first_allocated_view_id.value <=
          fixture.view.first_view_id.value ||
      fixture.view.second_allocated_view_id.value <=
          fixture.view.first_allocated_view_id.value ||
      fixture.view.third_allocated_view_id.value <=
          fixture.view.second_allocated_view_id.value) {
    return 120;
  }
  if (!fixture.view.first_allocated_view_id_was_allocated ||
      !fixture.view.second_allocated_view_id_was_allocated ||
      !fixture.view.first_allocated_view_id_stayed_allocated ||
      !fixture.view.second_allocated_view_id_stayed_allocated ||
      !fixture.view.third_allocated_view_id_was_allocated) {
    return 121;
  }
  if (!fixture.view.next_unallocated_view_id_was_missing ||
      fixture.view.third_allocated_view_id.value !=
          fixture.view.second_allocated_view_id.value + 1) {
    return 122;
  }

  return 0;
}

RuntimeFixture* action_dispatch_fixture = nullptr;

void dispatch_action_dispatch_sequence() {
  auto& callback = action_dispatch_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

int test_runtime_dispatches_named_actions() {
  RuntimeFixture fixture;
  action_dispatch_fixture = &fixture;
  fixture.app.on_run = &dispatch_action_dispatch_sequence;
  fixture.view.exercise_action_dispatch = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  action_dispatch_fixture = nullptr;

  if (result != 0) {
    return 150;
  }
  if (fixture.view.keyboard_key_count != 1 ||
      fixture.view.dispatched_action_count != 1) {
    return 151;
  }
  if (fixture.view.action_saw_context_view_id != fixture.view.first_view_id) {
    return 152;
  }
  if (fixture.view.first_action_result.name != "app.save" ||
      !fixture.view.first_action_result.handled ||
      !fixture.view.first_action_result.result.consumed ||
      fixture.view.first_action_result.result.cancelled) {
    return 153;
  }
  if (fixture.view.second_action_result.name != "app.missing" ||
      fixture.view.second_action_result.handled ||
      fixture.view.second_action_result.result.consumed ||
      fixture.view.second_action_result.result.cancelled) {
    return 154;
  }
  if (!fixture.view.last_action_result_from_context.has_value() ||
      fixture.view.last_action_result_from_context->name != "app.missing" ||
      fixture.view.last_action_result_from_context->handled) {
    return 155;
  }

  return 0;
}

RuntimeFixture* invalidation_fixture = nullptr;

void dispatch_invalidation_sequence() {
  auto& callback = invalidation_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 73,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_tracks_layout_and_paint_invalidation_requests() {
  RuntimeFixture fixture;
  invalidation_fixture = &fixture;
  fixture.app.on_run = &dispatch_invalidation_sequence;
  fixture.view.exercise_invalidation_requests = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::InvalidationState callback_invalidation{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        callback_invalidation = context.runtime.invalidation_state();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  invalidation_fixture = nullptr;

  if (result != 0) {
    return 178;
  }
  if (fixture.view.initial_invalidation.layout ||
      fixture.view.initial_invalidation.paint) {
    return 179;
  }
  if (!fixture.view.after_layout_request_invalidation.layout ||
      !fixture.view.after_layout_request_invalidation.paint) {
    return 180;
  }
  if (!fixture.view.after_paint_request_invalidation.layout ||
      !fixture.view.after_paint_request_invalidation.paint) {
    return 181;
  }
  if (fixture.view.after_clear_invalidation.layout ||
      fixture.view.after_clear_invalidation.paint) {
    return 182;
  }
  if (runtime.invalidation_state().layout ||
      runtime.invalidation_state().paint || callback_invalidation.layout ||
      !callback_invalidation.paint) {
    return 183;
  }

  return 0;
}

RuntimeFixture* scheduled_invalidation_fixture = nullptr;

void dispatch_scheduled_invalidation_sequence() {
  auto& callback = scheduled_invalidation_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 82,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_schedules_redraw_for_invalidation_requests() {
  RuntimeFixture fixture;
  scheduled_invalidation_fixture = &fixture;
  fixture.app.on_run = &dispatch_scheduled_invalidation_sequence;
  fixture.view.exercise_scheduled_invalidation_redraw = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::InvalidationState after_frame_invalidation{
      .layout = true,
      .paint = true,
  };
  runtime.set_after_frame_callback(
      [&](const cgpui::WindowRuntimeContext& context) {
        after_frame_invalidation = context.runtime.invalidation_state();
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  scheduled_invalidation_fixture = nullptr;

  if (result != 0) {
    return 184;
  }
  if (fixture.window.request_redraw_count != 1) {
    return 185;
  }
  if (fixture.renderer.begin_frame_count != 1 ||
      fixture.view.paint_count != 1 || fixture.frame.present_count != 1) {
    return 186;
  }
  if (runtime.invalidation_state().layout ||
      runtime.invalidation_state().paint || after_frame_invalidation.layout ||
      after_frame_invalidation.paint) {
    return 187;
  }

  return 0;
}

RuntimeFixture* key_binding_fixture = nullptr;

void dispatch_key_binding_sequence() {
  auto& callback = key_binding_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 79,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true}});
}

int test_runtime_dispatches_key_binding_actions() {
  RuntimeFixture fixture;
  key_binding_fixture = &fixture;
  fixture.app.on_run = &dispatch_key_binding_sequence;

  int action_count = 0;
  bool action_saw_keyboard_route = false;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.register_action(
      "app.save",
      [&](const cgpui::WindowRuntimeContext& context) {
        action_count += 1;
        action_saw_keyboard_route =
            context.event_route.has_value() &&
            context.event_route->event_kind == cgpui::EventKind::keyboard_key;
        return cgpui::EventResult::consumed_event();
      });
  runtime.bind_key(cgpui::KeyBinding{
      .key_code = 83,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.control = true},
      .action_name = "app.save"});

  const int result = runtime.run(cgpui::WindowDescriptor{});
  key_binding_fixture = nullptr;

  if (result != 0) {
    return 156;
  }
  if (fixture.view.keyboard_key_count != 2 || action_count != 1) {
    return 157;
  }
  if (!action_saw_keyboard_route) {
    return 158;
  }
  const std::optional<cgpui::ActionDispatchResult> dispatch =
      runtime.last_action_dispatch();
  if (!dispatch.has_value() || dispatch->name != "app.save" ||
      !dispatch->handled || !dispatch->result.consumed ||
      dispatch->result.cancelled) {
    return 159;
  }

  return 0;
}

RuntimeFixture* text_input_routing_fixture = nullptr;

void dispatch_text_input_routing_sequence() {
  auto& callback = text_input_routing_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "hi"});
  callback(cgpui::KeyboardKey{
      .key_code = 85,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 86,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::TextInput{.text = "!"});
}

int test_runtime_routes_text_input_to_focused_text_model() {
  RuntimeFixture fixture;
  text_input_routing_fixture = &fixture;
  fixture.app.on_run = &dispatch_text_input_routing_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;
  fixture.view.release_keyboard_focus_element_owner_on_third_key = true;

  cgpui::TextModel model;
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);

  const int result = runtime.run(cgpui::WindowDescriptor{});
  text_input_routing_fixture = nullptr;

  if (result != 0) {
    return 160;
  }
  if (fixture.view.keyboard_key_count != 3 ||
      fixture.view.text_input_count != 2) {
    return 161;
  }
  if (model.text() != "hi") {
    return 162;
  }
  if (model.cursor() != 2) {
    return 164;
  }
  if (fixture.view.last_keyboard_focus_element_owner_present ||
      fixture.view.last_route_element_id.has_value()) {
    return 163;
  }

  return 0;
}

RuntimeFixture* text_edit_action_fixture = nullptr;

void dispatch_text_edit_action_sequence() {
  auto& callback = text_edit_action_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true}});
  callback(cgpui::KeyboardKey{
      .key_code = 8,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::KeyboardKey{
      .key_code = 46,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_routes_text_edit_actions_to_focused_text_model() {
  RuntimeFixture fixture;
  text_edit_action_fixture = &fixture;
  fixture.app.on_run = &dispatch_text_edit_action_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcd");
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .edit_action = cgpui::TextEditAction::move_previous});
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 37,
      .action = cgpui::KeyAction::pressed,
      .modifiers = {.shift = true},
      .edit_action = cgpui::TextEditAction::extend_previous});
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 8,
      .action = cgpui::KeyAction::pressed,
      .edit_action = cgpui::TextEditAction::backspace});
  runtime.bind_text_edit_action(cgpui::TextEditBinding{
      .key_code = 46,
      .action = cgpui::KeyAction::pressed,
      .edit_action = cgpui::TextEditAction::delete_forward});

  const int result = runtime.run(cgpui::WindowDescriptor{});
  text_edit_action_fixture = nullptr;

  if (result != 0) {
    return 200;
  }
  if (fixture.view.keyboard_key_count != 5) {
    return 201;
  }
  if (model.text() != "ab") {
    return 202;
  }
  if (model.cursor() != 2 || !model.selection().collapsed) {
    return 203;
  }
  return 0;
}

RuntimeFixture* clipboard_paste_fixture = nullptr;

void dispatch_clipboard_paste_sequence() {
  auto& callback = clipboard_paste_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_pastes_clipboard_text_into_focused_text_model() {
  RuntimeFixture fixture;
  clipboard_paste_fixture = &fixture;
  fixture.app.on_run = &dispatch_clipboard_paste_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("a");
  cgpui::MemoryClipboard clipboard;
  (void)clipboard.write_text("bc");

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_clipboard(&clipboard);

  bool pasted = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        pasted = context.runtime.paste_clipboard_text();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  clipboard_paste_fixture = nullptr;

  if (result != 0) {
    return 204;
  }
  if (!pasted) {
    return 205;
  }
  if (model.text() != "abc" || model.cursor() != 3) {
    return 206;
  }
  return 0;
}

RuntimeFixture* clipboard_copy_fixture = nullptr;

void dispatch_clipboard_copy_sequence() {
  auto& callback = clipboard_copy_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_copies_focused_text_selection_to_clipboard() {
  RuntimeFixture fixture;
  clipboard_copy_fixture = &fixture;
  fixture.app.on_run = &dispatch_clipboard_copy_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcd");
  model.set_selection(1, 3);
  cgpui::MemoryClipboard clipboard;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_clipboard(&clipboard);

  bool copied = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        copied = context.runtime.copy_selection_to_clipboard();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  clipboard_copy_fixture = nullptr;

  if (result != 0) {
    return 207;
  }
  if (!copied) {
    return 208;
  }
  const std::optional<std::string> copied_text = clipboard.read_text();
  if (!copied_text.has_value() || *copied_text != "bc") {
    return 209;
  }
  if (model.text() != "abcd" || model.selection().start != 1 ||
      model.selection().end != 3) {
    return 210;
  }
  return 0;
}

RuntimeFixture* clipboard_cut_fixture = nullptr;

void dispatch_clipboard_cut_sequence() {
  auto& callback = clipboard_cut_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_cuts_focused_text_selection_to_clipboard() {
  RuntimeFixture fixture;
  clipboard_cut_fixture = &fixture;
  fixture.app.on_run = &dispatch_clipboard_cut_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model("abcd");
  model.set_selection(1, 3);
  cgpui::MemoryClipboard clipboard;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_clipboard(&clipboard);

  bool cut = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        cut = context.runtime.cut_selection_to_clipboard();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  clipboard_cut_fixture = nullptr;

  if (result != 0) {
    return 211;
  }
  if (!cut) {
    return 212;
  }
  const std::optional<std::string> cut_text = clipboard.read_text();
  if (!cut_text.has_value() || *cut_text != "bc") {
    return 213;
  }
  if (model.text() != "ad" || model.cursor() != 1 ||
      !model.selection().collapsed) {
    return 214;
  }
  return 0;
}

RuntimeFixture* ime_composition_fixture = nullptr;

void dispatch_ime_composition_sequence() {
  auto& callback = ime_composition_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::ImeComposition{
      .phase = cgpui::ImeCompositionPhase::update,
      .text = "draft"});
  callback(cgpui::ImeComposition{
      .phase = cgpui::ImeCompositionPhase::commit,
      .text = "\xE4\xB8\xAD"});
}

int test_runtime_routes_ime_composition_to_focused_text_model() {
  RuntimeFixture fixture;
  ime_composition_fixture = &fixture;
  fixture.app.on_run = &dispatch_ime_composition_sequence;
  fixture.view.focused_keyboard_element_id = cgpui::ElementId{21};
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::TextModel model;
  std::optional<cgpui::EventDispatchRecord> update_record;
  std::optional<cgpui::EventDispatchRecord> commit_record;
  bool update_saw_composition = false;
  bool update_changed_text = false;
  bool commit_cleared_composition = false;
  bool commit_inserted_text = false;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.bind_text_model(cgpui::ElementId{21}, &model);
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind != cgpui::EventKind::ime_composition) {
          return;
        }
        if (!update_record.has_value()) {
          update_record = record;
          update_saw_composition =
              model.has_composition() &&
              model.composition_text() == std::string_view{"draft"};
          update_changed_text = !model.text().empty();
        } else {
          commit_record = record;
          commit_cleared_composition = !model.has_composition();
          commit_inserted_text =
              model.text() == std::string_view{"\xE4\xB8\xAD"};
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  ime_composition_fixture = nullptr;

  if (result != 0) {
    return 193;
  }
  if (fixture.view.keyboard_key_count != 1 ||
      fixture.view.ime_composition_count != 2) {
    return 194;
  }
  if (!update_record.has_value() || !commit_record.has_value()) {
    return 195;
  }
  if (update_record->route.event_kind != cgpui::EventKind::ime_composition ||
      commit_record->event_kind != cgpui::EventKind::ime_composition) {
    return 196;
  }
  if (!update_record->route.target_element_id.has_value() ||
      *update_record->route.target_element_id != cgpui::ElementId{21} ||
      !commit_record->route.target_element_id.has_value() ||
      *commit_record->route.target_element_id != cgpui::ElementId{21}) {
    return 197;
  }
  if (!fixture.view.ime_composition_saw_keyboard_focus ||
      !fixture.view.ime_composition_saw_keyboard_focus_element_owner) {
    return 198;
  }
  if (!update_saw_composition || update_changed_text ||
      !commit_cleared_composition || !commit_inserted_text) {
    return 199;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_redraw_paints_initial_viewport(); result != 0) {
    return result;
  }
  if (const int result = test_resize_updates_renderer_and_viewport();
      result != 0) {
    return result;
  }
  if (const int result = test_close_request_quits_application(); result != 0) {
    return result;
  }
  if (const int result = test_render_failure_quits_and_returns_failure();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_dispatches_input_events_to_view();
      result != 0) {
    return result;
  }
  if (const int result = test_view_event_can_request_redraw(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_exposes_last_view_event_result();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_reports_each_view_event_dispatch();
      result != 0) {
    return result;
  }
  if (const int result = test_event_router_routes_events_to_root_view();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_exposes_current_event_route();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_pointer_events_to_hit_element();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_owns_installed_element_tree();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_lays_out_owned_element_tree_on_redraw();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_dispatches_consumed_element_event_before_view_fallback();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_falls_back_to_view_after_unhandled_element_event();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_tracks_hovered_pointer_element();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_cursor_shape_from_hovered_element();
      result != 0) {
    return result;
  }
  if (const int result = test_view_owner_can_capture_and_release_pointer();
      result != 0) {
    return result;
  }
  if (const int result = test_pointer_capture_tracks_owner_view_id();
      result != 0) {
    return result;
  }
  if (const int result = test_pointer_capture_routes_to_owner_element();
      result != 0) {
    return result;
  }
  if (const int result =
          test_hover_tracks_hit_element_while_pointer_is_captured();
      result != 0) {
    return result;
  }
  if (const int result = test_view_can_request_and_release_keyboard_focus();
      result != 0) {
    return result;
  }
  if (const int result = test_keyboard_focus_tracks_owner_view_id();
      result != 0) {
    return result;
  }
  if (const int result = test_keyboard_focus_routes_to_owner_element();
      result != 0) {
    return result;
  }
  if (const int result = test_context_can_access_runtime_entities();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_tracks_view_model_subscriptions();
      result != 0) {
    return result;
  }
  if (const int result = test_context_allocates_stable_view_ids();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_dispatches_named_actions();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_tracks_layout_and_paint_invalidation_requests();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_schedules_redraw_for_invalidation_requests();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_dispatches_key_binding_actions();
      result != 0) {
    return result;
  }
  if (const int result = test_runtime_routes_text_input_to_focused_text_model();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_routes_text_edit_actions_to_focused_text_model();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_pastes_clipboard_text_into_focused_text_model();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_copies_focused_text_selection_to_clipboard();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_cuts_focused_text_selection_to_clipboard();
      result != 0) {
    return result;
  }
  if (const int result =
          test_runtime_routes_ime_composition_to_focused_text_model();
      result != 0) {
    return result;
  }
  return 0;
}
