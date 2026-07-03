#pragma once

#include "cgpui/platform/platform.hpp"
#include "cgpui/platform/clipboard.hpp"
#include "cgpui/ui/element.hpp"
#include "cgpui/ui/text.hpp"
#include "cgpui/ui/ui.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace {

struct RuntimeEntity {
  int value = 0;
};

struct RuntimeGlobal {
  int value = 0;
};

struct MissingRuntimeGlobal {
  int value = 0;
};

struct RuntimeElementState {
  explicit RuntimeElementState(int value) : value(value) {}

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

bool has_platform_diagnostic(
    std::span<const cgpui::PlatformDiagnosticEvent> events,
    cgpui::PlatformDiagnosticKind kind,
    std::string_view operation,
    cgpui::EventKind event_kind = cgpui::EventKind::unknown) {
  return std::ranges::any_of(
      events,
      [&](const cgpui::PlatformDiagnosticEvent& event) {
        return event.kind == kind && event.operation == operation &&
               (event_kind == cgpui::EventKind::unknown ||
                event.event_kind == event_kind);
      });
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

  void draw_text(const cgpui::TextDraw& text) override {
    text_draw_count += 1;
    last_text = text;
  }

  void draw_text_selection(const cgpui::TextSelectionDraw& selection) override {
    text_selection_draw_count += 1;
    last_text_selection = selection;
  }

  void draw_text_caret(const cgpui::TextCaretDraw& caret) override {
    text_caret_draw_count += 1;
    last_text_caret = caret;
  }

  cgpui::Result<void> present() override {
    present_count += 1;
    return {};
  }

  int clear_count = 0;
  int draw_count = 0;
  int text_draw_count = 0;
  int text_selection_draw_count = 0;
  int text_caret_draw_count = 0;
  int present_count = 0;
  cgpui::Color last_clear{};
  cgpui::SolidRect last_rect{};
  cgpui::TextDraw last_text{};
  cgpui::TextSelectionDraw last_text_selection{};
  cgpui::TextCaretDraw last_text_caret{};
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
    void draw_text(const cgpui::TextDraw& text) override {
      frame_.draw_text(text);
    }
    void draw_text_selection(const cgpui::TextSelectionDraw& selection) override {
      frame_.draw_text_selection(selection);
    }
    void draw_text_caret(const cgpui::TextCaretDraw& caret) override {
      frame_.draw_text_caret(caret);
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
    saw_current_event_route_helper = context.current_event_route().has_value();
    if (const std::optional<cgpui::EventRoute> route =
            context.current_event_route();
        route.has_value()) {
      current_event_route_helper = *route;
      current_event_route_helper_matches_field =
          context.event_route.has_value() &&
          route->target_view_id == context.event_route->target_view_id &&
          route->target_element_id == context.event_route->target_element_id &&
          route->event_kind == context.event_route->event_kind &&
          route->element_ancestry == context.event_route->element_ancestry &&
          route->view_ancestry == context.event_route->view_ancestry;
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
    last_event_scale = context.scale;
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
    context_alias_same_view_context_type =
        std::is_same_v<cgpui::Context<RecordingView>, cgpui::ViewContext>;
    const cgpui::Context<RecordingView>& author_context = context;
    context_alias_input_state = author_context.input_state();
    context_alias_input_state_matches_snapshot =
        context_alias_input_state.focused == context.input.focused &&
        context_alias_input_state.pointer_captured ==
            context.input.pointer_captured &&
        context_alias_input_state.pointer_capture_owner ==
            context.input.pointer_capture_owner &&
        context_alias_input_state.keyboard_focused ==
            context.input.keyboard_focused &&
        context_alias_input_state.keyboard_focus_owner ==
            context.input.keyboard_focus_owner &&
        context_alias_input_state.keyboard_focus_element_owner ==
            context.input.keyboard_focus_element_owner &&
        context_alias_input_state.hovered_element_id ==
            context.input.hovered_element_id &&
        context_alias_input_state.cursor_shape == context.input.cursor_shape &&
        equal(context_alias_input_state.pointer_position,
              context.input.pointer_position);
    saw_input_state_helper = true;
    input_state_helper = context.input_state();
    input_state_helper_matches_field =
        input_state_helper.focused == context.input.focused &&
        input_state_helper.pointer_captured == context.input.pointer_captured &&
        input_state_helper.pointer_capture_owner ==
            context.input.pointer_capture_owner &&
        input_state_helper.keyboard_focused == context.input.keyboard_focused &&
        input_state_helper.keyboard_focus_owner ==
            context.input.keyboard_focus_owner &&
        input_state_helper.keyboard_focus_element_owner ==
            context.input.keyboard_focus_element_owner &&
        input_state_helper.hovered_element_id == context.input.hovered_element_id &&
        input_state_helper.cursor_shape == context.input.cursor_shape &&
        equal(input_state_helper.pointer_position,
              context.input.pointer_position);

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
      if (exercise_view_context_pointer_capture_helpers &&
          pointer_move_count == 1 && context.event_route &&
          context.event_route->target_element_id.has_value()) {
        captured_pointer_element_id = *context.event_route->target_element_id;
        context.capture_pointer(captured_pointer_element_id);
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
      if (exercise_view_context_pointer_capture_helpers &&
          pointer_move_count == 3) {
        context.release_pointer(captured_pointer_element_id);
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
      if (exercise_view_context_model_helpers && keyboard_key_count == 1) {
        const cgpui::Context<RecordingView>& author_context = context;
        model_id = author_context.new_model<RuntimeEntity>(10);
        author_context.subscribe_view_to_entity(
            author_context.view_id,
            model_id);
        const RuntimeEntity* created = author_context.read_model(model_id);
        first_model_read_value = created == nullptr ? -1 : created->value;
        updated_model = author_context.update_model(
            model_id,
            [](RuntimeEntity& model) {
              model.value = 24;
            });
        invalidation_after_model_update =
            author_context.runtime.invalidation_state();
        const RuntimeEntity* updated = author_context.read_model(model_id);
        updated_model_read_value = updated == nullptr ? -1 : updated->value;
        update_missing_model = author_context.update_model(
            cgpui::Model<RuntimeEntity>{model_id.value + 100},
            [](RuntimeEntity& model) {
              model.value = 99;
            });
        removed_model = author_context.remove_model(model_id);
        invalidation_after_model_remove =
            author_context.runtime.invalidation_state();
        removed_model_again = author_context.remove_model(model_id);
        missing_model_after_remove =
            author_context.read_model(model_id) == nullptr;
      }
      if (exercise_view_context_model_observe_helper &&
          keyboard_key_count == 1) {
        model_id = context.new_model<RuntimeEntity>(7);
        observed_model = context.observe_model(
            model_id,
            [this](const cgpui::ViewContext& observe_context,
                   cgpui::Model<RuntimeEntity> observed) {
              model_observer_count += 1;
              model_observer_saw_view_id = observe_context.view_id;
              model_observer_saw_model = observed;
              const RuntimeEntity* current =
                  observe_context.read_model(observed);
              model_observer_last_value =
                  current == nullptr ? -1 : current->value;
            });
        observed_missing_model = context.observe_model(
            cgpui::Model<RuntimeEntity>{model_id.value + 100},
            [this](const cgpui::ViewContext&,
                   cgpui::Model<RuntimeEntity>) {
              missing_model_observer_count += 1;
            });
        updated_model = context.update_model(
            model_id,
            [](RuntimeEntity& model) {
              model.value = 13;
            });
        observer_count_after_update = model_observer_count;
        observer_value_after_update = model_observer_last_value;
        removed_model = context.remove_model(model_id);
        observer_count_after_remove = model_observer_count;
        observer_value_after_remove = model_observer_last_value;
        update_missing_model = context.update_model(
            cgpui::Model<RuntimeEntity>{model_id.value + 100},
            [](RuntimeEntity& model) {
              model.value = 99;
            });
      }
      if (exercise_subscription_ownership_token &&
          keyboard_key_count == 1) {
        model_id = context.new_model<RuntimeEntity>(100);
        {
          cgpui::Subscription subscription =
              context.observe_model_subscription(
                  model_id,
                  [this](const cgpui::ViewContext&,
                         cgpui::Model<RuntimeEntity>) {
                    model_observer_count += 1;
                  });
          observed_model = subscription.connected();
          owned_subscription_id = subscription.id();
          subscription_id_was_valid = owned_subscription_id.value != 0;
          updated_model = context.update_model(
              model_id,
              [](RuntimeEntity& model) {
                model.value = 101;
              });
          observer_count_after_update = model_observer_count;
        }
        notified_after_token_drop = context.update_model(
            model_id,
            [](RuntimeEntity& model) {
              model.value = 102;
            });
        observer_count_after_drop = model_observer_count;

        cgpui::Subscription removed_subscription =
            context.observe_model_subscription(
                model_id,
                [this](const cgpui::ViewContext&,
                       cgpui::Model<RuntimeEntity>) {
                  model_observer_count += 1;
                });
        removed_subscription_id = removed_subscription.id();
        removed_subscription_disconnect =
            context.runtime.remove_subscription(removed_subscription.id());
        removed_subscription_duplicate_disconnect =
            context.runtime.remove_subscription(removed_subscription.id());
        removed_subscription_release_after_remove =
            removed_subscription.release();
        notified_after_remove = context.update_model(
            model_id,
            [](RuntimeEntity& model) {
              model.value = 103;
            });
        observer_count_after_remove = model_observer_count;
        missing_subscription_release =
            !context.runtime.remove_subscription(cgpui::SubscriptionId{
                removed_subscription_id.value + 100});
      }
      if (exercise_entity_handle_helpers && keyboard_key_count == 1) {
        const cgpui::Context<RecordingView>& author_context = context;
        entity_handle =
            cgpui::EntityHandle<RuntimeEntity>(
                author_context.insert_entity(RuntimeEntity{.value = 55}));
        const RuntimeEntity* read = entity_handle.read(author_context);
        entity_handle_read_value = read == nullptr ? -1 : read->value;
        author_context.subscribe_view_to_entity(
            author_context.view_id,
            entity_handle.id());
        entity_handle_update = entity_handle.update(
            author_context,
            [](RuntimeEntity& entity) {
              entity.value = 89;
            });
        invalidation_after_model_update =
            author_context.runtime.invalidation_state();
        const RuntimeEntity* updated = entity_handle.read(author_context);
        entity_handle_updated_value =
            updated == nullptr ? -1 : updated->value;
        removed_entity = author_context.remove_entity(entity_handle.id());
        entity_handle_missing_read =
            entity_handle.read(author_context) == nullptr;
        entity_handle_missing_update = entity_handle.update(
            author_context,
            [](RuntimeEntity& entity) {
              entity.value = 144;
            });
        weak_model = entity_handle.downgrade();
        upgraded_removed_weak_model = author_context.upgrade_entity(weak_model);
      }
      if (exercise_global_state_helpers && keyboard_key_count == 1) {
        const cgpui::Context<RecordingView>& author_context = context;
        global_missing_before_set =
            author_context.global<RuntimeGlobal>() == nullptr;
        global_missing_update =
            author_context.update_global<MissingRuntimeGlobal>(
                [](MissingRuntimeGlobal& global) {
                  global.value = 9;
                });
        author_context.set_global(RuntimeGlobal{.value = 12});
        const RuntimeGlobal* first_global =
            author_context.global<RuntimeGlobal>();
        global_first_read_value =
            first_global == nullptr ? -1 : first_global->value;
        global_update =
            author_context.update_global<RuntimeGlobal>(
                [](RuntimeGlobal& global) {
                  global.value += 30;
                });
        const RuntimeGlobal* updated_global =
            author_context.global<RuntimeGlobal>();
        global_updated_value =
            updated_global == nullptr ? -1 : updated_global->value;
        author_context.set_global(RuntimeGlobal{.value = 77});
        const RuntimeGlobal* replaced_global =
            author_context.global<RuntimeGlobal>();
        global_replaced_value =
            replaced_global == nullptr ? -1 : replaced_global->value;
      }
      if (exercise_weak_entity_and_view_handles && keyboard_key_count == 1) {
        model_id = context.new_model<RuntimeEntity>(41);
        weak_model = cgpui::WeakEntity<RuntimeEntity>(model_id);
        upgraded_weak_model = context.upgrade_entity(weak_model);
        const RuntimeEntity* upgraded =
            upgraded_weak_model.has_value()
                ? context.read_model(*upgraded_weak_model)
                : nullptr;
        weak_model_read_value = upgraded == nullptr ? -1 : upgraded->value;
        removed_model = context.remove_model(model_id);
        upgraded_removed_weak_model = context.upgrade_entity(weak_model);

        first_allocated_view_id = context.allocate_view_id();
        weak_view = cgpui::WeakView(first_allocated_view_id);
        upgraded_weak_view = context.upgrade_view(weak_view);
        upgraded_root_weak_view =
            context.upgrade_view(cgpui::WeakView(context.view_id));
        upgraded_missing_weak_view =
            context.upgrade_view(cgpui::WeakView(cgpui::ViewId{
                first_allocated_view_id.value + 100}));
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
      if (exercise_view_context_action_helper && keyboard_key_count == 1) {
        context.register_action(
            "view.open",
            [this](const cgpui::ViewContext& action_context) {
              view_context_action_count += 1;
              view_context_action_saw_context_view_id =
                  action_context.view_id;
              return cgpui::EventResult::consumed_event();
            });
        view_context_first_action_result =
            context.dispatch_action("view.open");
        view_context_second_action_result =
            context.dispatch_action("view.missing");
        view_context_last_action_result =
            context.last_action_dispatch();
      }
      if (exercise_view_context_key_binding_helper &&
          keyboard_key_count == 1) {
        context.register_action(
            "view.save",
            [this](const cgpui::ViewContext& action_context) {
              view_context_key_binding_action_count += 1;
              view_context_key_binding_action_saw_keyboard_route =
                  action_context.event_route.has_value() &&
                  action_context.event_route->event_kind ==
                      cgpui::EventKind::keyboard_key;
              return cgpui::EventResult::consumed_event();
            });
        context.bind_key(cgpui::KeyBinding{
            .key_code = 83,
            .action = cgpui::KeyAction::pressed,
            .modifiers = {.control = true},
            .action_name = "view.save"});
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
      if (exercise_view_context_text_model_binding &&
          keyboard_key_count == 1) {
        context.bind_text_model(
            focused_keyboard_element_id,
            view_context_bound_text_model);
        context.request_keyboard_focus(focused_keyboard_element_id);
      }
      if (exercise_view_context_focused_text_mutation) {
        if (keyboard_key_count == 1) {
          context.bind_text_model(
              focused_keyboard_element_id,
              view_context_bound_text_model);
          context.request_keyboard_focus(focused_keyboard_element_id);
        } else if (keyboard_key_count == 2) {
          view_context_mutated_focused_text =
              context.mutate_focused_text_model([](cgpui::TextModel& model) {
                model.insert_text("!");
              });
        } else if (keyboard_key_count == 3) {
          context.release_keyboard_focus(focused_keyboard_element_id);
        } else if (keyboard_key_count == 4) {
          view_context_skipped_missing_focused_text =
              !context.mutate_focused_text_model([](cgpui::TextModel& model) {
                model.insert_text("?");
              });
        }
      }
      if (exercise_view_context_text_edit_binding &&
          keyboard_key_count == 1) {
        context.bind_text_model(
            focused_keyboard_element_id,
            view_context_bound_text_model);
        context.request_keyboard_focus(focused_keyboard_element_id);
        context.bind_text_edit_action(cgpui::TextEditBinding{
            .key_code = 37,
            .action = cgpui::KeyAction::pressed,
            .edit_action = cgpui::TextEditAction::move_previous});
      }
      if (exercise_view_context_element_tree_installation &&
          keyboard_key_count == 1) {
        context.set_element_tree(std::move(view_context_element_tree));
        context.request_layout();
      }
      if (exercise_view_context_cursor_binding && keyboard_key_count == 1) {
        context.set_element_cursor(
            view_context_cursor_element_id,
            cgpui::CursorShape::text);
      }
      if (exercise_view_context_ime_rect && keyboard_key_count == 2) {
        const std::optional<cgpui::ImeCandidateRect> rect =
            context.focused_text_ime_rect();
        view_context_ime_rect_present = rect.has_value();
        if (rect.has_value()) {
          view_context_ime_rect = *rect;
        }
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
      if (exercise_view_context_focus_element_helpers) {
        if (keyboard_key_count == 1) {
          context.focus(focused_keyboard_element_id);
        } else if (keyboard_key_count == 3) {
          context.blur(focused_keyboard_element_id);
        }
      }
      if (exercise_view_context_focus_handle) {
        const cgpui::FocusHandle focus_handle =
            context.focus_handle(focused_keyboard_element_id);
        const cgpui::FocusHandle other_handle =
            context.focus_handle(cgpui::ElementId{
                focused_keyboard_element_id.value + 1});
        if (keyboard_key_count == 1) {
          view_context_focus_handle_id = focus_handle.id();
          view_context_focus_handle_empty =
              context.focus_handle(cgpui::ElementId{}).empty();
          view_context_focus_handle_contains_before =
              focus_handle.contains(context);
          view_context_focus_handle_focused_before =
              focus_handle.focused(context.input_state());
          focus_handle.request(context);
        } else if (keyboard_key_count == 2) {
          view_context_focus_handle_contains_after_request =
              focus_handle.contains(context);
          view_context_focus_handle_focused_after_request =
              focus_handle.focused(context.input_state());
          view_context_focus_handle_other_contains =
              other_handle.contains(context.input_state());
          other_handle.release(context.runtime);
        } else if (keyboard_key_count == 3) {
          view_context_focus_handle_still_focused_after_wrong_release =
              focus_handle.focused(context);
          focus_handle.release(context);
        } else if (keyboard_key_count == 4) {
          view_context_focus_handle_released =
              !focus_handle.contains(context) &&
              !focus_handle.focused(context.input_state());
        }
      }
      if (exercise_view_context_convenience && keyboard_key_count == 1) {
        const cgpui::ViewContext& view_context = context;
        view_context_convenience_same_alias_type =
            &view_context.runtime == &context.runtime;
        view_context_initial_invalidation = view_context.invalidation_state();
        view_context.request_layout();
        view_context_after_layout_request_invalidation =
            view_context.invalidation_state();
        view_context.clear_invalidation();
        view_context_after_clear_invalidation =
            view_context.invalidation_state();
        view_context_copied_selection =
            view_context.copy_selection_to_clipboard();
        view_context_cut_selection = view_context.cut_selection_to_clipboard();
        view_context_pasted_clipboard = view_context.paste_clipboard_text();
        view_context.capture_pointer(
            cgpui::PointerCaptureOwner::element(focused_keyboard_element_id));
        view_context.request_keyboard_focus(focused_keyboard_element_id);
        view_context.release_pointer(
            cgpui::PointerCaptureOwner::element(focused_keyboard_element_id));
        view_context.release_keyboard_focus(focused_keyboard_element_id);
        view_context.request_paint();
        view_context_after_paint_request_invalidation =
            view_context.invalidation_state();
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
    } else if (std::holds_alternative<cgpui::ImeDeleteSurroundingText>(event)) {
      ime_delete_surrounding_count += 1;
      ime_delete_surrounding_saw_keyboard_focus =
          context.input.keyboard_focused;
      ime_delete_surrounding_saw_keyboard_focus_element_owner =
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
  int ime_delete_surrounding_count = 0;
  int event_redraw_requests = 0;
  bool request_redraw_on_event = false;
  bool consume_next_event = false;
  bool cancel_next_event = false;
  bool exercise_entity_context_access = false;
  bool exercise_view_model_subscriptions = false;
  bool exercise_view_context_model_helpers = false;
  bool exercise_view_context_model_observe_helper = false;
  bool exercise_subscription_ownership_token = false;
  bool exercise_entity_handle_helpers = false;
  bool exercise_global_state_helpers = false;
  bool exercise_weak_entity_and_view_handles = false;
  bool exercise_view_identity_allocation = false;
  bool exercise_action_dispatch = false;
  bool exercise_view_context_action_helper = false;
  bool exercise_view_context_key_binding_helper = false;
  bool exercise_invalidation_requests = false;
  bool exercise_scheduled_invalidation_redraw = false;
  bool exercise_view_context_convenience = false;
  bool exercise_view_context_text_model_binding = false;
  bool exercise_view_context_focused_text_mutation = false;
  bool exercise_view_context_text_edit_binding = false;
  bool exercise_view_context_element_tree_installation = false;
  bool exercise_view_context_cursor_binding = false;
  bool exercise_view_context_focus_element_helpers = false;
  bool exercise_view_context_focus_handle = false;
  bool exercise_view_context_pointer_capture_helpers = false;
  bool exercise_view_context_ime_rect = false;
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
  bool saw_input_state_helper = false;
  bool input_state_helper_matches_field = false;
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
  bool ime_delete_surrounding_saw_keyboard_focus = false;
  bool ime_delete_surrounding_saw_keyboard_focus_element_owner = false;
  bool last_event_result_consumed = false;
  bool last_event_result_cancelled = false;
  bool saw_event_route = false;
  bool saw_current_event_route_helper = false;
  bool current_event_route_helper_matches_field = false;
  bool saw_last_event_dispatch = false;
  bool saw_first_view_id = false;
  bool view_id_stayed_stable = true;
  bool removed_entity = false;
  bool removed_entity_again = true;
  bool missing_entity_after_remove = false;
  bool updated_model = false;
  bool update_missing_model = true;
  bool removed_model = false;
  bool removed_model_again = true;
  bool missing_model_after_remove = false;
  bool observed_model = false;
  bool observed_missing_model = true;
  bool first_subscription_matches_entity = false;
  bool notified_subscribed_entity = false;
  bool notified_missing_entity = true;
  int first_entity_read_value = -1;
  int second_entity_read_value = -1;
  int emplaced_entity_read_value = -1;
  int first_model_read_value = -1;
  int updated_model_read_value = -1;
  int model_observer_count = 0;
  int missing_model_observer_count = 0;
  int model_observer_last_value = -1;
  int observer_count_after_update = 0;
  int observer_count_after_remove = 0;
  int observer_count_after_drop = 0;
  int observer_value_after_update = -1;
  int observer_value_after_remove = -1;
  int entity_handle_read_value = -1;
  int entity_handle_updated_value = -1;
  bool entity_handle_update = false;
  bool entity_handle_missing_update = true;
  bool entity_handle_missing_read = false;
  bool global_missing_before_set = false;
  bool global_update = false;
  bool global_missing_update = true;
  bool subscription_id_was_valid = false;
  bool notified_after_token_drop = true;
  bool removed_subscription_disconnect = false;
  bool removed_subscription_duplicate_disconnect = true;
  bool removed_subscription_release_after_remove = true;
  bool notified_after_remove = true;
  bool missing_subscription_release = false;
  int global_first_read_value = -1;
  int global_updated_value = -1;
  int global_replaced_value = -1;
  std::size_t subscriptions_after_subscribe = 0;
  cgpui::EntityId<RuntimeEntity> entity_id{};
  cgpui::EntityId<RuntimeEntity> inserted_entity_id{};
  cgpui::EntityId<RuntimeEntity> emplaced_entity_id{};
  cgpui::EntityHandle<RuntimeEntity> entity_handle{};
  cgpui::Model<RuntimeEntity> model_id{};
  cgpui::SubscriptionId owned_subscription_id{};
  cgpui::SubscriptionId removed_subscription_id{};
  cgpui::WeakEntity<RuntimeEntity> weak_model{};
  cgpui::WeakView weak_view{};
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
  cgpui::ViewId model_observer_saw_view_id{};
  cgpui::Model<RuntimeEntity> model_observer_saw_model{};
  cgpui::ActionDispatchResult first_action_result{};
  cgpui::ActionDispatchResult second_action_result{};
  std::optional<cgpui::ActionDispatchResult> last_action_result_from_context;
  int view_context_action_count = 0;
  cgpui::ViewId view_context_action_saw_context_view_id{};
  cgpui::ActionDispatchResult view_context_first_action_result{};
  cgpui::ActionDispatchResult view_context_second_action_result{};
  std::optional<cgpui::ActionDispatchResult> view_context_last_action_result;
  int view_context_key_binding_action_count = 0;
  bool view_context_key_binding_action_saw_keyboard_route = false;
  cgpui::InvalidationState initial_invalidation{};
  cgpui::InvalidationState after_layout_request_invalidation{};
  cgpui::InvalidationState after_paint_request_invalidation{};
  cgpui::InvalidationState after_clear_invalidation{};
  cgpui::InvalidationState invalidation_after_subscribed_notify{};
  cgpui::InvalidationState invalidation_after_model_update{};
  cgpui::InvalidationState invalidation_after_model_remove{};
  cgpui::InvalidationState view_context_initial_invalidation{};
  cgpui::InvalidationState view_context_after_layout_request_invalidation{};
  cgpui::InvalidationState view_context_after_clear_invalidation{};
  cgpui::InvalidationState view_context_after_paint_request_invalidation{};
  bool view_context_convenience_same_alias_type = false;
  bool context_alias_same_view_context_type = false;
  bool context_alias_input_state_matches_snapshot = false;
  bool view_context_copied_selection = false;
  bool view_context_cut_selection = false;
  bool view_context_pasted_clipboard = false;
  bool view_context_mutated_focused_text = false;
  bool view_context_skipped_missing_focused_text = true;
  bool view_context_ime_rect_present = false;
  bool view_context_focus_handle_empty = false;
  bool view_context_focus_handle_contains_before = true;
  bool view_context_focus_handle_focused_before = true;
  bool view_context_focus_handle_contains_after_request = false;
  bool view_context_focus_handle_focused_after_request = false;
  bool view_context_focus_handle_other_contains = true;
  bool view_context_focus_handle_still_focused_after_wrong_release = false;
  bool view_context_focus_handle_released = false;
  cgpui::ImeCandidateRect view_context_ime_rect{};
  int weak_model_read_value = -1;
  std::optional<cgpui::Model<RuntimeEntity>> upgraded_weak_model;
  std::optional<cgpui::Model<RuntimeEntity>> upgraded_removed_weak_model;
  std::optional<cgpui::ViewId> upgraded_weak_view;
  std::optional<cgpui::ViewId> upgraded_root_weak_view;
  std::optional<cgpui::ViewId> upgraded_missing_weak_view;
  cgpui::ViewId first_allocated_view_id{};
  cgpui::ViewId second_allocated_view_id{};
  cgpui::ViewId third_allocated_view_id{};
  cgpui::ElementId captured_pointer_element_id{};
  cgpui::ElementId focused_keyboard_element_id{21};
  cgpui::ElementId view_context_focus_handle_id{};
  cgpui::ElementId view_context_cursor_element_id{};
  cgpui::EventRoute last_event_route{};
  cgpui::EventRoute current_event_route_helper{};
  cgpui::EventDispatchRecord last_event_dispatch{};
  cgpui::ViewInputState input_state_helper{};
  cgpui::ViewInputState context_alias_input_state{};
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
  cgpui::DpiScale last_event_scale{};
  cgpui::Point last_input_pointer_position{};
  cgpui::Point pointer_move_event_position{};
  cgpui::Point pointer_button_event_position{};
  cgpui::Point pointer_scroll_event_position{};
  cgpui::TextModel* view_context_bound_text_model = nullptr;
  std::unique_ptr<cgpui::ElementTree> view_context_element_tree;
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
    saw_drag_event =
        saw_drag_event || std::holds_alternative<cgpui::DragEntered>(event) ||
        std::holds_alternative<cgpui::DragUpdated>(event) ||
        std::holds_alternative<cgpui::DragDropped>(event) ||
        std::holds_alternative<cgpui::DragExited>(event);
    if (const auto* dropped = std::get_if<cgpui::DragDropped>(&event);
        dropped != nullptr) {
      last_drag_payload = dropped->payload;
      last_drag_action = dropped->action;
    }
    return result;
  }

  int event_count = 0;
  bool saw_pointer_event = false;
  bool saw_drag_event = false;
  cgpui::ElementId last_target_element_id;
  cgpui::DragDropPayload last_drag_payload;
  cgpui::DragDropAction last_drag_action = cgpui::DragDropAction::none;
  cgpui::EventResult result = cgpui::EventResult::unhandled();
};

class RuntimeFocusableElement final : public cgpui::FixedSizeElement {
 public:
  explicit RuntimeFocusableElement(cgpui::Size preferred_size)
      : cgpui::FixedSizeElement(preferred_size) {}

  [[nodiscard]] bool focusable() const override {
    return true;
  }

  void focus(const cgpui::ElementFocusContext& context) override {
    focus_count += 1;
    last_focused_element_id = context.element_id;
  }

  int focus_count = 0;
  cgpui::ElementId last_focused_element_id;
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

  void set_cursor(cgpui::CursorShape cursor_shape) override {
    set_cursor_count += 1;
    last_cursor_shape = cursor_shape;
  }

  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement> placement) override {
    ime_placement_count += 1;
    last_ime_placement = placement;
    ime_placement_history.push_back(placement);
  }

  void update_accessibility_tree(
      cgpui::PlatformAccessibilityTreeUpdate update) override {
    accessibility_update_count += 1;
    last_accessibility_update = std::move(update);
  }

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
  int set_cursor_count = 0;
  int ime_placement_count = 0;
  int accessibility_update_count = 0;
  cgpui::CursorShape last_cursor_shape = cgpui::CursorShape::default_arrow;
  std::optional<cgpui::ImeTextInputPlacement> last_ime_placement;
  std::optional<cgpui::PlatformAccessibilityTreeUpdate>
      last_accessibility_update;
  std::vector<std::optional<cgpui::ImeTextInputPlacement>>
      ime_placement_history;
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
  int request_wakeup_count = 0;
  bool wakeup_pending = false;

  void request_wakeup() override {
    request_wakeup_count += 1;
    wakeup_pending = true;
  }

  void dispatch_wakeup() {
    if (!wakeup_pending) {
      return;
    }
    wakeup_pending = false;
    if (window_.callback) {
      window_.callback(cgpui::WindowWakeupRequested{});
    }
  }

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
    void set_cursor(cgpui::CursorShape cursor_shape) override {
      window_.set_cursor(cursor_shape);
    }
    void set_ime_text_input_placement(
        std::optional<cgpui::ImeTextInputPlacement> placement) override {
      window_.set_ime_text_input_placement(placement);
    }
    void update_accessibility_tree(
        cgpui::PlatformAccessibilityTreeUpdate update) override {
      window_.update_accessibility_tree(std::move(update));
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

class RegistryView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

class RenderHookView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override { paint_count += 1; }

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    render_count += 1;
    saw_context_view_id = context.view_id == cgpui::ViewId{9};
    saw_context_viewport_size =
        equal(context.viewport_size, cgpui::Size{123.0F, 45.0F});
    context.request_paint();
    return cgpui::into_element(
        cgpui::div().size(cgpui::Size{10.0F, 20.0F}));
  }

  int paint_count = 0;
  int render_count = 0;
  bool saw_context_view_id = false;
  bool saw_context_viewport_size = false;
};

class RuntimeRenderView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size viewport_size) override {
    paint_count += 1;
    last_paint_viewport_size = viewport_size;
  }

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    render_count += 1;
    saw_context_view_id = context.view_id == cgpui::ViewId{1};
    saw_context_viewport_size =
        equal(context.viewport_size, cgpui::Size{640.0F, 480.0F});
    return cgpui::into_element(
        cgpui::div().size(cgpui::Size{700.0F, 600.0F}));
  }

  int paint_count = 0;
  int render_count = 0;
  bool saw_context_view_id = false;
  bool saw_context_viewport_size = false;
  cgpui::Size last_paint_viewport_size{};
};

class HidpiRuntimeView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    render_count += 1;
    render_viewport_size = context.viewport_size;
    render_scale = context.scale;
    return cgpui::into_element(cgpui::text(model_)
                                   .font(cgpui::FontDescriptor{
                                       .family = "HiDPI"})
                                   .font_size(20.0F));
  }

  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport_size) override {
    paint_count += 1;
    paint_viewport_size = viewport_size;
    paint_list.fill_text(
        cgpui::Rect{
            .origin = {.x = 2.0F, .y = 3.0F},
            .size = {.width = 20.0F, .height = 20.0F}},
        cgpui::Color{.r = 0.9F, .g = 0.9F, .b = 0.9F, .a = 1.0F},
        "AB",
        cgpui::FontDescriptor{.family = "HiDPI"},
        20.0F);
  }

  int render_count = 0;
  int paint_count = 0;
  cgpui::Size render_viewport_size{};
  cgpui::Size paint_viewport_size{};
  cgpui::DpiScale render_scale{};

 private:
  cgpui::TextModel model_{"AB"};
};

class RenderInvalidationView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override { paint_count += 1; }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    event_count += 1;
    before_request = context.invalidation_state();
    context.request_render();
    after_request = context.invalidation_state();
    after_runtime_request = context.runtime.invalidation_state();
    return cgpui::EventResult::consumed_event();
  }

  int event_count = 0;
  int paint_count = 0;
  cgpui::InvalidationState before_request{};
  cgpui::InvalidationState after_request{};
  cgpui::InvalidationState after_runtime_request{};
};

} // namespace
