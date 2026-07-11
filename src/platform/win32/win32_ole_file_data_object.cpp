#include "win32_ole_file_data_object_internal.hpp"

#include <climits>
#include <cstring>
#include <limits>
#include <shlobj_core.h>

namespace cgpui {
namespace {
bool convert_path(std::string_view path, std::wstring& result) {
  if (path.empty() || path.find('\0') != std::string_view::npos ||
      path.size() > INT_MAX) return false;
  const int required = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, path.data(), static_cast<int>(path.size()),
      nullptr, 0);
  if (required <= 0) return false;
  result.resize(static_cast<std::size_t>(required));
  return MultiByteToWideChar(
             CP_UTF8, MB_ERR_INVALID_CHARS, path.data(),
             static_cast<int>(path.size()), result.data(), required) == required;
}
} // namespace

Win32OleFileDataObject::Win32OleFileDataObject(
    std::span<const std::string> paths) {
  if (paths.empty()) return;
  paths_.reserve(paths.size());
  std::size_t characters = 1U;
  for (const std::string& path : paths) {
    std::wstring converted;
    if (!convert_path(path, converted) ||
        converted.size() > (std::numeric_limits<std::size_t>::max)() -
            characters - 1U) return;
    characters += converted.size() + 1U;
    paths_.push_back(std::move(converted));
  }
  if (characters >
      ((std::numeric_limits<std::size_t>::max)() - sizeof(DROPFILES)) /
          sizeof(wchar_t)) return;
  character_count_ = characters;
  valid_ = true;
}

bool Win32OleFileDataObject::valid() const { return valid_; }

HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::QueryInterface(
    REFIID interface_id, void** object) {
  if (object == nullptr) return E_POINTER;
  if (IsEqualIID(interface_id, IID_IUnknown) ||
      IsEqualIID(interface_id, IID_IDataObject)) {
    *object = static_cast<IDataObject*>(this);
    AddRef();
    return S_OK;
  }
  *object = nullptr;
  return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Win32OleFileDataObject::AddRef() {
  return static_cast<ULONG>(reference_count_.fetch_add(1) + 1U);
}
ULONG STDMETHODCALLTYPE Win32OleFileDataObject::Release() {
  return static_cast<ULONG>(reference_count_.fetch_sub(1) - 1U);
}

HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::QueryGetData(
    FORMATETC* format) {
  if (format == nullptr) return E_POINTER;
  if (!valid_ || format->cfFormat != CF_HDROP) return DV_E_FORMATETC;
  if (format->dwAspect != DVASPECT_CONTENT) return DV_E_DVASPECT;
  if (format->lindex != -1) return DV_E_LINDEX;
  if ((format->tymed & TYMED_HGLOBAL) == 0) return DV_E_TYMED;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::GetData(
    FORMATETC* format, STGMEDIUM* storage) {
  if (storage == nullptr) return E_POINTER;
  *storage = {};
  const HRESULT query = QueryGetData(format);
  if (FAILED(query)) return query;
  const std::size_t bytes =
      sizeof(DROPFILES) + character_count_ * sizeof(wchar_t);
  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
  if (handle == nullptr) return STG_E_MEDIUMFULL;
  auto* drop = static_cast<DROPFILES*>(GlobalLock(handle));
  if (drop == nullptr) {
    GlobalFree(handle);
    return STG_E_MEDIUMFULL;
  }
  drop->pFiles = sizeof(DROPFILES);
  drop->fWide = TRUE;
  auto* cursor = reinterpret_cast<wchar_t*>(
      reinterpret_cast<unsigned char*>(drop) + drop->pFiles);
  for (const std::wstring& path : paths_) {
    std::memcpy(cursor, path.data(), path.size() * sizeof(wchar_t));
    cursor += path.size();
    *cursor++ = L'\0';
  }
  *cursor = L'\0';
  GlobalUnlock(handle);
  storage->tymed = TYMED_HGLOBAL;
  storage->hGlobal = handle;
  storage->pUnkForRelease = nullptr;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::GetDataHere(
    FORMATETC*, STGMEDIUM*) { return DATA_E_FORMATETC; }
HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::GetCanonicalFormatEtc(
    FORMATETC*, FORMATETC* canonical) {
  if (canonical == nullptr) return E_POINTER;
  canonical->ptd = nullptr;
  return DATA_S_SAMEFORMATETC;
}
HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::SetData(
    FORMATETC*, STGMEDIUM*, BOOL) { return E_NOTIMPL; }
HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::EnumFormatEtc(
    DWORD direction, IEnumFORMATETC** enumerator) {
  if (enumerator == nullptr) return E_POINTER;
  *enumerator = nullptr;
  if (direction != DATADIR_GET || !valid_) return E_NOTIMPL;
  FORMATETC format{CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  return SHCreateStdEnumFmtEtc(1, &format, enumerator);
}
HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::DAdvise(
    FORMATETC*, DWORD, IAdviseSink*, DWORD* connection) {
  if (connection != nullptr) *connection = 0;
  return OLE_E_ADVISENOTSUPPORTED;
}
HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::DUnadvise(DWORD) {
  return OLE_E_ADVISENOTSUPPORTED;
}
HRESULT STDMETHODCALLTYPE Win32OleFileDataObject::EnumDAdvise(
    IEnumSTATDATA**) { return OLE_E_ADVISENOTSUPPORTED; }

Win32OleDragResult run_win32_ole_file_drag(
    std::span<const std::string> paths,
    DWORD allowed_effects,
    Win32DoDragDrop runner) {
  Win32OleFileDataObject data_object{paths};
  if (!data_object.valid()) return {};
  return run_win32_ole_drag(data_object, allowed_effects, runner);
}

} // namespace cgpui
