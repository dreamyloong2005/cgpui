#include "ui_internal.hpp"

namespace cgpui {

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

} // namespace cgpui
