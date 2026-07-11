#include "win32_ole_text_data_object_internal.hpp"

#include <string>

namespace {
std::wstring expected_text;
int runner_calls = 0;

HRESULT WINAPI inspect_text(
    IDataObject* data, IDropSource*, DWORD allowed, DWORD* effect) {
  ++runner_calls;
  if (data == nullptr || effect == nullptr || allowed != DROPEFFECT_COPY) return E_FAIL;
  FORMATETC format{CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  if (data->QueryGetData(&format) != S_OK) return E_FAIL;
  IEnumFORMATETC* enumerator = nullptr;
  if (data->EnumFormatEtc(DATADIR_GET, &enumerator) != S_OK || enumerator == nullptr)
    return E_FAIL;
  FORMATETC enumerated{};
  const HRESULT next = enumerator->Next(1, &enumerated, nullptr);
  enumerator->Release();
  if (next != S_OK || enumerated.cfFormat != CF_UNICODETEXT) return E_FAIL;
  STGMEDIUM storage{};
  if (data->GetData(&format, &storage) != S_OK || storage.tymed != TYMED_HGLOBAL)
    return E_FAIL;
  const auto* text = static_cast<const wchar_t*>(GlobalLock(storage.hGlobal));
  const bool matches = text != nullptr && std::wstring{text} == expected_text;
  if (text != nullptr) GlobalUnlock(storage.hGlobal);
  ReleaseStgMedium(&storage);
  if (!matches) return E_FAIL;
  *effect = DROPEFFECT_COPY;
  return DRAGDROP_S_DROP;
}
} // namespace

int main() {
  cgpui::Win32OleTextDataObject object{"hello"};
  void* queried = nullptr;
  if (!object.valid() || object.QueryInterface(IID_IDataObject, &queried) != S_OK ||
      queried != static_cast<IDataObject*>(&object)) return 1;
  if (object.Release() != 1) return 2;
  FORMATETC wrong{CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  if (object.QueryGetData(&wrong) != DV_E_FORMATETC) return 3;

  expected_text = L"drag text \U0001F642\r\n";
  auto result = cgpui::run_win32_ole_text_drag(
      "drag text \xF0\x9F\x99\x82\r\n", DROPEFFECT_COPY, &inspect_text);
  if (result.result != DRAGDROP_S_DROP || result.effect != DROPEFFECT_COPY) return 4;
  expected_text.clear();
  result = cgpui::run_win32_ole_text_drag("", DROPEFFECT_COPY, &inspect_text);
  if (result.result != DRAGDROP_S_DROP || result.effect != DROPEFFECT_COPY) return 5;
  const int valid_calls = runner_calls;
  const std::string embedded{"a\0b", 3};
  if (cgpui::run_win32_ole_text_drag(embedded, DROPEFFECT_COPY, &inspect_text).result != E_FAIL ||
      cgpui::run_win32_ole_text_drag("\xC3\x28", DROPEFFECT_COPY, &inspect_text).result != E_FAIL ||
      runner_calls != valid_calls) return 6;
  return 0;
}
