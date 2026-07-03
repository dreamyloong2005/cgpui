#include "ui_internal.hpp"

namespace cgpui {

NativeMenuInstallation WindowRuntime::install_native_menu(
    NativeMenuModel menu) {
  NativeMenuModel stored_menu = std::move(menu);
  const PlatformMenuInstallationResult platform_result =
      application_.install_native_menu(stored_menu);
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::menu,
      .backend = platform_result.backend,
      .operation = "install-native-menu",
      .supported = platform_result.supported,
      .succeeded = platform_result.supported,
      .value_count = platform_result.item_count,
  });
  native_menu_installation_ = NativeMenuInstallation{
      .model = std::move(stored_menu),
      .platform = platform_result,
  };
  return native_menu_installation_;
}

const NativeMenuInstallation& WindowRuntime::native_menu_installation() const {
  return native_menu_installation_;
}

NativeFileDialogResult WindowRuntime::show_native_file_dialog(
    NativeFileDialogOptions options) {
  native_file_dialog_result_ =
      application_.show_native_file_dialog(std::move(options));
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::file_dialog,
      .backend = native_file_dialog_result_.backend,
      .operation = "show-native-file-dialog",
      .supported = native_file_dialog_result_.supported,
      .succeeded = native_file_dialog_result_.accepted,
      .value_count = native_file_dialog_result_.paths.size(),
  });
  return native_file_dialog_result_;
}

const NativeFileDialogResult& WindowRuntime::native_file_dialog_result()
    const {
  return native_file_dialog_result_;
}

void WindowRuntime::set_app_theme(Theme theme) {
  app_theme_ = std::move(theme);
  request_render();
}

const Theme& WindowRuntime::app_theme() const {
  return app_theme_;
}

void WindowRuntime::set_window_theme(
    WindowRuntimeId runtime_id,
    Theme theme) {
  if (runtime_id.value == 0) {
    return;
  }
  window_themes_[runtime_id.value] = std::move(theme);
  request_render();
}

bool WindowRuntime::clear_window_theme(WindowRuntimeId runtime_id) {
  if (runtime_id.value == 0) {
    return false;
  }
  const bool erased = window_themes_.erase(runtime_id.value) != 0;
  if (erased) {
    request_render();
  }
  return erased;
}

const Theme* WindowRuntime::window_theme(WindowRuntimeId runtime_id) const {
  if (runtime_id.value == 0) {
    return nullptr;
  }
  const auto theme = window_themes_.find(runtime_id.value);
  if (theme == window_themes_.end()) {
    return nullptr;
  }
  return &theme->second;
}

std::optional<Color> WindowRuntime::theme_color(
    WindowRuntimeId runtime_id,
    const ThemeTokenId& id) const {
  if (const Theme* theme = window_theme(runtime_id); theme != nullptr) {
    if (std::optional<Color> color = theme->color(id); color.has_value()) {
      return color;
    }
  }
  return app_theme_.color(id);
}

std::optional<float> WindowRuntime::theme_spacing(
    WindowRuntimeId runtime_id,
    const ThemeTokenId& id) const {
  if (const Theme* theme = window_theme(runtime_id); theme != nullptr) {
    if (std::optional<float> spacing = theme->spacing(id);
        spacing.has_value()) {
      return spacing;
    }
  }
  return app_theme_.spacing(id);
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
  const RuntimeTaskDiagnostics task_counts = task_diagnostics();

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
      .platform_diagnostics = platform_diagnostics_,
      .task_count = task_counts.task_count,
      .active_task_count = task_counts.active_task_count,
      .queued_task_count = task_counts.queued_task_count,
      .completed_task_count = task_counts.completed_task_count,
      .cancelled_task_count = task_counts.cancelled_task_count,
      .background_task_count = task_counts.background_task_count,
  };
}

std::span<const PlatformDiagnosticEvent> WindowRuntime::platform_diagnostics()
    const {
  return platform_diagnostics_;
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


void WindowRuntime::update_platform_accessibility_tree() {
  if (window_ == nullptr || owned_element_tree_ == nullptr) {
    return;
  }
  PlatformAccessibilityTreeUpdate update = build_platform_accessibility_update();
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::accessibility,
      .backend = "runtime",
      .operation = "update-tree",
      .supported = true,
      .succeeded = true,
      .value_count = update.node_count,
  });
  last_platform_accessibility_update_ = update;
  window_->update_accessibility_tree(std::move(update));
}

PlatformAccessibilityTreeUpdate
WindowRuntime::build_platform_accessibility_update() const {
  PlatformAccessibilityTreeUpdate update =
      platform_accessibility_update_from(accessibility_snapshot());
  if (last_platform_accessibility_update_.has_value()) {
    append_platform_accessibility_live_updates(
        update,
        *last_platform_accessibility_update_);
  }
  return update;
}


WindowRuntime::RuntimeTaskDiagnostics WindowRuntime::task_diagnostics()
    const {
  RuntimeTaskDiagnostics diagnostics;
  std::lock_guard lock(tasks_mutex_);
  diagnostics.task_count = tasks_.size();
  diagnostics.queued_task_count = task_completion_queue_.size();
  for (const RuntimeTask& task : tasks_) {
    if (task.background) {
      diagnostics.background_task_count += 1;
    }
    if (task.completed) {
      diagnostics.completed_task_count += 1;
    }
    if (task.cancelled) {
      diagnostics.cancelled_task_count += 1;
    }
    if (!task.queued && !task.completed && !task.cancelled) {
      diagnostics.active_task_count += 1;
    }
  }
  return diagnostics;
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
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::ime,
        .backend = "runtime",
        .operation = placement.has_value() ? "set-placement" :
                                             "clear-placement",
        .supported = true,
        .succeeded = true,
        .value_count = placement.has_value() ? placement->byte_offset : 0U,
    });
    window_->set_ime_text_input_placement(placement);
  }
}

void WindowRuntime::record_platform_diagnostic(
    PlatformDiagnosticEvent event) {
  constexpr std::size_t platform_diagnostic_limit = 32;
  event.sequence = ++platform_diagnostic_sequence_;
  if (event.backend.empty()) {
    event.backend = "runtime";
  }
  if (platform_diagnostics_.size() >= platform_diagnostic_limit) {
    platform_diagnostics_.erase(platform_diagnostics_.begin());
  }
  platform_diagnostics_.push_back(std::move(event));
}


} // namespace cgpui
