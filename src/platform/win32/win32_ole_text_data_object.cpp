#include "win32_ole_text_data_object_internal.hpp"

#include <climits>
#include <cstring>
#include <shlobj_core.h>

namespace cgpui {
namespace {
bool convert_text(std::string_view text, std::wstring& result) {
  if (text.find('\0') != std::string_view::npos || text.size() > INT_MAX) return false;
  if (text.empty()) return true;
  const int required = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()),
      nullptr, 0);
  if (required <= 0) return false;
  result.resize(static_cast<std::size_t>(required));
  return MultiByteToWideChar(
             CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
             static_cast<int>(text.size()), result.data(), required) == required;
}
} // namespace

Win32OleTextDataObject::Win32OleTextDataObject(std::string_view text)
    : valid_(convert_text(text, text_)) {}

bool Win32OleTextDataObject::valid() const { return valid_; }

HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::QueryInterface(
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

ULONG STDMETHODCALLTYPE Win32OleTextDataObject::AddRef() {
  return static_cast<ULONG>(reference_count_.fetch_add(1) + 1U);
}

ULONG STDMETHODCALLTYPE Win32OleTextDataObject::Release() {
  return static_cast<ULONG>(reference_count_.fetch_sub(1) - 1U);
}

HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::QueryGetData(
    FORMATETC* format) {
  if (format == nullptr) return E_POINTER;
  if (!valid_ || format->cfFormat != CF_UNICODETEXT) return DV_E_FORMATETC;
  if (format->dwAspect != DVASPECT_CONTENT) return DV_E_DVASPECT;
  if (format->lindex != -1) return DV_E_LINDEX;
  if ((format->tymed & TYMED_HGLOBAL) == 0) return DV_E_TYMED;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::GetData(
    FORMATETC* format, STGMEDIUM* storage) {
  if (storage == nullptr) return E_POINTER;
  *storage = {};
  const HRESULT query = QueryGetData(format);
  if (FAILED(query)) return query;
  const std::size_t bytes = (text_.size() + 1U) * sizeof(wchar_t);
  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
  if (handle == nullptr) return STG_E_MEDIUMFULL;
  void* memory = GlobalLock(handle);
  if (memory == nullptr) {
    GlobalFree(handle);
    return STG_E_MEDIUMFULL;
  }
  std::memcpy(memory, text_.c_str(), bytes);
  GlobalUnlock(handle);
  storage->tymed = TYMED_HGLOBAL;
  storage->hGlobal = handle;
  storage->pUnkForRelease = nullptr;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::GetDataHere(
    FORMATETC*, STGMEDIUM*) { return DATA_E_FORMATETC; }

HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::GetCanonicalFormatEtc(
    FORMATETC*, FORMATETC* canonical) {
  if (canonical == nullptr) return E_POINTER;
  canonical->ptd = nullptr;
  return DATA_S_SAMEFORMATETC;
}

HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::SetData(
    FORMATETC*, STGMEDIUM*, BOOL) { return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::EnumFormatEtc(
    DWORD direction, IEnumFORMATETC** enumerator) {
  if (enumerator == nullptr) return E_POINTER;
  *enumerator = nullptr;
  if (direction != DATADIR_GET || !valid_) return E_NOTIMPL;
  FORMATETC format{CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  return SHCreateStdEnumFmtEtc(1, &format, enumerator);
}

HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::DAdvise(
    FORMATETC*, DWORD, IAdviseSink*, DWORD* connection) {
  if (connection != nullptr) *connection = 0;
  return OLE_E_ADVISENOTSUPPORTED;
}
HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::DUnadvise(DWORD) {
  return OLE_E_ADVISENOTSUPPORTED;
}
HRESULT STDMETHODCALLTYPE Win32OleTextDataObject::EnumDAdvise(
    IEnumSTATDATA**) { return OLE_E_ADVISENOTSUPPORTED; }

Win32OleDragResult run_win32_ole_text_drag(
    std::string_view text, DWORD allowed_effects, Win32DoDragDrop runner) {
  Win32OleTextDataObject data_object{text};
  if (!data_object.valid()) return {};
  return run_win32_ole_drag(data_object, allowed_effects, runner);
}

} // namespace cgpui
