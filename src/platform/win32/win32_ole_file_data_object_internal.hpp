#pragma once

#include "win32_drag_drop_internal.hpp"

#include <span>
#include <string>
#include <vector>

namespace cgpui {

class Win32OleFileDataObject final : public IDataObject {
 public:
  explicit Win32OleFileDataObject(std::span<const std::string> paths);
  [[nodiscard]] bool valid() const;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID interface_id, void** object)
      override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  HRESULT STDMETHODCALLTYPE GetData(FORMATETC* format, STGMEDIUM* storage)
      override;
  HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC*, STGMEDIUM*) override;
  HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* format) override;
  HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(
      FORMATETC*, FORMATETC* canonical) override;
  HRESULT STDMETHODCALLTYPE SetData(FORMATETC*, STGMEDIUM*, BOOL) override;
  HRESULT STDMETHODCALLTYPE EnumFormatEtc(
      DWORD direction, IEnumFORMATETC** enumerator) override;
  HRESULT STDMETHODCALLTYPE DAdvise(
      FORMATETC*, DWORD, IAdviseSink*, DWORD*) override;
  HRESULT STDMETHODCALLTYPE DUnadvise(DWORD) override;
  HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA**) override;

 private:
  std::atomic_ulong reference_count_{1};
  std::vector<std::wstring> paths_;
  std::size_t character_count_ = 0;
  bool valid_ = false;
};

[[nodiscard]] Win32OleDragResult run_win32_ole_file_drag(
    std::span<const std::string> paths,
    DWORD allowed_effects,
    Win32DoDragDrop runner = &DoDragDrop);

} // namespace cgpui
