#include "win32_internal.hpp"

namespace cgpui {

DragDropPayload drag_payload_from_test_hook(
    const Win32TestDragDropPayload* payload) {
  if (payload == nullptr) {
    return {};
  }

  DragDropPayload result;
  if (payload->files != nullptr && payload->file_count > 0) {
    result.kind = DragDropPayloadKind::files;
    result.files.reserve(payload->file_count);
    for (std::size_t index = 0; index < payload->file_count; ++index) {
      if (payload->files[index] != nullptr) {
        result.files.push_back(utf8_from_utf16(payload->files[index]));
      }
    }
    return result;
  }
  if (payload->text != nullptr && payload->text[0] != L'\0') {
    result.kind = DragDropPayloadKind::text;
    result.text = utf8_from_utf16(payload->text);
  }
  return result;
}

Point drag_position_from_test_hook(const Win32TestDragDropPayload* payload) {
  if (payload == nullptr) {
    return {};
  }
  return Point{.x = payload->x, .y = payload->y};
}

} // namespace cgpui
