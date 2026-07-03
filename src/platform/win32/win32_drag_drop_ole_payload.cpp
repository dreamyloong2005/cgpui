#include "win32_internal.hpp"

namespace cgpui {

DragDropPayload drag_payload_from_ole_data_object(IDataObject* data_object) {
  if (data_object == nullptr) {
    return {};
  }

  FORMATETC text_format{
      .cfFormat = CF_UNICODETEXT,
      .ptd = nullptr,
      .dwAspect = DVASPECT_CONTENT,
      .lindex = -1,
      .tymed = TYMED_HGLOBAL,
  };
  STGMEDIUM text_storage{};
  if (SUCCEEDED(data_object->GetData(&text_format, &text_storage))) {
    DragDropPayload payload;
    if (text_storage.tymed == TYMED_HGLOBAL &&
        text_storage.hGlobal != nullptr) {
      const auto* text =
          static_cast<const wchar_t*>(GlobalLock(text_storage.hGlobal));
      if (text != nullptr && text[0] != L'\0') {
        payload.kind = DragDropPayloadKind::text;
        payload.text = utf8_from_utf16(text);
      }
      if (text != nullptr) {
        GlobalUnlock(text_storage.hGlobal);
      }
    }
    ReleaseStgMedium(&text_storage);
    if (payload.kind != DragDropPayloadKind::none) {
      return payload;
    }
  }

  FORMATETC file_format{
      .cfFormat = CF_HDROP,
      .ptd = nullptr,
      .dwAspect = DVASPECT_CONTENT,
      .lindex = -1,
      .tymed = TYMED_HGLOBAL,
  };
  STGMEDIUM file_storage{};
  if (SUCCEEDED(data_object->GetData(&file_format, &file_storage))) {
    DragDropPayload payload;
    if (file_storage.tymed == TYMED_HGLOBAL &&
        file_storage.hGlobal != nullptr) {
      auto* drop_handle = reinterpret_cast<HDROP>(file_storage.hGlobal);
      const UINT file_count =
          DragQueryFileW(drop_handle, 0xFFFFFFFFU, nullptr, 0);
      if (file_count > 0) {
        payload.kind = DragDropPayloadKind::files;
        payload.files.reserve(file_count);
        for (UINT index = 0; index < file_count; ++index) {
          const UINT length = DragQueryFileW(drop_handle, index, nullptr, 0);
          std::wstring file_path(static_cast<std::size_t>(length) + 1U, L'\0');
          DragQueryFileW(
              drop_handle,
              index,
              file_path.data(),
              static_cast<UINT>(file_path.size()));
          file_path.resize(length);
          if (!file_path.empty()) {
            payload.files.push_back(utf8_from_utf16(file_path));
          }
        }
      }
    }
    ReleaseStgMedium(&file_storage);
    return payload;
  }

  return {};
}

} // namespace cgpui
