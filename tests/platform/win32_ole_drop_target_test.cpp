#include "win32_drag_drop_internal.hpp"

#include <cstdint>

namespace {
class Owner final : public cgpui::Win32OleDropTargetOwner {
 public:
  void ole_drag_entered(IDataObject* data, POINTL, DWORD, DWORD*) override {
    entered = data;
  }
  void ole_drag_updated(POINTL, DWORD, DWORD*) override { ++updated; }
  void ole_drag_exited() override { ++exited; }
  void ole_drag_dropped(IDataObject* data, POINTL, DWORD, DWORD*) override {
    dropped = data;
  }

  IDataObject* entered = nullptr;
  IDataObject* dropped = nullptr;
  std::uint32_t updated = 0;
  std::uint32_t exited = 0;
};
} // namespace

int main() {
  Owner owner;
  cgpui::Win32OleDropTarget target(owner);
  if (target.QueryInterface(IID_IUnknown, nullptr) != E_POINTER) return 1;
  void* object = reinterpret_cast<void*>(1);
  if (target.QueryInterface(IID_IClassFactory, &object) != E_NOINTERFACE ||
      object != nullptr) return 2;
  if (target.QueryInterface(IID_IDropTarget, &object) != S_OK ||
      object != static_cast<IDropTarget*>(&target)) return 3;
  if (target.Release() != 1) return 4;

  DWORD effect = DROPEFFECT_COPY;
  if (target.DragEnter(nullptr, 0, POINTL{}, &effect) != E_INVALIDARG ||
      effect != DROPEFFECT_NONE || owner.entered != nullptr) return 5;
  if (target.DragOver(0, POINTL{}, nullptr) != E_INVALIDARG ||
      owner.updated != 0) return 6;
  effect = DROPEFFECT_MOVE;
  if (target.Drop(nullptr, 0, POINTL{}, &effect) != E_INVALIDARG ||
      effect != DROPEFFECT_NONE || owner.dropped != nullptr) return 7;

  auto* data = reinterpret_cast<IDataObject*>(static_cast<std::uintptr_t>(1));
  if (target.DragEnter(data, 0, POINTL{}, &effect) != S_OK ||
      owner.entered != data) return 8;
  if (target.DragOver(0, POINTL{}, &effect) != S_OK || owner.updated != 1) {
    return 9;
  }
  if (target.DragLeave() != S_OK || owner.exited != 1) return 10;
  if (target.Drop(data, 0, POINTL{}, &effect) != S_OK ||
      owner.dropped != data) return 11;
  return 0;
}
