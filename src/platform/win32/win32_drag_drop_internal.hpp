#pragma once

#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <shellapi.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace cgpui {

struct Win32TestDragDropPayload {
  float x = 0.0F;
  float y = 0.0F;
  const wchar_t* text = nullptr;
  const wchar_t* const* files = nullptr;
  std::size_t file_count = 0;
  std::uint32_t drop_effect = 0;
};

struct Win32OleDropTargetRegistrationState {
  HWND hwnd = nullptr;
  bool registered = false;
  HRESULT last_registration_result = S_FALSE;
  HRESULT last_revocation_result = S_FALSE;
};

class Win32OleDropTargetOwner {
 public:
  virtual ~Win32OleDropTargetOwner() = default;
  virtual void ole_drag_entered(
      IDataObject* data_object,
      POINTL point,
      DWORD key_state,
      DWORD* effect) = 0;
  virtual void ole_drag_updated(
      POINTL point,
      DWORD key_state,
      DWORD* effect) = 0;
  virtual void ole_drag_exited() = 0;
  virtual void ole_drag_dropped(
      IDataObject* data_object,
      POINTL point,
      DWORD key_state,
      DWORD* effect) = 0;
};

class Win32OleDropTarget final : public IDropTarget {
 public:
  explicit Win32OleDropTarget(Win32OleDropTargetOwner& owner);

  HRESULT STDMETHODCALLTYPE QueryInterface(
      REFIID interface_id,
      void** object) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  HRESULT STDMETHODCALLTYPE DragEnter(
      IDataObject* data_object,
      DWORD key_state,
      POINTL point,
      DWORD* effect) override;
  HRESULT STDMETHODCALLTYPE DragOver(
      DWORD key_state,
      POINTL point,
      DWORD* effect) override;
  HRESULT STDMETHODCALLTYPE DragLeave() override;
  HRESULT STDMETHODCALLTYPE Drop(
      IDataObject* data_object,
      DWORD key_state,
      POINTL point,
      DWORD* effect) override;

 private:
  std::atomic_ulong reference_count_{1};
  Win32OleDropTargetOwner* owner_ = nullptr;
};

} // namespace cgpui
