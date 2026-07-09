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

void WindowRuntime::apply_cursor_shape(CursorShape cursor_shape) {
  if (window_ == nullptr || applied_cursor_shape_ == cursor_shape) {
    return;
  }
  applied_cursor_shape_ = cursor_shape;
  window_->set_cursor(cursor_shape);
}

void WindowRuntime::apply_focused_text_ime_placement() {
  std::optional<ImeTextInputPlacement> placement;
  const TextModel* model = focused_text_model();
  if (const std::optional<ImeCandidateRect> candidate =
          focused_text_ime_rect();
      candidate.has_value() && model != nullptr) {
    placement = ImeTextInputPlacement{
        .rect = candidate->rect,
        .candidate_rect = candidate->rect,
        .byte_offset = candidate->byte_offset,
        .surrounding_text = std::string(model->text()),
        .selection_anchor = model->selection_anchor(),
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
