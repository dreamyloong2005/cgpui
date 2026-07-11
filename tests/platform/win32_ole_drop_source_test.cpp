#include "win32_drag_drop_internal.hpp"

namespace {
DWORD observed_allowed = DROPEFFECT_NONE;

HRESULT WINAPI run_drag(
    IDataObject* data,
    IDropSource* source,
    DWORD allowed,
    DWORD* effect) {
  if (data == nullptr || source == nullptr || effect == nullptr) return E_POINTER;
  observed_allowed = allowed;
  if (source->QueryContinueDrag(TRUE, MK_LBUTTON) != DRAGDROP_S_CANCEL) return E_FAIL;
  if (source->QueryContinueDrag(FALSE, 0) != DRAGDROP_S_DROP) return E_FAIL;
  if (source->QueryContinueDrag(FALSE, MK_LBUTTON) != S_OK) return E_FAIL;
  if (source->GiveFeedback(DROPEFFECT_COPY) !=
      DRAGDROP_S_USEDEFAULTCURSORS) return E_FAIL;
  *effect = DROPEFFECT_MOVE;
  return DRAGDROP_S_DROP;
}

HRESULT WINAPI cancel_drag(
    IDataObject*,
    IDropSource* source,
    DWORD,
    DWORD* effect) {
  if (source == nullptr || effect == nullptr) return E_POINTER;
  if (source->QueryContinueDrag(TRUE, MK_LBUTTON) != DRAGDROP_S_CANCEL) {
    return E_FAIL;
  }
  *effect = DROPEFFECT_MOVE;
  return DRAGDROP_S_CANCEL;
}
} // namespace

int main() {
  cgpui::Win32OleDropSource source;
  void* object = nullptr;
  if (source.QueryInterface(IID_IDropSource, &object) != S_OK ||
      object != static_cast<IDropSource*>(&source)) return 1;
  if (source.Release() != 1) return 2;

  auto* data = reinterpret_cast<IDataObject*>(static_cast<std::uintptr_t>(1));
  const auto invalid = cgpui::run_win32_ole_drag(
      *data, DROPEFFECT_NONE, &run_drag);
  if (invalid.result != E_FAIL || invalid.effect != DROPEFFECT_NONE) return 3;
  const auto result = cgpui::run_win32_ole_drag(
      *data, DROPEFFECT_COPY | DROPEFFECT_MOVE, &run_drag);
  if (result.result != DRAGDROP_S_DROP || result.effect != DROPEFFECT_MOVE ||
      observed_allowed != (DROPEFFECT_COPY | DROPEFFECT_MOVE)) return 4;
  const auto cancelled = cgpui::run_win32_ole_drag(
      *data, DROPEFFECT_COPY | DROPEFFECT_MOVE, &cancel_drag);
  if (cancelled.result != DRAGDROP_S_CANCEL ||
      cancelled.effect != DROPEFFECT_NONE) return 5;
  return 0;
}
