#include "ui_internal.hpp"

namespace cgpui {
namespace {

bool reclaimable_closed_record(
    const WindowRuntimeRecord& record,
    WindowRuntimeId root_runtime_id) {
  return record.runtime_id != root_runtime_id && !record.active &&
      record.window == nullptr && record.renderer == nullptr &&
      !record.owns_window && !record.owns_renderer && !record.owns_root_view;
}

} // namespace

void WindowRuntime::reclaim_closed_window_records() {
  std::erase_if(
      app_opened_windows_,
      [this](const AppOpenedWindow& opened) {
        const WindowRuntimeRecord* record =
            find_window_runtime_record(opened.runtime_id);
        return record != nullptr &&
            reclaimable_closed_record(*record, root_window_runtime_id_);
      });
  std::erase_if(
      window_runtime_records_,
      [this](const WindowRuntimeRecord& record) {
        if (!reclaimable_closed_record(record, root_window_runtime_id_)) {
          return false;
        }
        window_themes_.erase(record.runtime_id.value);
        return true;
      });
}

} // namespace cgpui
