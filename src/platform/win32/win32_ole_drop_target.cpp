#include "win32_internal.hpp"

namespace cgpui {

Win32OleDropTarget::Win32OleDropTarget(Win32OleDropTargetOwner& owner)
    : owner_(&owner) {}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::QueryInterface(
    REFIID interface_id,
    void** object) {
  if (object == nullptr) {
    return E_POINTER;
  }
  if (IsEqualIID(interface_id, IID_IUnknown) ||
      IsEqualIID(interface_id, IID_IDropTarget)) {
    *object = static_cast<IDropTarget*>(this);
    AddRef();
    return S_OK;
  }
  *object = nullptr;
  return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Win32OleDropTarget::AddRef() {
  return static_cast<ULONG>(reference_count_.fetch_add(1) + 1U);
}

ULONG STDMETHODCALLTYPE Win32OleDropTarget::Release() {
  const auto count = reference_count_.fetch_sub(1) - 1U;
  return static_cast<ULONG>(count);
}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::DragEnter(
    IDataObject* data_object,
    DWORD key_state,
    POINTL point,
    DWORD* effect) {
  if (owner_ != nullptr) {
    owner_->ole_drag_entered(data_object, point, key_state, effect);
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::DragOver(
    DWORD key_state,
    POINTL point,
    DWORD* effect) {
  if (owner_ != nullptr) {
    owner_->ole_drag_updated(point, key_state, effect);
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::DragLeave() {
  if (owner_ != nullptr) {
    owner_->ole_drag_exited();
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::Drop(
    IDataObject* data_object,
    DWORD key_state,
    POINTL point,
    DWORD* effect) {
  if (owner_ != nullptr) {
    owner_->ole_drag_dropped(data_object, point, key_state, effect);
  }
  return S_OK;
}

} // namespace cgpui
