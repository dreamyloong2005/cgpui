#pragma once

#include "win32_drag_drop_internal.hpp"

#include <string>
#include <string_view>

namespace cgpui {

class Win32OleTextDataObject final : public IDataObject {
 public:
  explicit Win32OleTextDataObject(std::string_view text);
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
  std::wstring text_;
  bool valid_ = false;
};

[[nodiscard]] Win32OleDragResult run_win32_ole_text_drag(
    std::string_view text,
    DWORD allowed_effects,
    Win32DoDragDrop runner = &DoDragDrop);

} // namespace cgpui
