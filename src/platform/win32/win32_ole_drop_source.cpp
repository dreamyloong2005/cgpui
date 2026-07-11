#include "win32_drag_drop_internal.hpp"

namespace cgpui {

HRESULT STDMETHODCALLTYPE Win32OleDropSource::QueryInterface(
    REFIID interface_id,
    void** object) {
  if (object == nullptr) return E_POINTER;
  if (IsEqualIID(interface_id, IID_IUnknown) ||
      IsEqualIID(interface_id, IID_IDropSource)) {
    *object = static_cast<IDropSource*>(this);
    AddRef();
    return S_OK;
  }
  *object = nullptr;
  return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Win32OleDropSource::AddRef() {
  return static_cast<ULONG>(reference_count_.fetch_add(1) + 1U);
}

ULONG STDMETHODCALLTYPE Win32OleDropSource::Release() {
  return static_cast<ULONG>(reference_count_.fetch_sub(1) - 1U);
}

HRESULT STDMETHODCALLTYPE Win32OleDropSource::QueryContinueDrag(
    BOOL escape_pressed,
    DWORD key_state) {
  if (escape_pressed != FALSE) return DRAGDROP_S_CANCEL;
  if ((key_state & (MK_LBUTTON | MK_RBUTTON)) == 0) return DRAGDROP_S_DROP;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleDropSource::GiveFeedback(DWORD) {
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

Win32OleDragResult run_win32_ole_drag(
    IDataObject& data_object,
    DWORD allowed_effects,
    Win32DoDragDrop runner) {
  if (runner == nullptr || allowed_effects == DROPEFFECT_NONE) return {};
  Win32OleDropSource source;
  DWORD effect = DROPEFFECT_NONE;
  const HRESULT result = runner(&data_object, &source, allowed_effects, &effect);
  return Win32OleDragResult{.result = result, .effect = effect};
}

} // namespace cgpui
