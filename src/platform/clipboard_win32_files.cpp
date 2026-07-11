#include "clipboard_win32_internal.hpp"

#if defined(_WIN32)
#include <shellapi.h>
#include <shlobj_core.h>

#include <cstring>
#include <limits>
#include <utility>
#include <vector>

namespace cgpui {

std::optional<std::vector<std::string>> Win32Clipboard::read_files() const {
  if (IsClipboardFormatAvailable(CF_HDROP) == FALSE ||
      OpenClipboard(nullptr) == FALSE) {
    return std::nullopt;
  }
  const HANDLE handle = GetClipboardData(CF_HDROP);
  if (handle == nullptr) {
    CloseClipboard();
    return std::nullopt;
  }

  const auto drop = reinterpret_cast<HDROP>(handle);
  const UINT count = DragQueryFileW(drop, 0xFFFFFFFFU, nullptr, 0);
  if (count == 0) {
    CloseClipboard();
    return std::nullopt;
  }

  std::vector<std::string> paths;
  paths.reserve(count);
  for (UINT index = 0; index < count; ++index) {
    const UINT length = DragQueryFileW(drop, index, nullptr, 0);
    if (length == 0) {
      CloseClipboard();
      return std::nullopt;
    }
    std::wstring path(static_cast<std::size_t>(length) + 1U, L'\0');
    if (DragQueryFileW(drop, index, path.data(), length + 1U) != length) {
      CloseClipboard();
      return std::nullopt;
    }
    path.resize(length);
    auto utf8 = narrow_clipboard_text(path);
    if (!utf8) {
      CloseClipboard();
      return std::nullopt;
    }
    paths.push_back(std::move(*utf8));
  }
  CloseClipboard();
  return paths;
}

bool Win32Clipboard::write_files(std::span<const std::string> paths) {
  if (paths.empty()) return false;

  std::vector<std::wstring> wide_paths;
  wide_paths.reserve(paths.size());
  std::size_t character_count = 1U;
  for (const std::string& path : paths) {
    auto wide = widen_clipboard_text(path);
    if (!wide || wide->empty() ||
        wide->size() > (std::numeric_limits<std::size_t>::max)() -
            character_count - 1U) {
      return false;
    }
    character_count += wide->size() + 1U;
    wide_paths.push_back(std::move(*wide));
  }
  if (character_count >
      ((std::numeric_limits<std::size_t>::max)() - sizeof(DROPFILES)) /
          sizeof(wchar_t)) {
    return false;
  }

  const std::size_t byte_size =
      sizeof(DROPFILES) + character_count * sizeof(wchar_t);
  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, byte_size);
  if (handle == nullptr) return false;
  auto* drop = static_cast<DROPFILES*>(GlobalLock(handle));
  if (drop == nullptr) {
    GlobalFree(handle);
    return false;
  }
  drop->pFiles = sizeof(DROPFILES);
  drop->fWide = TRUE;
  auto* cursor = reinterpret_cast<wchar_t*>(
      reinterpret_cast<unsigned char*>(drop) + drop->pFiles);
  for (const std::wstring& path : wide_paths) {
    std::memcpy(cursor, path.data(), path.size() * sizeof(wchar_t));
    cursor += path.size();
    *cursor++ = L'\0';
  }
  *cursor = L'\0';
  GlobalUnlock(handle);

  if (OpenClipboard(nullptr) == FALSE) {
    GlobalFree(handle);
    return false;
  }
  const bool installed = EmptyClipboard() != FALSE &&
      SetClipboardData(CF_HDROP, handle) != nullptr;
  CloseClipboard();
  if (!installed) GlobalFree(handle);
  return installed;
}

} // namespace cgpui
#endif
